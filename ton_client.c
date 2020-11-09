/* ton_client extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef ZEND_DEBUG
#define ZEND_DEBUG 0
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_ton_client.h"
#include <stdbool.h>
#include "tonclient.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

void fail(const char *fmt, ...) {
    va_list args;
            va_start(args, fmt);
    vfprintf(stderr, fmt, args);
            va_end(args);
    exit(-1);
}

typedef struct ton_handler_data {
    zval handler;
    zend_fcall_info_cache fcc;
} ton_handler_data_t;

ton_handler_data_t *create_handler_data(zval * handler) {
    ton_handler_data_t *data = calloc(1, sizeof(ton_handler_data_t));
    data->fcc = empty_fcall_info_cache;
    ZVAL_COPY(&data->handler, handler);
    return data;
}

void free_handler_data(ton_handler_data_t *data) {
    assert(data);
    if (!data) {
        fail("NULL passed to free_handler_data\n");
        return;
    }
    free(data);
}

char *mem_copy_str_n(const char *str, uint32_t len) {
    char *copy = malloc((len + 1) * sizeof(char));
    strncpy(copy, str, len);
    copy[len] = 0;
    return copy;
}

void ton_response_handler(
        void *request_ptr,
        tc_string_data_t params_json,
        uint32_t response_type,
        bool finished) {

#if defined(TON_DEBUG)
    char* json = mem_copy_str_n(params_json.content, params_json.len);
    fprintf(stderr, "ton_response_handler is called: response_type=%d; finished=%d; params_json=%s\n",
            response_type, finished, json);
    free(json);
#endif

    ton_handler_data_t *data = request_ptr;
    assert(data);

    if (!data) {
        fail("NULL ptr passed to response handler\n");
        return;
    }

#if defined(TON_DEBUG)
    fprintf(stderr, "tmp ref count is: %d", Z_REFCOUNT(data->handler));
    fprintf(stderr, "calling callback handler: ");
    zend_string	*callback_name = zend_get_callable_name(&data->handler);
    fprintf(stderr, "%s\n", ZSTR_VAL(callback_name));
    zend_string_release_ex(callback_name, 0);
#endif

    zval callback_args[3]; // json, response_type, finished
    ZVAL_STRINGL(&callback_args[0], params_json.content, params_json.len);
    ZVAL_LONG(&callback_args[1], (zend_long)response_type);
    ZVAL_BOOL(&callback_args[2], (zend_bool)finished);

    zend_fcall_info fci;
    fci.size = sizeof(fci);
    fci.object = NULL;
    fci.param_count = 3; // json, response_type, finished
    fci.params = callback_args;
    fci.no_separation = 0;
    ZVAL_COPY_VALUE(&fci.function_name, &data->handler);
    int error = zend_call_function(&fci, &data->fcc);
    if (error == FAILURE) {
#if defined(TON_DEBUG)
        fprintf(stderr, "callback failed\n");
#endif
        php_error_docref(NULL, E_WARNING, "Cannot call the callback");
    } else if (error == SUCCESS) {
#if defined(TON_DEBUG)
        fprintf(stderr, "callback called successfully\n");
#endif
    }
    zval_ptr_dtor(&callback_args[0]);

    if (finished) {
        free_handler_data(data);
    }
}

/* {{{ string ton_create_context( string $config_json )
 */
PHP_FUNCTION(ton_create_context)
{
    zend_string *config_json;

#if defined(TON_DEBUG)
    fprintf(stderr, "ton_create_context is called\n");
#endif

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(config_json)
    ZEND_PARSE_PARAMETERS_END();

    tc_string_data_t str = {ZSTR_VAL(config_json), ZSTR_LEN(config_json)};
    tc_string_handle_t *result = tc_create_context(str);
    tc_string_data_t json = tc_read_string(result);
    zend_string *return_str = zend_string_init(json.content, json.len, 0);
    tc_destroy_string(result);

#if defined(TON_DEBUG)
    fprintf(stderr, "tc_create_context returned %s\n", ZSTR_VAL(return_str));
#endif

    RETURN_STR(return_str);
}
/* }}} */

/* {{{ void ton_destroy_context( int $context )
 */
