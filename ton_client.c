/* ton_client extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_ton_client.h"
#include "tonclient.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ string ton_create_context()
 */
PHP_FUNCTION(ton_create_context)
{
	ZEND_PARSE_PARAMETERS_NONE();

    // TODO: replace with TON library call
    zend_string *result = zend_string_init("{\"result\":1}", strlen("{\"result\":1}"), 0);

    php_printf("Calling %s\n", "ton_create_context");
    php_printf("Result of %s: %s\n", "ton_create_context", ZSTR_VAL(result));

    RETURN_STR(result);
}
/* }}} */

/* {{{ void ton_destroy_context([ int $context ])
 */
PHP_FUNCTION(ton_destroy_context)
{
    zend_long context = -1;
    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(context)
    ZEND_PARSE_PARAMETERS_END();

    php_printf("Calling %s(%zd)\n", "ton_destroy_context", context);
    // TODO: make TON library call
}
/* }}} */

/* {{{ void ton_request( [ string $json, mixed callback ] )
 */
PHP_FUNCTION(ton_request)
{
    zend_string *json;
    zval *callback;
    zend_fcall_info_cache fcc;

	ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(json)
        Z_PARAM_ZVAL(callback)
	ZEND_PARSE_PARAMETERS_END();

    if (!zend_is_callable_ex(callback, NULL, 0, NULL, &fcc, NULL)) {
        zend_string	*callback_name = zend_get_callable_name(callback);
        php_error_docref(NULL, E_WARNING, "Requires argument 2, '%s', to be a valid callback", ZSTR_VAL(callback_name));
        zend_string_release_ex(callback_name, 0);
        return;
    }

    php_printf("Calling %s with JSON argument %s\n", "ton_request", ZSTR_VAL(json));

    zval callback_args[3]; // json, response_type, finished
    ZVAL_STRINGL(&callback_args[0], "{\"test\":true}", strlen("{\"test\":true}"));
    ZVAL_LONG(&callback_args[1], (zend_long)1);
    ZVAL_BOOL(&callback_args[2], (zend_bool)1);

    zend_fcall_info fci;
    fci.size = sizeof(fci);
    fci.object = NULL;
    fci.param_count = 3; // json, response_type, finished
    fci.params = callback_args;
    fci.no_separation = 0;
    ZVAL_COPY_VALUE(&fci.function_name, callback);
    if (zend_call_function(&fci, &fcc) == FAILURE) {
        php_error_docref(NULL, E_WARNING, "Cannot call the callback");
    }
    zval_ptr_dtor(&callback_args[0]);
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
ZEND_BEGIN_ARG_INFO(arginfo_ton_create_context, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ton_destroy_context, 0)
	ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ton_request, 0)
    ZEND_ARG_INFO(0, json)
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
