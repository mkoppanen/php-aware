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

#include "php_aware_skeleton.h"

ZEND_DECLARE_MODULE_GLOBALS(aware_skeleton)

php_aware_storage_module php_aware_storage_module_skeleton = {
	PHP_AWARE_STORAGE_MOD(skeleton)
};

PHP_AWARE_CONNECT_FUNC(skeleton)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_GET_FUNC(skeleton)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_STORE_FUNC(skeleton)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_GET_MULTI_FUNC(skeleton)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_DISCONNECT_FUNC(skeleton)
{
	return AwareOperationNotSupported;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("aware_skeleton.foobar", "somevalue", PHP_INI_PERDIR, OnUpdateString, foobar, zend_aware_skeleton_globals, aware_skeleton_globals)
PHP_INI_END()

static void php_aware_skeleton_init_globals(zend_aware_skeleton_globals *aware_skeleton_globals)
{
	aware_skeleton_globals->foobar = NULL;	
}

/* {{{ PHP_MINIT_FUNCTION(aware_skeleton) */
PHP_MINIT_FUNCTION(aware_skeleton) 
{
	ZEND_INIT_MODULE_GLOBALS(aware_skeleton, php_aware_skeleton_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	if (php_aware_register_storage_module(php_aware_storage_module_skeleton_ptr) == SUCCESS) {
		aware_printf("Registered skeleton module successfully\n");
		return SUCCESS;
	} else {
		return FAILURE;
	}
	
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(aware_skeleton) */
PHP_MSHUTDOWN_FUNCTION(aware_skeleton)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(aware_skeleton) */
PHP_MINFO_FUNCTION(aware_skeleton)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "aware_skeleton storage", "enabled");
	php_info_print_table_row(2, "aware_skeleton version", PHP_AWARE_SKELETON_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry aware_skeleton_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry aware_skeleton_module_entry = {
        STANDARD_MODULE_HEADER,
        "aware_skeleton",
        aware_skeleton_functions,
        PHP_MINIT(aware_skeleton),
        PHP_MSHUTDOWN(aware_skeleton),
        NULL,
        NULL,
        PHP_MINFO(aware_skeleton),
    	PHP_AWARE_SKELETON_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_AWARE_SKELETON
ZEND_GET_MODULE(aware_skeleton)
#endif