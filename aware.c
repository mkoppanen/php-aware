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
#include "php_ini.h"

#ifndef Z_ADDREF_PP
# define Z_ADDREF_PP(ppz) ppz->refcount++;
#endif

/* TODO: platform dependent */
#include <uuid/uuid.h>

ZEND_DECLARE_MODULE_GLOBALS(aware)

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

static void _add_assoc_zval_helper(zval *aware_array, char *name, uint name_len)
{	
	zval **ppzval;
	if (zend_hash_find(&EG(symbol_table), name, name_len, (void **) &ppzval) == SUCCESS) {
		/* Make sure that freeing aware_array doesn't destroy superglobals */
		Z_ADDREF_PP(ppzval);
		add_assoc_zval(aware_array, name, *ppzval);
	}	
}

void php_aware_capture_error_ex(int type, const char *error_filename, const uint error_lineno, zend_bool call_orig, const char *format, va_list args)
{
	va_list args_cp;
	int len;
	char *buffer;
	zval *aware_array;
	smart_str data_var = {0};
	
	uuid_t identifier;
	char uuid_str[37];
	
	/* Create new array */
	ALLOC_INIT_ZVAL(aware_array);
	array_init(aware_array);

	/* Capture superglobals */
	if (AWARE_G(log_get)) {
		_add_assoc_zval_helper(aware_array, "_GET", sizeof("_GET"));
	}
	
	if (AWARE_G(log_post)) {
		_add_assoc_zval_helper(aware_array, "_POST", sizeof("_POST"));
	}
	
	if (AWARE_G(log_cookie)) {
		_add_assoc_zval_helper(aware_array, "_COOKIE", sizeof("_COOKIE"));
	}
	
	if (AWARE_G(log_session)) {
		_add_assoc_zval_helper(aware_array, "_SESSION", sizeof("_SESSION"));
	}
	
	if (AWARE_G(log_server)) {
		_add_assoc_zval_helper(aware_array, "_SERVER", sizeof("_SERVER"));
	}
	
	if (AWARE_G(log_env)) {
		_add_assoc_zval_helper(aware_array, "_ENV", sizeof("_ENV"));
	}
	
	/* Capture backtrace */
	if (AWARE_G(log_backtrace)) {
		zval *btrace;
		ALLOC_INIT_ZVAL(btrace);
		zend_fetch_debug_backtrace(btrace, 0, 0 TSRMLS_CC);
		add_assoc_zval(aware_array, "backtrace", btrace);
	}
	
	va_copy(args_cp, args);
	len = vspprintf(&buffer, PG(log_errors_max_len), format, args_cp);
	va_end(args_cp);

	add_assoc_string(aware_array,	"error_message",	buffer, 0);
	add_assoc_string(aware_array,	"filename",			(char *)error_filename, 1);
	
	add_assoc_long(aware_array,		"line_number",		error_lineno);
	add_assoc_long(aware_array,		"error_type",		type);
	
	/* Generate unique identifier 
		TODO: probably platform dependant atm */
	uuid_generate(identifier);
	uuid_unparse(identifier, uuid_str);

	/* Send to backend */
	php_aware_storage_store_all(uuid_str, aware_array, error_filename, error_lineno TSRMLS_CC);

	/* Free aware array */
	zval_dtor(aware_array);
	FREE_ZVAL(aware_array);
	
	/* Hand back to original error handler */
	if (call_orig) {
		AWARE_G(orig_error_cb)(type, error_filename, error_lineno, format, args);
	}
}

void php_aware_invoke_handler(int type TSRMLS_DC, const char *error_filename, const uint error_lineno, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	php_aware_capture_error_ex(type, error_filename, error_lineno, 0, format, args);
	va_end(args);
}

/* Wrapper that calls the original callback */
void php_aware_capture_error(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args)
{
	if (type & AWARE_G(log_level)) {
		php_aware_capture_error_ex(type, error_filename, error_lineno, 1, format, args);
	} else {
		AWARE_G(orig_error_cb)(type, error_filename, error_lineno, format, args);	
	}
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
	STD_PHP_INI_ENTRY("aware.enabled",			"1",	PHP_INI_PERDIR, OnUpdateBool, 	enabled,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_level",		"22519",	PHP_INI_PERDIR, OnUpdateLogLevel, log_level,	zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.depth",			"10",	PHP_INI_PERDIR, OnUpdateLong, 	depth,				zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_get",			"1",	PHP_INI_PERDIR, OnUpdateBool, 	log_get,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_post",			"1",	PHP_INI_PERDIR, OnUpdateBool, 	log_post,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_session",		"1",	PHP_INI_PERDIR, OnUpdateBool, 	log_session,		zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_cookie",		"1",	PHP_INI_PERDIR, OnUpdateBool, 	log_cookie,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_env",			"1",	PHP_INI_PERDIR, OnUpdateBool, 	log_env,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_server",		"1",	PHP_INI_PERDIR, OnUpdateBool, 	log_server,			zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.log_backtrace",	"1",	PHP_INI_PERDIR, OnUpdateBool,	log_backtrace,		zend_aware_globals, aware_globals)	
	STD_PHP_INI_ENTRY("aware.log_generated",	"1",	PHP_INI_PERDIR, OnUpdateBool,	log_generated,		zend_aware_globals, aware_globals)
	STD_PHP_INI_ENTRY("aware.storage_modules",	"",		PHP_INI_PERDIR, OnUpdateString, storage_modules,	zend_aware_globals, aware_globals)
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
}

PHP_RINIT_FUNCTION(aware)
{
	if (AWARE_G(enabled)) {
		AWARE_G(orig_error_cb) = zend_error_cb;
		zend_error_cb          =& php_aware_capture_error;
	}
	
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(aware)
{
	if (AWARE_G(enabled)) {
		zend_error_cb = AWARE_G(orig_error_cb);
	}
	return SUCCESS;
}

/* {{{ PHP_MINIT_FUNCTION(aware) */
PHP_MINIT_FUNCTION(aware) 
{
	ZEND_INIT_MODULE_GLOBALS(aware, php_aware_init_globals, NULL); 
	REGISTER_INI_ENTRIES();

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