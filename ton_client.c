/* ton_client extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef ZEND_DEBUG
#define ZEND_DEBUG 0
#endif

#include "os.h"
#include "php.h"
#include "ext/standard/info.h"
#include "php_ton_client.h"
#include <stdbool.h>
#include "tonclient.h"
#include "rpa_queue.h"
#include "debug.h"

#define NAMEOF(val) "#VAR"

// MAX number of unprocessed callback handler calls per single TON request.
// In other words, its the MAX number of times the callback can be received
// before previous callbacks have been processes via calling function ton_request_next.
// Worth keeping this number relatively low.

#define CALLBACK_QUEUE_CAPACITY 1024

// Queue element structure.
// Blocking queue is used to operate with the core ton client callbacks.
// PHP client calls ton_request_next func which blocks until the next callback
// JSON is pushed to the queue.

typedef struct ton_callback_queue_element {
    char *json;
    uint32_t len;
    uint32_t status;
    bool finished;
} ton_callback_queue_element_t;

ton_callback_queue_element_t *ton_callback_queue_element_create(
        tc_string_data_t params_json,
        int response_type,
        bool finished) {
    ton_callback_queue_element_t *e = malloc(sizeof(ton_callback_queue_element_t));
    e->json = malloc(params_json.len);
    e->len = params_json.len;
    memcpy(e->json, params_json.content, params_json.len);
    e->status = response_type;
    e->finished = finished;
    return e;
}

void ton_callback_queue_element_free(ton_callback_queue_element_t *e) {
    free(e->json);
    free(e);
}

typedef struct ton_request_data {
    rpa_queue_t * queue;
    bool finished;
    int last_status;
} ton_request_data_t;

ton_request_data_t *ton_request_data_create() {
    ton_request_data_t *data = calloc(1, sizeof(ton_request_data_t));
    data->last_status = -1;
    rpa_queue_create(&data->queue, CALLBACK_QUEUE_CAPACITY);
    return data;
}

void ton_request_data_shutdown_queue(ton_request_data_t *data) {
    TON_DBG_MSG("freeing queue for request %p; size is %d\n", data, rpa_queue_size(data->queue));
    rpa_queue_term(data->queue);
    ton_callback_queue_element_t *e;
    while (rpa_queue_trypop(data->queue, (void**)&e)) {
        ton_callback_queue_element_free(e);
    }
    rpa_queue_destroy(data->queue);
    free(data->queue);
    data->queue = NULL;
}

void ton_request_data_free(ton_request_data_t *data) {
    TON_DBG_MSG("in ton_request_data_free: %p\n", data);
    if (data->queue) {
        ton_request_data_shutdown_queue(data);
    }
    free(data);
}

void response_queueing_handler(
        void *request_ptr,
        tc_string_data_t params_json,
        uint32_t response_type,
        bool finished) {

    TON_DBG_MSG("response_queueing_handler called with request=%p, status=%d, finished=%d\n",
                request_ptr, response_type, finished);

    ton_request_data_t *data = request_ptr;
    ton_callback_queue_element_t *e = ton_callback_queue_element_create(
            params_json, response_type, finished);

    data->last_status = response_type;
    data->finished = finished;
    rpa_queue_push(data->queue, e);
    TON_DBG_MSG("request %p callback data pushed to the queue; queue size is: %d\n", request_ptr,
                rpa_queue_size(data->queue));
}

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
    ZEND_PARSE_PARAMETERS_START(0, 0) \
    ZEND_PARSE_PARAMETERS_END()
#endif

/* True global resources - no need for thread safety here */
static int res_num;
/* }}} */