PHP_FUNCTION(ton_destroy_context)
{
    zend_long context = -1;

#if defined(TON_DEBUG)
    fprintf(stderr, "ton_destroy_context is called\n");
#endif

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(context)
    ZEND_PARSE_PARAMETERS_END();

#if defined(TON_DEBUG)
    fprintf(stderr, "calling tc_destroy_context with argument %d\n", (int)context);
#endif

    tc_destroy_context(context);

#if defined(TON_DEBUG)
    fprintf(stderr, "tc_destroy_context succeeded\n");
#endif

}
/* }}} */

/* {{{ void ton_request( int $context, string $function_name, string $params_json, mixed callback )
 */
PHP_FUNCTION(ton_request)
{
    zend_long context;
    zend_string *function_name;
    zend_string *params_json;
    zval *callback;
    zend_fcall_info_cache fcc;

#if defined(TON_DEBUG)
    fprintf(stderr, "ton_request is called\n");
#endif

    ZEND_PARSE_PARAMETERS_START(4, 4)
        Z_PARAM_LONG(context)
        Z_PARAM_STR(function_name)
        Z_PARAM_STR(params_json)
        Z_PARAM_ZVAL(callback)
    ZEND_PARSE_PARAMETERS_END();

    if (!zend_is_callable_ex(callback, NULL, 0, NULL, &fcc, NULL)) {
        zend_string	*callback_name = zend_get_callable_name(callback);
        php_error_docref(NULL, E_WARNING, "Requires argument 2, '%s', to be a valid callback", ZSTR_VAL(callback_name));
        zend_string_release_ex(callback_name, 0);
        return;
    }

#if defined(TON_DEBUG)
    zend_string	*callback_name = zend_get_callable_name(callback);
    fprintf(stderr, "context id: %d, func name: %s; params: %s; callback name: %s\n",
        (int)context, ZSTR_VAL(function_name), ZSTR_VAL(params_json), ZSTR_VAL(callback_name));
    zend_string_release_ex(callback_name, 0);
#endif

    ton_handler_data_t *data = create_handler_data(callback);
    tc_string_data_t f_name = {ZSTR_VAL(function_name), ZSTR_LEN(function_name)};
    tc_string_data_t f_params = {ZSTR_VAL(params_json), ZSTR_LEN(params_json)};

// NOTE: seems it's impossible to share callback function between multiple threads,
// since Zend uses thread locals internally (share nothing memory management model).
// So we're just using sync version here, which seems the only one option for us.
//    tc_request_ptr(context, f_name, f_params, data, &ton_response_handler);

    tc_string_handle_t * response_handle = tc_request_sync(context, f_name, f_params);
    tc_string_data_t json = tc_read_string(response_handle);
    ton_response_handler(data, json, 0, true);
    tc_destroy_string(response_handle);
}
/* }}}*/

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(ton_client)
{
#if defined(ZTS) && defined(COMPILE_DL_TON_CLIENT)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(ton_client)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "ton_client support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ton_create_context, 0, 0, 1)
    ZEND_ARG_INFO(0, config_json)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ton_destroy_context, 0, 0, 1)
    ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ton_request, 0, 0, 4)
    ZEND_ARG_INFO(0, context)
    ZEND_ARG_INFO(0, function_name)
    ZEND_ARG_INFO(0, params_json)
    ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ ton_client_functions[]
 */
static const zend_function_entry ton_client_functions[] = {
	PHP_FE(ton_create_context,		arginfo_ton_create_context)
	PHP_FE(ton_destroy_context,		arginfo_ton_destroy_context)
    PHP_FE(ton_request,		        arginfo_ton_request)
	PHP_FE_END
};
/* }}} */

/* {{{ ton_client_module_entry
 */
zend_module_entry ton_client_module_entry = {
	STANDARD_MODULE_HEADER,
	"ton_client",					/* Extension name */
	ton_client_functions,			/* zend_function_entry */
	NULL,							/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(ton_client),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(ton_client),			/* PHP_MINFO - Module info */
	PHP_TON_CLIENT_VERSION,		    /* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_TON_CLIENT
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(ton_client)
#endif
