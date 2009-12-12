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
	AwareOperationStatus retval = AwareOperationNotSupported;
	
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		int mode = TDBOWRITER|TDBOCREAT;
		
		if (!AWARE_TOKYO_G(cabinet_block)) {
			mode |= TDBOLCKNB;
		}
		if (php_aware_cabinet_open(AWARE_TOKYO_G(cabinet), AWARE_TOKYO_G(cabinet_file), mode)) {
			retval = AwareOperationSuccess;
		} else {
			retval = AwareOperationFailure;
		}
	} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
		if (php_aware_tyrant_open(AWARE_TOKYO_G(tyrant), AWARE_TOKYO_G(tyrant_host), AWARE_TOKYO_G(tyrant_port))) {
			retval = AwareOperationSuccess;
		} else {
			retval = AwareOperationFailure;
		}
	}
	return retval;
}

PHP_AWARE_GET_FUNC(tokyo)
{
	AwareOperationStatus retval = AwareOperationNotSupported;
	
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		if (php_aware_cabinet_get(AWARE_TOKYO_G(cabinet), uuid, event TSRMLS_CC)) {
			retval = AwareOperationSuccess;
		} else {
			retval = AwareOperationFailure;
		}
	} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
		if (php_aware_tyrant_get(AWARE_TOKYO_G(tyrant), uuid, event TSRMLS_CC)) {
			retval = AwareOperationSuccess;
		} else {
			retval = AwareOperationFailure;
		}
	}
	return retval;
}

PHP_AWARE_STORE_FUNC(tokyo)
{
	AwareOperationStatus retval = AwareOperationNotSupported;
	smart_str string = {0};
	
	php_aware_storage_serialize(uuid, event, &string TSRMLS_CC);
	
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		if (php_aware_cabinet_put(AWARE_TOKYO_G(cabinet), uuid, string.c, string.len)) {
			retval = AwareOperationSuccess;
		} else {
			retval = AwareOperationFailure;
		}
	} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
		if (php_aware_tyrant_put(AWARE_TOKYO_G(tyrant), uuid, string.c, string.len)) {
			retval = AwareOperationSuccess;
		} else {
			retval = AwareOperationFailure;
		}
	}
	smart_str_free(&string);
	return retval;
}

PHP_AWARE_GET_LIST_FUNC(tokyo)
{
	AwareOperationStatus retval = AwareOperationNotSupported;
	
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		if (php_aware_cabinet_get_list(AWARE_TOKYO_G(cabinet), start, limit, events)) {
			retval = AwareOperationSuccess;
		} else {
			retval = AwareOperationFailure;
		}
	} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
		if (php_aware_tyrant_get_list(AWARE_TOKYO_G(tyrant), start, limit, events)) {
			retval = AwareOperationSuccess;
		} else {
			retval = AwareOperationFailure;
		}
	}
	return retval;
}

PHP_AWARE_DISCONNECT_FUNC(tokyo)
{
	AwareOperationStatus retval = AwareOperationNotSupported;
	
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		if (php_aware_cabinet_close(AWARE_TOKYO_G(cabinet))) {
			retval = AwareOperationSuccess;
		} else {
			retval = AwareOperationFailure;
		}
	} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
		if (php_aware_tyrant_close(AWARE_TOKYO_G(tyrant))) {
			retval = AwareOperationSuccess;
		} else {
			retval = AwareOperationFailure;
		}
	}
	return retval;
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
	STD_PHP_INI_ENTRY("aware_tokyo.cabinet_file", "/tmp/casket.tct", PHP_INI_PERDIR, OnUpdateString, cabinet_file, zend_aware_tokyo_globals, aware_tokyo_globals)
	STD_PHP_INI_ENTRY("aware_tokyo.cabinet_block", "1", PHP_INI_PERDIR, OnUpdateBool, cabinet_block, zend_aware_tokyo_globals, aware_tokyo_globals)
	
PHP_INI_END()

static void php_aware_tokyo_init_globals(zend_aware_tokyo_globals *aware_tokyo_globals)
{
	aware_tokyo_globals->backend_str = NULL;
	aware_tokyo_globals->backend     = AwareTokyoBackendNotSet;
	
	/* If backend == tyrant */
	aware_tokyo_globals->tyrant_host = NULL;
	aware_tokyo_globals->tyrant_port = 0;
	
	/* If backend == cabinet */
	aware_tokyo_globals->cabinet_file  = NULL;
	aware_tokyo_globals->cabinet_block = 1;
}

