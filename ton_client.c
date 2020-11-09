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

/* {{{ void ton_request( int $context, string $function_name, string $params_json )
 */
PHP_FUNCTION(ton_request)
{
    zend_long context;
    zend_string *function_name;
    zend_string *params_json;

#if defined(TON_DEBUG)
    fprintf(stderr, "ton_request is called\n");
#endif

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_LONG(context)
        Z_PARAM_STR(function_name)
        Z_PARAM_STR(params_json)
    ZEND_PARSE_PARAMETERS_END();

    tc_string_data_t f_name = {ZSTR_VAL(function_name), ZSTR_LEN(function_name)};
    tc_string_data_t f_params = {ZSTR_VAL(params_json), ZSTR_LEN(params_json)};
    tc_string_handle_t * response_handle = tc_request_sync(context, f_name, f_params);
    tc_string_data_t json = tc_read_string(response_handle);
    zend_string *response_json = zend_string_init(json.content, json.len, 0);
    tc_destroy_string(response_handle);

    RETURN_STR(response_json);
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_ton_request, 0, 0, 3)
    ZEND_ARG_INFO(0, context)
    ZEND_ARG_INFO(0, function_name)
    ZEND_ARG_INFO(0, params_json)
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
