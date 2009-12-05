/*
   +----------------------------------------------------------------------+
   | PHP Version 5 / aware                                                |
   +----------------------------------------------------------------------+
   | Copyright (c) 2009 Mikko Koppanen                                    |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Mikko Kopppanen <mkoppanen@php.net>                          |
   +----------------------------------------------------------------------+
*/

#include "php_aware_private.h"
#include "ext/standard/php_smart_str.h"
#include "Zend/zend_builtin_functions.h"
#include "main/php_config.h"

#ifndef Z_ADDREF_PP
# define Z_ADDREF_PP(ppz) ppz->refcount++;
#endif

/* TODO: platform dependent */
#include <uuid/uuid.h>

ZEND_DECLARE_MODULE_GLOBALS(aware)

/* {{{ aware_event_trigger(int error_level, string message)
	Trigger an event
*/
PHP_FUNCTION(aware_event_trigger)
{
	char *message;
	int message_len;
	char *error_filename;
	int error_lineno = 0;
	long type; 

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &type, &message, &message_len) != SUCCESS) {
		return;
	}
	
	if (!AWARE_G(log_generated))
		RETURN_FALSE;

	error_filename = zend_get_executed_filename(TSRMLS_C);
	error_lineno   = zend_get_executed_lineno(TSRMLS_C);
	
	if (type & AWARE_G(log_level)) {
		php_aware_invoke_handler(type TSRMLS_CC, error_filename, error_lineno, message);
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ aware_event_get(string module_name, string uuid)
	Get event as an array
*/
PHP_FUNCTION(aware_event_get)
{
	char *uuid, *mod_name;
	int uuid_len, mod_name_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &mod_name, &mod_name_len, &uuid, &uuid_len) != SUCCESS) {
		return;
	}
	php_aware_storage_get(mod_name, uuid, return_value TSRMLS_CC);
	return;
}
/* }}} */

/* {{{ aware_event_get_list(string module_name[ ,int start, int limit])
	Get list of events
*/
PHP_FUNCTION(aware_event_get_list)
{
	char *mod_name;
	int mod_name_len;
	long start = 0, limit = 10;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll", &mod_name, &mod_name_len, &start, &limit) != SUCCESS) {
		return;
	}
	php_aware_storage_get_list(mod_name, start, limit, return_value TSRMLS_CC);
	return;
}
/* }}} */

/* {{{ __aware_error_handler_callback
	Callback
*/
PHP_FUNCTION(__aware_error_handler_callback)
{
	if (AWARE_G(user_error_handler)) {
		zval *args[5], retval;
	
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|zzz", &args[0], &args[1], &args[2], &args[3], &args[4]) != SUCCESS) {
			return;
		}
		
		aware_printf("Invoking error handler: %s\n", Z_STRVAL_P(AWARE_G(user_error_handler)));
		php_aware_invoke_handler(Z_LVAL_P(args[0]) TSRMLS_CC, Z_STRVAL_P(args[2]), Z_LVAL_P(args[3]), Z_STRVAL_P(args[1]));
		call_user_function(EG(function_table), NULL, AWARE_G(user_error_handler), &retval, 5, args TSRMLS_CC);
	}
}
/* }}} */