static zend_bool php_aware_tokyo_init_backend(AwareTokyoBackend configured_backend) 
{
	if (configured_backend == AwareTokyoBackendCabinet) {
		int ecode;
		
		AWARE_TOKYO_G(cabinet) = php_aware_cabinet_init();
		
		if (!AWARE_TOKYO_G(cabinet)) {
			php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to allocate tokyo cabinet handle");
			return 0;
		}
		
		if (!php_aware_cabinet_open(AWARE_TOKYO_G(cabinet), AWARE_TOKYO_G(cabinet_file), TDBOWRITER|TDBOCREAT)) {
			ecode = tctdbecode(AWARE_TOKYO_G(cabinet));
			php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to open %s: %s", AWARE_TOKYO_G(cabinet_file), tctdberrmsg(ecode));
			return 0;
		}
		
		if (!php_aware_cabinet_optimize(AWARE_TOKYO_G(cabinet))) {
			ecode = tctdbecode(AWARE_TOKYO_G(cabinet));
			php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to optimize %s: %s", AWARE_TOKYO_G(cabinet_file), tctdberrmsg(ecode));
			return 0;	
		}
		
		if (!php_aware_cabinet_close(AWARE_TOKYO_G(cabinet))) {
			ecode = tctdbecode(AWARE_TOKYO_G(cabinet));
			php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to close %s: %s", AWARE_TOKYO_G(cabinet_file), tctdberrmsg(ecode));
			return 0;
		}
		return 1;
	} else if (configured_backend == AwareTokyoBackendTyrant) {
		int ecode;
		
		AWARE_TOKYO_G(tyrant) = php_aware_tyrant_init();
		
		if (!AWARE_TOKYO_G(tyrant)) {
			php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to allocate tokyo cabinet handle");
			return 0;
		}
		
		if (!php_aware_tyrant_open(AWARE_TOKYO_G(tyrant), AWARE_TOKYO_G(tyrant_host), AWARE_TOKYO_G(tyrant_port))) {
			ecode = tcrdbecode(AWARE_TOKYO_G(tyrant));
			php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to open %s:%d: %s", AWARE_TOKYO_G(tyrant_host), AWARE_TOKYO_G(tyrant_port), tcrdberrmsg(ecode));
			return 0;
		}
		
		if (!php_aware_tyrant_optimize(AWARE_TOKYO_G(tyrant))) {
			ecode = tcrdbecode(AWARE_TOKYO_G(tyrant));
			php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to optimize %s:%d: %s", AWARE_TOKYO_G(tyrant_host), AWARE_TOKYO_G(tyrant_port), tcrdberrmsg(ecode));
			return 0;	
		}
		
		if (!php_aware_tyrant_close(AWARE_TOKYO_G(tyrant))) {
			ecode = tcrdbecode(AWARE_TOKYO_G(tyrant));
			php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to close %s:%d: %s", AWARE_TOKYO_G(tyrant_host), AWARE_TOKYO_G(tyrant_port), tcrdberrmsg(ecode));
			return 0;
		}
		return 1;
		
	}
	return 0;
}

/* {{{ PHP_MINIT_FUNCTION(aware_tokyo) */
PHP_MINIT_FUNCTION(aware_tokyo) 
{
	AwareModuleRegisterStatus status;
	
	ZEND_INIT_MODULE_GLOBALS(aware_tokyo, php_aware_tokyo_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	status = PHP_AWARE_STORAGE_REGISTER(tokyo);
	
	switch (status) 
	{
		case AwareModuleRegistered:	
			if (AWARE_TOKYO_G(backend) == AwareTokyoBackendNotSet) {
				php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable aware_tokyo, no aware_tokyo.backend defined");
				return FAILURE;
			}
			if (!php_aware_tokyo_init_backend(AWARE_TOKYO_G(backend))) {
				php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to initialize the tokyo backend");
				return FAILURE;
			}
			AWARE_TOKYO_G(enabled) = 1;
		break;
		
		case AwareModuleFailed:
			AWARE_TOKYO_G(enabled) = 0;
			return FAILURE;
		break;

		case AwareModuleNotConfigured:
			AWARE_TOKYO_G(enabled) = 0;
		break;	
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(aware_tokyo) */
PHP_MSHUTDOWN_FUNCTION(aware_tokyo)
{
	UNREGISTER_INI_ENTRIES();
	
	if (AWARE_TOKYO_G(enabled)) {
		if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
			php_aware_cabinet_deinit(AWARE_TOKYO_G(cabinet));
		} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
			php_aware_tyrant_deinit(AWARE_TOKYO_G(tyrant));
		}
	}
	
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