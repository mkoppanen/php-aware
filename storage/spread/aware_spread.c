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

#include "php_aware_spread.h"

ZEND_DECLARE_MODULE_GLOBALS(aware_spread)

php_aware_storage_module php_aware_storage_module_spread = {
	PHP_AWARE_STORAGE_MOD(spread)
};

PHP_AWARE_CONNECT_FUNC(spread)
{	
	if (!AWARE_SPREAD_G(connected)) {
		int retval;
		char private_group[MAX_GROUP_NAME];
		
		retval = SP_connect(AWARE_SPREAD_G(spread_name), AWARE_SPREAD_G(user_name), 0, 1, &AWARE_SPREAD_G(spread_mailbox), private_group);
	
		if (retval < 0) {
			return AwareOperationFailed;
		}
		AWARE_SPREAD_G(connected) = 1;
	}
	
	return AwareOperationSuccess;
}

PHP_AWARE_GET_FUNC(spread)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_STORE_FUNC(spread)
{
	int retval;
	smart_str string = {0};
	
	php_aware_storage_serialize(uuid, event, &string TSRMLS_CC);
	
	retval = SP_multicast(AWARE_SPREAD_G(spread_mailbox), AGREED_MESS, AWARE_SPREAD_G(group_name), 1, string.len, string.c);
	smart_str_free(&string);
	
	return (retval < 0) ? AwareOperationFailed : AwareOperationSuccess;
}

PHP_AWARE_GET_LIST_FUNC(spread)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_DELETE_FUNC(spread)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_DISCONNECT_FUNC(spread)
{
	return AwareOperationNotSupported;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("aware_spread.spread_name",	NULL, PHP_INI_PERDIR, OnUpdateString, spread_name, zend_aware_spread_globals, aware_spread_globals)
	STD_PHP_INI_ENTRY("aware_spread.group_name",	NULL, PHP_INI_PERDIR, OnUpdateString, group_name, zend_aware_spread_globals, aware_spread_globals)
	STD_PHP_INI_ENTRY("aware_spread.user_name",		NULL, PHP_INI_PERDIR, OnUpdateString, user_name, zend_aware_spread_globals, aware_spread_globals)
PHP_INI_END()

static void php_aware_spread_init_globals(zend_aware_spread_globals *aware_spread_globals)
{
	aware_spread_globals->spread_name = NULL;
	aware_spread_globals->group_name  = NULL;
	aware_spread_globals->user_name   = NULL;
	
	aware_spread_globals->connected   = 0;
}

/* {{{ PHP_MINIT_FUNCTION(aware_spread) */
PHP_MINIT_FUNCTION(aware_spread) 
{
	AwareModuleRegisterStatus reg_status;
	
	ZEND_INIT_MODULE_GLOBALS(aware_spread, php_aware_spread_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	reg_status = PHP_AWARE_STORAGE_REGISTER(spread);
	
	switch (reg_status) 
	{
		case AwareModuleRegistered:	
			AWARE_SPREAD_G(enabled) = 1;
			
			if (!AWARE_SPREAD_G(spread_name)) {
				php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable aware_spread, missing aware_spread.spread_name");
				return FAILURE;
			}
			
			if (!AWARE_SPREAD_G(group_name)) {
				php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable aware_spread, missing aware_spread.group_name");
				return FAILURE;
			}
			
			if (!AWARE_SPREAD_G(user_name)) {
				php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable aware_spread, missing aware_spread.user_name");
				return FAILURE;
			}

		break;
		
		case AwareModuleFailed:
			AWARE_SPREAD_G(enabled) = 0;
			return FAILURE;
		break;

		case AwareModuleNotConfigured:
			AWARE_SPREAD_G(enabled) = 0;
		break;	
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(aware_spread) */
PHP_MSHUTDOWN_FUNCTION(aware_spread)
{
	UNREGISTER_INI_ENTRIES();
	
	if (AWARE_SPREAD_G(connected)) {
		SP_disconnect(AWARE_SPREAD_G(spread_mailbox));
		AWARE_SPREAD_G(connected) = 0;
	}
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(aware_spread) */
PHP_MINFO_FUNCTION(aware_spread)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "aware_spread storage", "enabled");
	php_info_print_table_row(2, "aware_spread version", PHP_AWARE_SPREAD_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry aware_spread_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry aware_spread_module_entry = {
        STANDARD_MODULE_HEADER,
        "aware_spread",
        aware_spread_functions,
        PHP_MINIT(aware_spread),
        PHP_MSHUTDOWN(aware_spread),
        NULL,
        NULL,
        PHP_MINFO(aware_spread),
    	PHP_AWARE_SPREAD_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_AWARE_SPREAD
ZEND_GET_MODULE(aware_spread)
#endif