void ton_resource_destructor(zend_resource *rsrc) /* {{{ */
{
    TON_DBG_MSG("in ton_resource_destructor: %p\n", rsrc->ptr);
    if (rsrc->ptr) {
        ton_request_data_t *data = (ton_request_data_t *) rsrc->ptr;
        ton_request_data_free(data);
        rsrc->ptr = NULL;
    }
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(ton_client)
{
    res_num = zend_register_list_destructors_ex(ton_resource_destructor, NULL, NAMEOF(ton_request_data_t), module_number);
    return SUCCESS;
}
/* }}} */

/* {{{ string ton_create_context( string $config_json )
 */
PHP_FUNCTION(ton_create_context)
{
    zend_string *config_json;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(config_json)
    ZEND_PARSE_PARAMETERS_END();

    TON_DBG_MSG("ton_create_context is called with config %s\n", ZSTR_VAL(config_json));

    tc_string_data_t str = {ZSTR_VAL(config_json), ZSTR_LEN(config_json)};
    tc_string_handle_t *result = tc_create_context(str);
    tc_string_data_t json = tc_read_string(result);
    zend_string *return_str = zend_string_init(json.content, json.len, 0);
    tc_destroy_string(result);

    TON_DBG_MSG("tc_create_context returned %s\n", ZSTR_VAL(return_str));

    RETURN_STR(return_str);
}
/* }}} */

/* {{{ void ton_destroy_context( int $context )
 */
PHP_FUNCTION(ton_destroy_context)
{
    zend_long context = -1;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(context)
    ZEND_PARSE_PARAMETERS_END();

    TON_DBG_MSG("calling tc_destroy_context with argument %d\n", (int)context);
    tc_destroy_context(context);
    TON_DBG_MSG("tc_destroy_context succeeded\n");
}
/* }}} */

/* {{{ string ton_request_sync( int $context, string $function_name, string $params_json )
 */
PHP_FUNCTION(ton_request_sync)
{
    zend_long context;
    zend_string *function_name;
    zend_string *params_json;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_LONG(context)
        Z_PARAM_STR(function_name)
        Z_PARAM_STR(params_json)
    ZEND_PARSE_PARAMETERS_END();

    TON_DBG_MSG("ton_request_sync is called with arguments %lld, %s, %s\n",
                context,
                ZSTR_VAL(function_name),
                ZSTR_VAL(params_json));

    tc_string_data_t f_name = {ZSTR_VAL(function_name), ZSTR_LEN(function_name)};
    tc_string_data_t f_params = {ZSTR_VAL(params_json), ZSTR_LEN(params_json)};
    tc_string_handle_t * response_handle = tc_request_sync(context, f_name, f_params);
    tc_string_data_t json = tc_read_string(response_handle);
    zend_string *response_json = zend_string_init(json.content, json.len, 0);
    tc_destroy_string(response_handle);

    RETURN_STR(response_json);
}
/* }}}*/

/* {{{ resource ton_request_start( int $context, string $function_name, string $params_json )
 */
PHP_FUNCTION(ton_request_start)
{
    zend_long context;
    zend_string *function_name;
    zend_string *params_json;

    ZEND_PARSE_PARAMETERS_START(3, 3)
    Z_PARAM_LONG(context)
    Z_PARAM_STR(function_name)
    Z_PARAM_STR(params_json)
    ZEND_PARSE_PARAMETERS_END();

    TON_DBG_MSG("ton_request_start is called with arguments %lld, %s, %s\n",
                context,
                ZSTR_VAL(function_name),
                ZSTR_VAL(params_json));

    ton_request_data_t* payload = ton_request_data_create();
    tc_string_data_t f_name = {ZSTR_VAL(function_name), ZSTR_LEN(function_name)};
    tc_string_data_t f_params = {ZSTR_VAL(params_json), ZSTR_LEN(params_json)};
    tc_request_ptr(context, f_name, f_params, payload, &response_queueing_handler);

    TON_DBG_MSG("ton_request_start returned with resource %p\n", payload);

    RETURN_RES(zend_register_resource(payload, res_num));
}
/* }}}*/

/* {{{ array ton_request_next( resource $request, int $timeout )
 */
PHP_FUNCTION(ton_request_next)
{
    zval *res;
    zend_long timeout = -1;

    ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_RESOURCE(res)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG(timeout)
    ZEND_PARSE_PARAMETERS_END();

    ton_request_data_t * data;
    if ((data = (ton_request_data_t*)zend_fetch_resource(Z_RES_P(res), NAMEOF(ton_request_data_t), res_num)) == NULL) {
        RETURN_NULL();
    }

    TON_DBG_MSG("ton_request_next is called for request %p\n", data);
    TON_DBG_MSG("Calling rpa_queue_pop for request %p; timeout = %lld\n", data, timeout);
    ton_callback_queue_element_t *e;
    bool result = (ZEND_NUM_ARGS() == 1)
            ? rpa_queue_pop(data->queue, (void**)&e)
            : rpa_queue_timedpop(data->queue, (void**)&e, (int)timeout);
    if (!result) {
        TON_DBG_MSG("rpa_queue_pop for request %p returned false\n", data);
        RETURN_NULL();
    }

#ifdef TON_DEBUG
    zend_string *str = zend_string_init(e->json, e->len, 0);
    TON_DBG_MSG("ton_request_next (%p): return %s\n", data, ZSTR_VAL(str));
    zend_string_release(str);
#endif

    // returning tuple [json, status, finished]
    zval json, status, finished;
    ZVAL_STRINGL(&json, e->json, e->len);
    ZVAL_LONG(&status, e->status);
    ZVAL_BOOL(&finished, e->finished);
    HashTable *tuple = zend_new_array(3);
    zend_hash_next_index_insert(tuple, &json);
    zend_hash_next_index_insert(tuple, &status);
    zend_hash_next_index_insert(tuple, &finished);

    ton_callback_queue_element_free(e);
    RETURN_ARR(tuple);
}
/* }}}*/

/* {{{ ?bool is_ton_request_finished( resource $request )
 */
PHP_FUNCTION(is_ton_request_finished)
{
    zval *res;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(res)
    ZEND_PARSE_PARAMETERS_END();

    ton_request_data_t* data;
    if ((data = (ton_request_data_t*)zend_fetch_resource(Z_RES_P(res), NAMEOF(ton_request_data_t), res_num)) == NULL) {
        RETURN_NULL();
    }

    TON_DBG_MSG("is_ton_request_finished is called for request %p\n", data);
    TON_DBG_MSG("is_ton_request_finished returning %d for request %p\n", data->finished, data);

    RETURN_BOOL(data->finished);
}
/* }}}*/

/* {{{ ?int ton_request_last_status( resource $request )
 */
PHP_FUNCTION(ton_request_last_status)
{
    zval *res;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_RESOURCE(res)
    ZEND_PARSE_PARAMETERS_END();

    ton_request_data_t* data;
    if ((data = (ton_request_data_t*)zend_fetch_resource(Z_RES_P(res), NAMEOF(ton_request_data_t), res_num)) == NULL) {
        RETURN_NULL();
    }

    TON_DBG_MSG("ton_request_last_status is called for request %p\n", data);
    TON_DBG_MSG("ton_request_last_status returning %d for request %p\n", data->last_status, data);

    RETURN_LONG(data->last_status);
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_ton_request_sync, 0, 0, 3)
    ZEND_ARG_INFO(0, context)
    ZEND_ARG_INFO(0, function_name)
    ZEND_ARG_INFO(0, params_json)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ton_request_start, 0, 0, 3)
    ZEND_ARG_INFO(0, context)
    ZEND_ARG_INFO(0, function_name)
    ZEND_ARG_INFO(0, params_json)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ton_request_next, 0, 0, 1)
    ZEND_ARG_INFO(0, request_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_is_ton_request_finished, 0, 0, 1)
    ZEND_ARG_INFO(0, request_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ton_request_last_status, 0, 0, 1)
    ZEND_ARG_INFO(0, request_id)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ ton_client_functions[]
 */