PHP_FUNCTION(aware_set_error_handler)
{
	if (AWARE_G(orig_set_error_handler)) {
		AWARE_G(orig_set_error_handler)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

		/* Take the user error handler and push to our stack */
		if (EG(user_error_handler)) {
			zval *tmp;
			
			/* Override the error handler with our callback */
			if (!strcmp(Z_STRVAL_P(EG(user_error_handler)), "__aware_error_handler_callback")) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "Do not call set_error_handler(\"__aware_error_handler_callback\")");
			}
			
			if (zend_ptr_stack_num_elements(&AWARE_G(user_error_handlers)) > 0) {
				zval *old_handler = (zval *)zend_ptr_stack_pop(&AWARE_G(user_error_handlers));
				zend_ptr_stack_push(&AWARE_G(user_error_handlers), old_handler);
			
				zval_dtor(return_value);
				ZVAL_STRING(return_value, Z_STRVAL_P(old_handler), 1);
			}
			
			MAKE_STD_ZVAL(tmp);
			ZVAL_STRING(tmp, Z_STRVAL_P(EG(user_error_handler)), 1);

			/* free previous error handler */
			if (AWARE_G(user_error_handler)) {
				zval_dtor(AWARE_G(user_error_handler));
				FREE_ZVAL(AWARE_G(user_error_handler));
			}
			
			MAKE_STD_ZVAL(AWARE_G(user_error_handler));
			ZVAL_STRING(AWARE_G(user_error_handler), Z_STRVAL_P(EG(user_error_handler)), 1);

			/* Create a new handler */
			zend_ptr_stack_push(&AWARE_G(user_error_handlers), tmp);

			zval_dtor(EG(user_error_handler));
			ZVAL_STRING(EG(user_error_handler), "__aware_error_handler_callback", 1);
		} else {
			zval_dtor(AWARE_G(user_error_handler));
			FREE_ZVAL(AWARE_G(user_error_handler));
			AWARE_G(user_error_handler) = NULL;
		}
	}
}

PHP_FUNCTION(aware_restore_error_handler)
{
	if (AWARE_G(orig_restore_error_handler)) {
		AWARE_G(orig_restore_error_handler)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

		if (AWARE_G(user_error_handler)) {
			zval_dtor(AWARE_G(user_error_handler));
			FREE_ZVAL(AWARE_G(user_error_handler));
			AWARE_G(user_error_handler) = NULL;
		}

		/* Delete the top element from our stack */
		if (zend_ptr_stack_num_elements(&AWARE_G(user_error_handlers)) > 0) {
			zval *tmp = (zval *)zend_ptr_stack_pop(&AWARE_G(user_error_handlers));
			zval_dtor(tmp);
			FREE_ZVAL(tmp);
			tmp = NULL;
			
			if (zend_ptr_stack_num_elements(&AWARE_G(user_error_handlers)) > 0) {
				tmp = (zval *)zend_ptr_stack_pop(&AWARE_G(user_error_handlers));
				zend_ptr_stack_push(&AWARE_G(user_error_handlers), tmp);
				
				if (AWARE_G(user_error_handler)) {
					zval_dtor(AWARE_G(user_error_handler));
					FREE_ZVAL(AWARE_G(user_error_handler));
				}
				MAKE_STD_ZVAL(AWARE_G(user_error_handler));
				ZVAL_STRING(AWARE_G(user_error_handler), Z_STRVAL_P(tmp), 1);
			}
		}
	}
}

static void _add_assoc_zval_helper(zval *event, char *name, uint name_len)
{	
	zval **ppzval;
	if (zend_hash_find(&EG(symbol_table), name, name_len, (void **) &ppzval) == SUCCESS) {
		/* Make sure that freeing aware_array doesn't destroy superglobals */
		Z_ADDREF_PP(ppzval);
		add_assoc_zval(event, name, *ppzval);
	}	
}

