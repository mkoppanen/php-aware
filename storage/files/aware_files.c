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

#include "php_aware_files.h"

ZEND_DECLARE_MODULE_GLOBALS(aware_files)

php_aware_storage_module php_aware_storage_module_files = {
	PHP_AWARE_STORAGE_MOD(files)
};

PHP_AWARE_CONNECT_FUNC(files)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_GET_FUNC(files)
{
	char *filename, *buff;
	php_stream *stream;
	size_t buff_size;

	if (spprintf(&filename, MAXPATHLEN, "%s/%s.aware", AWARE_FILES_G(storage_path), uuid) <= 0) {
		return AwareOperationFailure;
	}
	
	aware_printf("Filename: %s\n", filename);
	
	stream = php_stream_open_wrapper(filename, "r", ENFORCE_SAFE_MODE & ~REPORT_ERRORS, NULL);
	efree(filename);
	
	if (!stream) {
		return AwareOperationFailure;
	}
	
    buff_size = php_stream_copy_to_mem(stream, &buff, PHP_STREAM_COPY_ALL, 0);
	php_stream_close(stream);

    if (!buff_size) {
		return AwareOperationFailure;
    }

	php_aware_storage_unserialize(buff, buff_size, event TSRMLS_CC);
	efree(buff);
	
	return AwareOperationSuccess;
}

PHP_AWARE_STORE_FUNC(files)
{
	char *filename;
	php_stream *stream;
	smart_str string = {0};

	if (spprintf(&filename, MAXPATHLEN, "%s/%s.aware", AWARE_FILES_G(storage_path), uuid) <= 0) {
		return AwareOperationFailure;
	}
	
	aware_printf("Storage filename: %s\n", filename);
	
	stream = php_stream_open_wrapper(filename, "w+", ENFORCE_SAFE_MODE & ~REPORT_ERRORS, NULL);
	efree(filename);
	
	php_aware_storage_serialize(uuid, event, &string TSRMLS_CC);
	
	if (!stream) {
		smart_str_free(&string);
		return AwareOperationFailure;
	}
	
	if (php_stream_write(stream, string.c, string.len) < string.len) {
		php_stream_close(stream);
		smart_str_free(&string);
		return AwareOperationFailure;
	}
	
	php_stream_close(stream);
	smart_str_free(&string);
	
	return AwareOperationSuccess;
}

/* Just the uuid part of the full path */
static int _php_aware_files_clean_path(zval **path TSRMLS_DC)
{
	if (Z_TYPE_PP(path) == IS_STRING && Z_STRLEN_PP(path) > 36) {
		char *ptr, buffer[37];
		memset(buffer, 0, 37);
		
		ptr = Z_STRVAL_PP(path) + (Z_STRLEN_PP(path) - 42);
		memcpy(buffer, ptr, 36);
		
		buffer[36] = '\0';
		efree(Z_STRVAL_PP(path));
		ZVAL_STRING(*path, buffer, 1);
		
		return ZEND_HASH_APPLY_KEEP;
	}
	return ZEND_HASH_APPLY_REMOVE;
}	

PHP_AWARE_GET_LIST_FUNC(files)
{
	char path[MAXPATHLEN];
	zval *fname;
	zval *args[1];

	/* Use php glob */
	MAKE_STD_ZVAL(fname);
	ZVAL_STRING(fname, "glob", 1);

	MAKE_STD_ZVAL(args[0]);
	snprintf(path, MAXPATHLEN, "%s/*-*-*-*-*.aware", AWARE_FILES_G(storage_path));
	
	ZVAL_STRING(args[0], path, 1);

	call_user_function(EG(function_table), NULL, fname, events, 1, args TSRMLS_CC);
	zval_dtor(fname);
	FREE_ZVAL(fname);
	
	zval_dtor(args[0]);
	FREE_ZVAL(args[0]);
	
	/* TODO: Start + limit */
	if (Z_TYPE_P(events) == IS_ARRAY) {
		zend_hash_apply(Z_ARRVAL_P(events), (apply_func_t)_php_aware_files_clean_path TSRMLS_CC);
	}
	return AwareOperationSuccess;
}

PHP_AWARE_DISCONNECT_FUNC(files)
{
	return AwareOperationNotSupported;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("aware_files.storage_path", "/tmp",	PHP_INI_PERDIR, OnUpdateString, storage_path,	zend_aware_files_globals, aware_files_globals)
PHP_INI_END()

static void php_aware_files_init_globals(zend_aware_files_globals *aware_files_globals)
{
	aware_files_globals->storage_path = NULL;	
}

/* {{{ PHP_MINIT_FUNCTION(aware_files) */
PHP_MINIT_FUNCTION(aware_files) 
{
	ZEND_INIT_MODULE_GLOBALS(aware_files, php_aware_files_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	if (PHP_AWARE_STORAGE_REGISTER(files) == AwareModuleFailed) {
		return FAILURE;
	} else {
		return SUCCESS;
	}
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(aware_files) */
PHP_MSHUTDOWN_FUNCTION(aware_files)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(aware_files) */
PHP_MINFO_FUNCTION(aware_files)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "aware_files storage", "enabled");
	php_info_print_table_row(2, "aware_files version", PHP_AWARE_FILES_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry aware_files_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry aware_files_module_entry = {
        STANDARD_MODULE_HEADER,
        "aware_files",
        aware_files_functions,
        PHP_MINIT(aware_files),
        PHP_MSHUTDOWN(aware_files),
        NULL,
        NULL,
        PHP_MINFO(aware_files),
    	PHP_AWARE_FILES_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_AWARE_FILES
ZEND_GET_MODULE(aware_files)
#endif