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

// MAX number of unprocessed callback handler calls per single TON request.
// In other words, its the MAX number of times the callback can be received
// before previous callbacks have been processes via calling function ton_request_next.
// Worth keeping this number relatively low.

#define CALLBACK_QUEUE_CAPACITY 1024

static zend_long TON_REQUEST_NEXT_ID = 1;
static zend_llist unused_requests;

typedef struct ton_request_data {
    zend_long id;
    rpa_queue_t * queue;
    bool finished;
    bool unused;
    int last_status;
    struct ton_request_data *joined_to;
} ton_request_data_t;

static ton_request_data_t *ton_request_data_create() {
    ton_request_data_t *data = calloc(1, sizeof(ton_request_data_t));
    data->id = TON_REQUEST_NEXT_ID++;
    data->last_status = -1;
    rpa_queue_create(&data->queue, CALLBACK_QUEUE_CAPACITY);
    return data;
}

// Queue element structure.
// Blocking queue is used to operate with the core ton client callbacks.
// PHP client calls ton_request_next func which blocks until the next callback
// JSON is pushed to the queue.

typedef struct ton_callback_queue_element {
    char *json;
    uint32_t len;
    uint32_t status;
    bool finished;
    ton_request_data_t *data;
} ton_callback_queue_element_t;

static ton_callback_queue_element_t *ton_callback_queue_element_create(
        tc_string_data_t params_json,
        int response_type,
        bool finished,
        ton_request_data_t *data) {
    ton_callback_queue_element_t *e = malloc(sizeof(ton_callback_queue_element_t));
    e->json = malloc(params_json.len);
    e->len = params_json.len;
    memcpy(e->json, params_json.content, params_json.len);
    e->status = response_type;
    e->finished = finished;
    e->data = data;
    return e;
}

static void ton_callback_queue_element_free(ton_callback_queue_element_t *e) {
    free(e->json);
    free(e);
}