/* event must be initialized with MAKE_STD_ZVAL or similar and array_init before sending here */
void php_aware_capture_error_ex(zval *event, int type, const char *error_filename, const uint error_lineno, zend_bool free_event, const char *format, va_list args)
{
	va_list args_cp;
	int len;
	char *buffer;

	uuid_t identifier;
	char uuid_str[37];
	
	TSRMLS_FETCH();

	/* Capture superglobals */
	if (AWARE_G(log_get)) {
		_add_assoc_zval_helper(event, "_GET", sizeof("_GET"));
	}
	
	if (AWARE_G(log_post)) {
		_add_assoc_zval_helper(event, "_POST", sizeof("_POST"));
	}
	
	if (AWARE_G(log_cookie)) {
		_add_assoc_zval_helper(event, "_COOKIE", sizeof("_COOKIE"));
	}
	
	if (AWARE_G(log_session)) {
		_add_assoc_zval_helper(event, "_SESSION", sizeof("_SESSION"));
	}
	
	if (AWARE_G(log_server)) {
		_add_assoc_zval_helper(event, "_SERVER", sizeof("_SERVER"));
	}
	
	if (AWARE_G(log_env)) {
		_add_assoc_zval_helper(event, "_ENV", sizeof("_ENV"));
	}
	
	/* Capture backtrace */
	if (AWARE_G(log_backtrace)) {
		zval *btrace;
		ALLOC_INIT_ZVAL(btrace);
		zend_fetch_debug_backtrace(btrace, 0, 0 TSRMLS_CC);
		add_assoc_zval(event, "backtrace", btrace);
	}
	
	va_copy(args_cp, args);
	len = vspprintf(&buffer, PG(log_errors_max_len), format, args_cp);
	va_end(args_cp);

	add_assoc_string(event,	"error_message", buffer, 0);
	add_assoc_string(event,	"filename",	(char *)error_filename, 1);
	
	add_assoc_long(event, "line_number", error_lineno);
	add_assoc_long(event, "error_type", type);
	
	/* Generate unique identifier 
		TODO: probably platform dependant atm */
	uuid_generate(identifier);
	uuid_unparse(identifier, uuid_str);

	/* Send to backend */
	php_aware_storage_store_all(uuid_str, event, error_filename, error_lineno TSRMLS_CC);
	
	if (free_event) {
		zval_dtor(event);
		FREE_ZVAL(event);
	}
}

void php_aware_invoke_handler(int type, const char *error_filename, const uint error_lineno, const char *format, ...)
{
	zval *event;
	va_list args;

	ALLOC_INIT_ZVAL(event);
	array_init(event);
	
	va_start(args, format);
	php_aware_capture_error_ex(event, type, error_filename, error_lineno, 1, format, args);
	va_end(args);
}

/* Wrapper that calls the original callback or our callback */
void php_aware_capture_error(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args)
{
	if (type & AWARE_G(log_level)) {
		zval *event;

		ALLOC_INIT_ZVAL(event);
		array_init(event);

		php_aware_capture_error_ex(event, type, error_filename, error_lineno, 1, format, args);
		AWARE_G(orig_error_cb)(type, error_filename, error_lineno, format, args);
	} else {
		AWARE_G(orig_error_cb)(type, error_filename, error_lineno, format, args);	
	}
}

/* Wrapper that calls the original callback or our callback */
void php_aware_capture_slow_request(long elapsed, const char *format, ...)
{
	va_list args;
	zval *event, *slow_request;
	
	ALLOC_INIT_ZVAL(event);
	array_init(event);
	
	ALLOC_INIT_ZVAL(slow_request);
	array_init(slow_request);
	
	add_assoc_bool(slow_request, "is_slow_request", 1);
	add_assoc_long(slow_request, "time_elapsed", elapsed);
	add_assoc_long(slow_request, "slow_request_threshold", AWARE_G(slow_request_threshold));	
	
	add_assoc_zval(event, "_AWARE", slow_request);

	va_start(args, format);
	php_aware_capture_error_ex(event, E_CORE_WARNING, "aware internal report", 0, 1, format, args);
	va_end(args);
}

/* Aware internal errors go through here */
void php_aware_original_error_cb(int type TSRMLS_DC, const char *format, ...)
{
	char *error_filename;
	int error_lineno = 0;
	va_list args;

	error_filename = zend_get_executed_filename(TSRMLS_C);
	error_lineno   = zend_get_executed_lineno(TSRMLS_C);

	va_start(args, format);
	
	if (AWARE_G(orig_error_cb)) 
		AWARE_G(orig_error_cb)(type, error_filename, error_lineno, format, args);
	else 
		zend_error_cb(type, error_filename, error_lineno, format, args);

	va_end(args);
}

