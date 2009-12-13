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
#include "ext/standard/php_filestat.h"

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
	char filename[MAXPATHLEN], *buff;
	php_stream *stream;
	size_t buff_size;
	zend_bool status;

	if (snprintf(filename, MAXPATHLEN, "%s/%s.aware", AWARE_FILES_G(storage_path), uuid) <= 0) {
		return AwareOperationFailure;
	}
	
	aware_printf("Filename: %s\n", filename);
	
	stream = php_stream_open_wrapper(filename, "r", ENFORCE_SAFE_MODE & ~REPORT_ERRORS, NULL);
	
	if (!stream) {
		return AwareOperationFailure;
	}
	
    buff_size = php_stream_copy_to_mem(stream, &buff, PHP_STREAM_COPY_ALL, 0);
	php_stream_close(stream);

    if (!buff_size) {
		return AwareOperationFailure;
    }

	status = php_aware_storage_unserialize(buff, buff_size, event TSRMLS_CC);
	efree(buff);
	
	if (!status) {
		return AwareOperationFailure;
	}
	return AwareOperationSuccess;
}

PHP_AWARE_STORE_FUNC(files)
{
	char filename[MAXPATHLEN];
	php_stream *stream;
	smart_str string = {0};

	if (snprintf(filename, MAXPATHLEN, "%s/%s.aware", AWARE_FILES_G(storage_path), uuid) <= 0) {
		return AwareOperationFailure;
	}
	
	aware_printf("Storage filename: %s\n", filename);
	
	stream = php_stream_open_wrapper(filename, "w+", ENFORCE_SAFE_MODE & ~REPORT_ERRORS, NULL);
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

static int php_aware_sort_mtime(const void *a, const void *b TSRMLS_DC) /* {{{ */
{
	Bucket *f, *s;
	zval *first, *second, *stat_f, *stat_s;
	int result;

	f = *((Bucket **) a);
	s = *((Bucket **) b);
	
	first = *((zval **) f->pData);
	second = *((zval **) s->pData);
	
	convert_to_string(first);
	convert_to_string(second);
	
	MAKE_STD_ZVAL(stat_f);
	MAKE_STD_ZVAL(stat_s);
	
	php_stat(Z_STRVAL_P(first), Z_STRLEN_P(first), FS_MTIME, stat_f TSRMLS_CC);
	php_stat(Z_STRVAL_P(second), Z_STRLEN_P(second), FS_MTIME, stat_s TSRMLS_CC);

	result = (Z_LVAL_P(stat_f) < Z_LVAL_P(stat_s));
	
	zval_dtor(stat_f);
	FREE_ZVAL(stat_f);
	
	zval_dtor(stat_s);
	FREE_ZVAL(stat_s);

	return result;
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
	
	if (Z_TYPE_P(events) == IS_ARRAY) {
		
		if (zend_hash_sort(Z_ARRVAL_P(events), zend_qsort, php_aware_sort_mtime, 0 TSRMLS_CC) == FAILURE) {
			return AwareOperationFailure;
		}
		
		if (zend_hash_num_elements(Z_ARRVAL_P(events)) > limit) {
			
			zval *slice_args[3], *events_copy;
		
			MAKE_STD_ZVAL(fname);
			ZVAL_STRING(fname, "array_slice", 1);

			slice_args[0] = events;
		
			MAKE_STD_ZVAL(slice_args[1]);
			ZVAL_LONG(slice_args[1], start);
		
			MAKE_STD_ZVAL(slice_args[2]);
			ZVAL_LONG(slice_args[2], limit);
		
			MAKE_STD_ZVAL(events_copy);
			call_user_function(EG(function_table), NULL, fname, events_copy, 3, slice_args TSRMLS_CC);
		
			zval_dtor(fname);
			FREE_ZVAL(fname);

			zval_dtor(events);
			ZVAL_ZVAL(events, events_copy, 1, 1);

			zval_dtor(slice_args[1]);
			FREE_ZVAL(slice_args[1]);
		
			zval_dtor(slice_args[2]);
			FREE_ZVAL(slice_args[2]);
		}
		zend_hash_apply(Z_ARRVAL_P(events), (apply_func_t)_php_aware_files_clean_path TSRMLS_CC);
	}
	
	return AwareOperationSuccess;
}

PHP_AWARE_DELETE_FUNC(files)
{
	AwareOperationStatus status;
	char path[MAXPATHLEN];
	int path_len;
	zval *stat;

	path_len = snprintf(path, MAXPATHLEN, "%s/%s.aware", AWARE_FILES_G(storage_path), uuid);
	
	MAKE_STD_ZVAL(stat);
	php_stat(path, path_len, FS_IS_FILE, stat TSRMLS_CC);
	
	status = AwareOperationFailure;
	
	if (Z_BVAL_P(stat)) {
		if (VCWD_UNLINK(path) == SUCCESS) {
			status = AwareOperationSuccess;
		}
	}
	zval_dtor(stat);
	FREE_ZVAL(stat);
	return status;
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
		int retval = SUCCESS;
		zval *stat;
		
		MAKE_STD_ZVAL(stat);
		php_stat(AWARE_FILES_G(storage_path), strlen(AWARE_FILES_G(storage_path)), FS_IS_DIR, stat TSRMLS_CC);
		
		if (Z_TYPE_P(stat) != IS_BOOL || !Z_BVAL_P(stat)) {
			php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable aware_files. %s is not a director", AWARE_FILES_G(storage_path));
			retval = FAILURE;
		}
		zval_dtor(stat);
		FREE_ZVAL(stat);
		return retval;
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