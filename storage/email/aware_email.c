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

#include "php_aware_email.h"

#include "main/php_output.h"

ZEND_DECLARE_MODULE_GLOBALS(aware_email)

php_aware_storage_module php_aware_storage_module_email = {
	PHP_AWARE_STORAGE_MOD(email)
};

PHP_AWARE_CONNECT_FUNC(email)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_GET_FUNC(email)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_STORE_FUNC(email)
{
	zval *fname, *args[3], *retval, **ppzval;
	char *buffer;
	int buffer_len;

	/*
		Error body
	*/
	MAKE_STD_ZVAL(args[2]);
	php_start_ob_buffer(NULL, 4096, 0 TSRMLS_CC);
	php_var_dump(&event, AWARE_G(depth), 0 TSRMLS_CC);

	if (php_ob_get_buffer(args[2] TSRMLS_CC) == FAILURE) {
		zval_dtor(args[2]);
		FREE_ZVAL(args[2]);
		
		php_end_ob_buffer(0, 0 TSRMLS_CC);
		
		return AwareOperationFailure;
    }
	php_end_ob_buffer(0, 0 TSRMLS_CC);

	MAKE_STD_ZVAL(fname);
	ZVAL_STRING(fname, "mail", 1);

	/*
		Recipient
	*/
	MAKE_STD_ZVAL(args[0]);
	ZVAL_STRING(args[0], AWARE_EMAIL_G(to_address), 1);

	/*
		Subject
	*/
	MAKE_STD_ZVAL(args[1]);

	if (zend_hash_find(Z_ARRVAL_P(event), "error_message", sizeof("error_message"), (void **) &ppzval) == SUCCESS) {
		ZVAL_STRING(args[1], Z_STRVAL_PP(ppzval), 1);
	} else {
		ZVAL_STRING(args[1], "Aware: No error message", 1);
	}

	MAKE_STD_ZVAL(retval);
	call_user_function(EG(function_table), NULL, fname, retval, 3, args TSRMLS_CC);

	zval_dtor(fname);
	FREE_ZVAL(fname);

	zval_dtor(retval);
	FREE_ZVAL(retval);

	zval_dtor(args[0]);
	FREE_ZVAL(args[0]);

	zval_dtor(args[1]);
	FREE_ZVAL(args[1]);

	zval_dtor(args[2]);
	FREE_ZVAL(args[2]);
	
	return AwareOperationSuccess;
}

PHP_AWARE_GET_LIST_FUNC(email)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_DELETE_FUNC(email)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_DISCONNECT_FUNC(email)
{
	return AwareOperationNotSupported;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("aware_email.to_address", NULL, PHP_INI_PERDIR, OnUpdateString, to_address, zend_aware_email_globals, aware_email_globals)
PHP_INI_END()

static void php_aware_email_init_globals(zend_aware_email_globals *aware_email_globals)
{
	aware_email_globals->to_address = NULL;
}

/* {{{ PHP_MINIT_FUNCTION(aware_email) */
PHP_MINIT_FUNCTION(aware_email) 
{
	AwareModuleRegisterStatus reg_status;
	
	ZEND_INIT_MODULE_GLOBALS(aware_email, php_aware_email_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	if (PHP_AWARE_STORAGE_REGISTER(email) == AwareModuleFailed) {
		return FAILURE;
	}
	
	if (!AWARE_EMAIL_G(to_address)) {
		php_aware_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable aware_email, missing aware_email.to_address");
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(aware_email) */
PHP_MSHUTDOWN_FUNCTION(aware_email)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(aware_email) */
PHP_MINFO_FUNCTION(aware_email)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "aware_email storage", "enabled");
	php_info_print_table_row(2, "aware_email version", PHP_AWARE_EMAIL_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry aware_email_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry aware_email_module_entry = {
        STANDARD_MODULE_HEADER,
        "aware_email",
        aware_email_functions,
        PHP_MINIT(aware_email),
        PHP_MSHUTDOWN(aware_email),
        NULL,
        NULL,
        PHP_MINFO(aware_email),
    	PHP_AWARE_EMAIL_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_AWARE_EMAIL
ZEND_GET_MODULE(aware_email)
#endif