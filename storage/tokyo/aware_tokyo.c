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

#include "php_aware_tokyo.h"

ZEND_DECLARE_MODULE_GLOBALS(aware_tokyo)

php_aware_storage_module php_aware_storage_module_tokyo = {
	PHP_AWARE_STORAGE_MOD(tokyo)
};

PHP_AWARE_CONNECT_FUNC(tokyo)
{
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		/* TODO: non-blocking locks? */
		if (!php_aware_cabinet_open(AWARE_TOKYO_G(cabinet), AWARE_TOKYO_G(cabinet_path), TDBOWRITER|TDBOCREAT)) {
			return AwareOperationFailure;
		}
		return AwareOperationSuccess;
	}
	return AwareOperationNotSupported;
}

PHP_AWARE_GET_FUNC(tokyo)
{
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		if (!php_aware_cabinet_get(AWARE_TOKYO_G(cabinet), uuid, event)) {
			return AwareOperationFailure;
		}
		return AwareOperationSuccess;
	}
	return AwareOperationNotSupported;
}

PHP_AWARE_STORE_FUNC(tokyo)
{
	smart_str string = {0};
	php_aware_storage_serialize(event, &string TSRMLS_CC);
	
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		if (!php_aware_cabinet_put(AWARE_TOKYO_G(cabinet), uuid, string.c, string.len)) {
			return AwareOperationFailure;
		}
		return AwareOperationSuccess;
	}
	return AwareOperationNotSupported;
}

PHP_AWARE_GET_LIST_FUNC(tokyo)
{
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		if (!php_aware_cabinet_get_list(AWARE_TOKYO_G(cabinet), start, limit, events)) {
			return AwareOperationFailure;
		}
		return AwareOperationSuccess;
	}
	return AwareOperationNotSupported;
}

PHP_AWARE_DISCONNECT_FUNC(tokyo)
{
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		if (!php_aware_cabinet_close(AWARE_TOKYO_G(cabinet))) {
			return AwareOperationFailure;
		}
		return AwareOperationSuccess;
	}
	return AwareOperationNotSupported;
}

static PHP_INI_MH(OnUpdateBackend)
{
	if (new_value && new_value_length > 0) {
		if (new_value_length == 6 && !strcmp(new_value, "tyrant")) {
			AWARE_TOKYO_G(backend) = AwareTokyoBackendTyrant;			
		} else if (new_value_length == 7 && !strcmp(new_value, "cabinet")) {
			AWARE_TOKYO_G(backend) = AwareTokyoBackendCabinet;
		} else {
			AWARE_TOKYO_G(backend) = AwareTokyoBackendNotSet;
			return FAILURE;
		}
	} else {
		AWARE_TOKYO_G(backend) = AwareTokyoBackendNotSet;
	}
	return OnUpdateString(entry, new_value, new_value_length, mh_arg1, mh_arg2, mh_arg3, stage TSRMLS_CC);
}

PHP_INI_BEGIN()
	/* Possible values "cabinet", "tyrant" */
	STD_PHP_INI_ENTRY("aware_tokyo.backend", "", PHP_INI_PERDIR, OnUpdateBackend, backend_str, zend_aware_tokyo_globals, aware_tokyo_globals)
	/* Tokyo Tyrant config */
	STD_PHP_INI_ENTRY("aware_tokyo.tyrant_host", "localhost", PHP_INI_PERDIR, OnUpdateString, tyrant_host, zend_aware_tokyo_globals, aware_tokyo_globals)
	STD_PHP_INI_ENTRY("aware_tokyo.tyrant_port", "1978", PHP_INI_PERDIR, OnUpdateLong, tyrant_port, zend_aware_tokyo_globals, aware_tokyo_globals)

	/* Tokyo Cabinet config */
	STD_PHP_INI_ENTRY("aware_tokyo.cabinet_path", "/tmp/casket.tct", PHP_INI_PERDIR, OnUpdateString, cabinet_path, zend_aware_tokyo_globals, aware_tokyo_globals)
PHP_INI_END()

static void php_aware_tokyo_init_globals(zend_aware_tokyo_globals *aware_tokyo_globals)
{
	aware_tokyo_globals->backend_str = NULL;
	aware_tokyo_globals->backend = AwareTokyoBackendNotSet;
	
	/* If backend == tyrant */
	aware_tokyo_globals->tyrant_host = NULL;
	aware_tokyo_globals->tyrant_port = 0;
	
	/* If backend == cabinet */
	aware_tokyo_globals->cabinet_path = NULL;
}

/* {{{ PHP_MINIT_FUNCTION(aware_tokyo) */
PHP_MINIT_FUNCTION(aware_tokyo) 
{
	ZEND_INIT_MODULE_GLOBALS(aware_tokyo, php_aware_tokyo_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	if (php_aware_register_storage_module(php_aware_storage_module_tokyo_ptr TSRMLS_CC) == AwareModuleFailed) {
		return FAILURE;
	} else {
		/* No backend */
		if (AWARE_TOKYO_G(backend) == AwareTokyoBackendNotSet) {
			php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable aware_tokyo, no aware_tokyo.backend defined");
			return FAILURE;
		}
		
		if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
			int ecode;
			
			AWARE_TOKYO_G(cabinet) = php_aware_cabinet_init();
			
			if (!AWARE_TOKYO_G(cabinet)) {
				php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to allocate tokyo cabinet handle");
				return FAILURE;
			}
			
			if (!php_aware_cabinet_open(AWARE_TOKYO_G(cabinet), AWARE_TOKYO_G(cabinet_path), TDBOWRITER|TDBOCREAT)) {
				ecode = tctdbecode(AWARE_TOKYO_G(cabinet));
				php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to open %s: %s", AWARE_TOKYO_G(cabinet_path), tctdberrmsg(ecode));
				return FAILURE;
			}
			
			if (!php_aware_cabinet_optimize(AWARE_TOKYO_G(cabinet))) {
				ecode = tctdbecode(AWARE_TOKYO_G(cabinet));
				php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to optimize %s: %s", AWARE_TOKYO_G(cabinet_path), tctdberrmsg(ecode));
				return FAILURE;	
			}
			
			if (!php_aware_cabinet_close(AWARE_TOKYO_G(cabinet))) {
				ecode = tctdbecode(AWARE_TOKYO_G(cabinet));
				php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to close %s: %s", AWARE_TOKYO_G(cabinet_path), tctdberrmsg(ecode));
				return FAILURE;
			}
		}
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(aware_tokyo) */
PHP_MSHUTDOWN_FUNCTION(aware_tokyo)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(aware_tokyo) */
PHP_MINFO_FUNCTION(aware_tokyo)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "aware_tokyo storage", "enabled");
	php_info_print_table_row(2, "aware_tokyo version", PHP_AWARE_TOKYO_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry aware_tokyo_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry aware_tokyo_module_entry = {
        STANDARD_MODULE_HEADER,
        "aware_tokyo",
        aware_tokyo_functions,
        PHP_MINIT(aware_tokyo),
        PHP_MSHUTDOWN(aware_tokyo),
        NULL,
        NULL,
        PHP_MINFO(aware_tokyo),
    	PHP_AWARE_TOKYO_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_AWARE_TOKYO
ZEND_GET_MODULE(aware_tokyo)
#endif