static void ton_request_data_shutdown_queue(ton_request_data_t *data) {
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

static void ton_request_data_free(ton_request_data_t *data) {
    TON_DBG_MSG("in ton_request_data_free: %p\n", data);
    if (!data->finished) {
        TON_DBG_MSG("WARNING: request %p is not finished yet. Possible cause of segfault. Skipping.\n", data);
        return;
    }
    if (data->queue) {
        ton_request_data_shutdown_queue(data);
    }
    free(data);
}

static void ton_free_unused_request_data(void **ptr)
{
    TON_DBG_MSG("freeing unused request data ptr\n");
    ton_request_data_free((ton_request_data_t *)*ptr);
}

static void response_queueing_handler(
        void *request_ptr,
        tc_string_data_t params_json,
        uint32_t response_type,
        bool finished) {

    TON_DBG_MSG("response_queueing_handler called with request=%p, status=%d, finished=%d\n",
                request_ptr, response_type, finished);

    ton_request_data_t *data = request_ptr;
    if (data->unused) {
        TON_DBG_MSG("request %p is not used anymore\n", request_ptr);
        data->last_status = response_type;
        data->finished = finished;
        // Don't queue unused request data
        return;
    }

    ton_callback_queue_element_t *e = ton_callback_queue_element_create(
            params_json, response_type, finished, data);

    if (data->joined_to) {
        data = data->joined_to;
    }

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

static void ton_resource_destructor(zend_resource *rsrc) /* {{{ */
{
    TON_DBG_MSG("in ton_resource_destructor: %p\n", rsrc->ptr);
    if (rsrc->ptr) {
        ton_request_data_t *data = (ton_request_data_t *) rsrc->ptr;
        if (data->finished) {
            ton_request_data_free(data);
        } else {
            TON_DBG_MSG("%p request marked as unused\n", rsrc->ptr);
            data->unused = true;
            zend_llist_add_element(&unused_requests, &data);
        }
        rsrc->ptr = NULL;
    }
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

    TON_DBG_MSG("ton_request_sync is called with arguments %ld, %s, %s\n",
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

    TON_DBG_MSG("ton_request_start is called with arguments %ld, %s, %s\n",
                context,
                ZSTR_VAL(function_name),
                ZSTR_VAL(params_json));

    ton_request_data_t* payload = ton_request_data_create();
    tc_string_data_t f_name = {ZSTR_VAL(function_name), ZSTR_LEN(function_name)};
    tc_string_data_t f_params = {ZSTR_VAL(params_json), ZSTR_LEN(params_json)};
    tc_request_ptr(context, f_name, f_params, payload, &response_queueing_handler);

    TON_DBG_MSG("ton_request_start returned with resource %p\n", payload);

    zend_resource *resource = zend_register_resource(payload, res_num);
    RETURN_RES(resource);
}
/* }}}*/

/* {{{ int ton_request_id( resource $request )
 */
PHP_FUNCTION(ton_request_id)
{
    zval *res;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_RESOURCE(res)
    ZEND_PARSE_PARAMETERS_END();

    ton_request_data_t * data;
    if ((data = (ton_request_data_t*)zend_fetch_resource(Z_RES_P(res), "ton_request_data_t", res_num)) == NULL) {
        RETURN_NULL();
    }

    TON_DBG_MSG("ton_request_id is called for request %p\n", data);
    TON_DBG_MSG("ton_request_id (%p): return %ld\n", data, data->id);
    RETURN_LONG(data->id);
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
    if ((data = (ton_request_data_t*)zend_fetch_resource(Z_RES_P(res), "ton_request_data_t", res_num)) == NULL) {
        RETURN_NULL();
    }

    TON_DBG_MSG("ton_request_next is called for request %p\n", data);
    TON_DBG_MSG("Calling rpa_queue_pop for request %p; timeout = %ld\n", data, timeout);
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

    // returning tuple [json, status, finished, resource]
    zval json, status, finished, id;
    ZVAL_STRINGL(&json, e->json, e->len);
    ZVAL_LONG(&status, e->status);
    ZVAL_BOOL(&finished, e->finished);
    ZVAL_LONG(&id, e->data->id);
    HashTable *tuple = zend_new_array(4);
    zend_hash_next_index_insert(tuple, &json);
    zend_hash_next_index_insert(tuple, &status);
    zend_hash_next_index_insert(tuple, &finished);
    zend_hash_next_index_insert(tuple, &id);

    ton_callback_queue_element_free(e);
    TON_DBG_MSG("ton_request_next (%p) finished\n", data);
    RETURN_ARR(tuple);
}
/* }}}*/

/* {{{ bool ton_request_join( resource $request, resource $request2 )
 */
PHP_FUNCTION(ton_request_join)
{
    zval *res;
    zval *res2;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_RESOURCE(res)
    Z_PARAM_RESOURCE(res2)
    ZEND_PARSE_PARAMETERS_END();

    ton_request_data_t * data, *data2;
    if ((data = (ton_request_data_t*)zend_fetch_resource(Z_RES_P(res), "ton_request_data_t", res_num)) == NULL ||
        (data2 = (ton_request_data_t*)zend_fetch_resource(Z_RES_P(res2), "ton_request_data_t", res_num)) == NULL) {
        if (data == NULL) TON_DBG_MSG("Invalid resource for $request\n");
        if (data2 == NULL) TON_DBG_MSG("Invalid resource for $request2\n");
        RETURN_FALSE
    }

    TON_DBG_MSG("ton_request_join is called for requests %p, %p\n", data, data2);
    if (!data2->joined_to) {
        data2->joined_to = data;
        TON_DBG_MSG("request %p started to receive all events of request %p\n", data, data2);
        RETURN_TRUE
    } else {
        TON_DBG_MSG("Request %p already used for join", data2);
        RETURN_FALSE
    }
}
/* }}}*/

/* {{{ bool ton_request_disconnect( resource $request, resource $request2 )
 */
PHP_FUNCTION(ton_request_disconnect)
{
    zval *res;
    zval *res2;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_RESOURCE(res)
    Z_PARAM_RESOURCE(res2)
    ZEND_PARSE_PARAMETERS_END();

    ton_request_data_t * data, *data2;
    if ((data = (ton_request_data_t*)zend_fetch_resource(Z_RES_P(res), "ton_request_data_t", res_num)) == NULL ||
        (data2 = (ton_request_data_t*)zend_fetch_resource(Z_RES_P(res2), "ton_request_data_t", res_num)) == NULL) {
        if (data == NULL) TON_DBG_MSG("Invalid resource for $request\n");
        if (data2 == NULL) TON_DBG_MSG("Invalid resource for $request2\n");
        RETURN_FALSE
    }

    TON_DBG_MSG("ton_request_disconnect is called for requests %p, %p\n", data, data2);
    if (data2->joined_to == data){
        data2->joined_to = NULL;
        TON_DBG_MSG("request %p disconnected from %p\n", data, data2);
        RETURN_TRUE
    } else {
        TON_DBG_MSG("WARNING: request %p was not joined to %p\n", data, data2);
        RETURN_FALSE
    }
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
    if ((data = (ton_request_data_t*)zend_fetch_resource(Z_RES_P(res), "ton_request_data_t", res_num)) == NULL) {
        RETURN_NULL();
    }

    TON_DBG_MSG("is_ton_request_finished is called for request %p\n", data);

    uint32_t size = rpa_queue_size(data->queue);
    bool result = data->finished && size == 0;
    TON_DBG_MSG("is_ton_request_finished returning %d for request %p (finished: %d, queue size: %d)\n",
                result, data, data->finished, size);

    RETURN_BOOL(result);
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
    if ((data = (ton_request_data_t*)zend_fetch_resource(Z_RES_P(res), "ton_request_data_t", res_num)) == NULL) {
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
    TON_DBG_MSG("in RINIT\n");
#if defined(ZTS) && defined(COMPILE_DL_TON_CLIENT)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(ton_client)
{
    TON_DBG_MSG("in RSHUTDOWN\n");
    TON_DBG_MSG("cleaning up unused requests\n");
    zend_llist_clean(&unused_requests);
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

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(ton_client)
{
    TON_DBG_MSG("in MINIT\n");
    zend_llist_init(&unused_requests, sizeof(ton_request_data_t *), (llist_dtor_func_t) ton_free_unused_request_data, 0);
    res_num = zend_register_list_destructors_ex(ton_resource_destructor, NULL, "ton_request_data_t", module_number);
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(ton_client)
{
    return SUCCESS;
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_ton_request_id, 0, 0, 1)
    ZEND_ARG_INFO(0, request_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ton_request_next, 0, 0, 1)
    ZEND_ARG_INFO(0, request_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ton_request_join, 0, 0, 2)
    ZEND_ARG_INFO(0, request_id)
    ZEND_ARG_INFO(0, join_request_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ton_request_disconnect, 0, 0, 2)
    ZEND_ARG_INFO(0, request_id)
    ZEND_ARG_INFO(0, join_request_id)
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
    PHP_FE(ton_request_id,          arginfo_ton_request_id)
    PHP_FE(ton_request_next,        arginfo_ton_request_next)
    PHP_FE(ton_request_join,        arginfo_ton_request_join)
    PHP_FE(ton_request_disconnect,  arginfo_ton_request_disconnect)
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
    PHP_MSHUTDOWN(ton_client),        /* PHP_MSHUTDOWN - Module shutdown */
    PHP_RINIT(ton_client),            /* PHP_RINIT - Request initialization */
    PHP_RSHUTDOWN(ton_client),      /* PHP_RSHUTDOWN - Request shutdown */
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