static PHP_INI_MH(OnUpdateLogLevel) 
{
	if (!new_value) {
		AWARE_G(log_level) = E_ALL & ~E_NOTICE & ~E_STRICT & ~E_DEPRECATED;
	} else {
		AWARE_G(log_level) = atoi(new_value);
	}
	return SUCCESS;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("aware.enabled",			"1",		PHP_INI_PERDIR, OnUpdateBool, 		enabled,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.error_reporting",	"22519",	PHP_INI_PERDIR, OnUpdateLogLevel,	log_level,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.depth",			"10",		PHP_INI_PERDIR, OnUpdateLong,		depth,				zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_get",			"1",		PHP_INI_PERDIR, OnUpdateBool,		log_get,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_post",			"1",		PHP_INI_PERDIR, OnUpdateBool,		log_post,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_session",		"1",		PHP_INI_PERDIR, OnUpdateBool,		log_session,		zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_cookie",		"1",		PHP_INI_PERDIR, OnUpdateBool,		log_cookie,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_env",			"1",		PHP_INI_PERDIR, OnUpdateBool,		log_env,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_server",		"1",		PHP_INI_PERDIR, OnUpdateBool,		log_server,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_backtrace",	"1",		PHP_INI_PERDIR, OnUpdateBool,		log_backtrace,		zend_aware_globals, aware_globals)	
	STD_PHP_INI_ENTRY("aware.log_generated",	"1",		PHP_INI_PERDIR, OnUpdateBool,		log_generated,		zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.storage_modules",	NULL,		PHP_INI_PERDIR, OnUpdateString,		storage_modules,	zend_aware_globals, aware_globals)

	STD_PHP_INI_ENTRY("aware.slow_request_threshold",	"0",	PHP_INI_PERDIR, OnUpdateLong,	slow_request_threshold,	zend_aware_globals, aware_globals)
PHP_INI_END()

static void php_aware_init_globals(zend_aware_globals *aware_globals)
{
	aware_globals->storage_modules	= NULL;
	aware_globals->enabled	   		= 1;

	aware_globals->log_level   		= 22519;

	aware_globals->depth	   		= 10;
	aware_globals->enabled	   		= 1;
	aware_globals->log_get	   		= 1;
	aware_globals->log_post	   		= 1;
	aware_globals->log_session 		= 1;
	aware_globals->log_cookie		= 1;
	aware_globals->log_server		= 1;
	aware_globals->log_env			= 1;
	aware_globals->log_backtrace	= 1;
	aware_globals->log_generated	= 1;
	aware_globals->slow_request_threshold = 0;
	
	aware_globals->orig_set_error_handler = NULL;
	aware_globals->user_error_handler = NULL;
}

#ifdef HAVE_GETTIMEOFDAY
static zend_bool aware_timestamp_get(struct timeval *tp)
{
	if (gettimeofday(tp, NULL)) {
		return 0;
    }
	return 1;
}
#endif

PHP_RINIT_FUNCTION(aware)
{
	if (AWARE_G(enabled)) {
		zend_function *orig_set_error_handler, *orig_restore_error_handler;
		
		AWARE_G(orig_error_cb) = zend_error_cb;
		zend_error_cb          =& php_aware_capture_error;
		
		if (zend_hash_find(EG(function_table), "set_error_handler", sizeof("set_error_handler"), (void **)&orig_set_error_handler) == SUCCESS) {
			AWARE_G(orig_set_error_handler) = orig_set_error_handler->internal_function.handler;
			orig_set_error_handler->internal_function.handler = zif_aware_set_error_handler;
		}
		if (zend_hash_find(EG(function_table), "restore_error_handler", sizeof("restore_error_handler"), (void **)&orig_restore_error_handler) == SUCCESS) {
			AWARE_G(orig_restore_error_handler) = orig_restore_error_handler->internal_function.handler;
			orig_restore_error_handler->internal_function.handler = zif_aware_restore_error_handler;
		}
		zend_ptr_stack_init(&AWARE_G(user_error_handlers));

#ifdef HAVE_GETTIMEOFDAY	
		if (AWARE_G(slow_request_threshold)) {
			if (!aware_timestamp_get(&AWARE_G(request_start))) {
				/* failed to get timestamp */
				AWARE_G(slow_request_threshold) = 0;
			}
		}
#endif
	}
	
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(aware)
{
	if (AWARE_G(enabled)) {
		zend_function *orig_set_error_handler, *orig_restore_error_handler;

#ifdef HAVE_GETTIMEOFDAY	
		if (AWARE_G(slow_request_threshold)) {
			struct timeval tp;
			if (aware_timestamp_get(&tp)) {
				/* in milliseconds */
				long elapsed;
				
				elapsed  = (long)((tp.tv_sec - AWARE_G(request_start).tv_sec) * 1000);
				elapsed += (long)((tp.tv_usec - AWARE_G(request_start).tv_usec) / 1000);
				
				if (elapsed > AWARE_G(slow_request_threshold)) {
					php_aware_capture_slow_request(elapsed, "Slow request detected (elapsed: %d ms, threshold %d ms)", 
													elapsed, AWARE_G(slow_request_threshold));
				}
			}
		}
#endif		
		zend_error_cb = AWARE_G(orig_error_cb);
		zend_ptr_stack_clean(&AWARE_G(user_error_handlers), ZVAL_DESTRUCTOR, 1);
		zend_ptr_stack_destroy(&AWARE_G(user_error_handlers));
		
		if (AWARE_G(user_error_handler)) {
			zval_dtor(AWARE_G(user_error_handler));
			FREE_ZVAL(AWARE_G(user_error_handler));
		}
		
		if (zend_hash_find(EG(function_table), "set_error_handler", sizeof("set_error_handler"), (void **)&orig_set_error_handler) == SUCCESS) {
			orig_set_error_handler->internal_function.handler = AWARE_G(orig_set_error_handler);
		}
		if (zend_hash_find(EG(function_table), "restore_error_handler", sizeof("restore_error_handler"), (void **)&orig_restore_error_handler) == SUCCESS) {
			orig_restore_error_handler->internal_function.handler = AWARE_G(orig_restore_error_handler);
		}
	}
	return SUCCESS;
}

/* {{{ PHP_MINIT_FUNCTION(aware) */
PHP_MINIT_FUNCTION(aware) 
{
	ZEND_INIT_MODULE_GLOBALS(aware, php_aware_init_globals, NULL); 
	REGISTER_INI_ENTRIES();
	
	if (!AWARE_G(storage_modules))
		AWARE_G(enabled) = 0;
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(aware) */
PHP_MSHUTDOWN_FUNCTION(aware)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(aware) */
PHP_MINFO_FUNCTION(aware)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "aware extension", "enabled");
	php_info_print_table_row(2, "aware version", PHP_AWARE_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry aware_functions[] = {
	PHP_FE(aware_event_trigger, NULL)
	PHP_FE(aware_event_get, NULL)
	PHP_FE(aware_event_get_list, NULL)
	PHP_FE(__aware_error_handler_callback, NULL)
	{NULL, NULL, NULL}
};

zend_module_entry aware_module_entry = {
        STANDARD_MODULE_HEADER,
        "aware",
        aware_functions,
        PHP_MINIT(aware),
        PHP_MSHUTDOWN(aware),
        PHP_RINIT(aware),
        PHP_RSHUTDOWN(aware),
        PHP_MINFO(aware),
    	PHP_AWARE_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_AWARE
ZEND_GET_MODULE(aware)
#endif