static const zend_function_entry ton_client_functions[] = {
    PHP_FE(ton_create_context,      arginfo_ton_create_context)
    PHP_FE(ton_destroy_context,     arginfo_ton_destroy_context)
    PHP_FE(ton_request_sync,        arginfo_ton_request_sync)
    PHP_FE(ton_request_start,       arginfo_ton_request_start)
    PHP_FE(ton_request_next,        arginfo_ton_request_next)
    PHP_FE(is_ton_request_finished, arginfo_is_ton_request_finished)
    PHP_FE(ton_request_last_status, arginfo_ton_request_last_status)
    PHP_FE_END
};
/* }}} */

/* {{{ ton_client_module_entry
 */
zend_module_entry ton_client_module_entry = {
    STANDARD_MODULE_HEADER,
    "ton_client",               /* Extension name */
    ton_client_functions,             /* zend_function_entry */
    PHP_MINIT(ton_client),            /* PHP_MINIT - Module initialization */
    NULL,           /* PHP_MSHUTDOWN - Module shutdown */
    PHP_RINIT(ton_client),            /* PHP_RINIT - Request initialization */
    NULL,           /* PHP_RSHUTDOWN - Request shutdown */
    PHP_MINFO(ton_client),            /* PHP_MINFO - Module info */
    PHP_TON_CLIENT_VERSION,           /* Version */
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_TON_CLIENT
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(ton_client)
#endif
