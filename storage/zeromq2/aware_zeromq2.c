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

#include "php_aware_zeromq2.h"

ZEND_DECLARE_MODULE_GLOBALS(aware_zeromq2)

php_aware_storage_module php_aware_storage_module_zeromq2 = {
	PHP_AWARE_STORAGE_MOD(zeromq2)
};

PHP_AWARE_CONNECT_FUNC(zeromq2)
{
	int rc, linger =100;
	
	if (AWARE_ZEROMQ2_G(connected)) {
		return AwareOperationSuccess;
	}
	
	AWARE_ZEROMQ2_G(ctx) = zmq_init(1);
	
	if (!AWARE_ZEROMQ2_G(ctx)) {
		return AwareOperationFailed;
	}
	
	AWARE_ZEROMQ2_G(socket) = zmq_socket(AWARE_ZEROMQ2_G(ctx), ZMQ_PUB);
	
	if (!AWARE_ZEROMQ2_G(socket)) {
		zmq_term(AWARE_ZEROMQ2_G(ctx));
		return AwareOperationFailed;
	}
	
	rc = zmq_connect(AWARE_ZEROMQ2_G(socket), AWARE_ZEROMQ2_G(dsn));

#ifdef ZMQ_LINGER	
	(void) zmq_setsockopt(AWARE_ZEROMQ2_G(socket), ZMQ_LINGER, &linger, sizeof(int));
#endif
	return (rc == 0) ? AwareOperationSuccess : AwareOperationFailed;
}

PHP_AWARE_GET_FUNC(zeromq2)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_STORE_FUNC(zeromq2)
{
	size_t topic_len, pos = 0;
	int rc;
	zmq_msg_t msg;
	
	smart_str string = {0};
	
	php_aware_storage_serialize(uuid, event, &string TSRMLS_CC);
	
	topic_len = strlen(AWARE_ZEROMQ2_G(topic));
	
	if (topic_len) {
		rc = zmq_msg_init_size(&msg, string.len + topic_len + 1 + 1);
	} else {
		rc = zmq_msg_init_size(&msg, string.len + 1);
	}

	if (rc != 0) {
		smart_str_free(&string);
		return AwareOperationFailed;
	}
	
	if (topic_len) {
		memcpy ((void *)zmq_msg_data(&msg), AWARE_ZEROMQ2_G(topic), topic_len);
		memcpy ((void *)zmq_msg_data(&msg) + topic_len, "|", 1);
		pos = topic_len + 1;
	}
	memcpy ((void *)zmq_msg_data(&msg) + pos, string.c, string.len + 1);
	
	rc = zmq_send(AWARE_ZEROMQ2_G(socket), &msg, 0);
	
	zmq_msg_close(&msg);
	smart_str_free(&string);
	
	return (rc == 0) ? AwareOperationSuccess : AwareOperationFailed;
}

PHP_AWARE_GET_LIST_FUNC(zeromq2)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_DELETE_FUNC(zeromq2)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_DISCONNECT_FUNC(zeromq2)
{
	return AwareOperationNotSupported;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("aware_zeromq2.topic", "aware", PHP_INI_PERDIR, OnUpdateString, topic, zend_aware_zeromq2_globals, aware_zeromq2_globals)
	STD_PHP_INI_ENTRY("aware_zeromq2.dsn", "tcp://127.0.0.1:5555", PHP_INI_PERDIR, OnUpdateString, dsn, zend_aware_zeromq2_globals, aware_zeromq2_globals)
PHP_INI_END()

static void php_aware_zeromq2_init_globals(zend_aware_zeromq2_globals *aware_zeromq2_globals)
{
	aware_zeromq2_globals->dsn = NULL;
	aware_zeromq2_globals->topic = NULL;
	aware_zeromq2_globals->connected = 0;
}

/* {{{ PHP_MINIT_FUNCTION(aware_zeromq2) */
PHP_MINIT_FUNCTION(aware_zeromq2) 
{
	AwareModuleRegisterStatus reg_status;
	
	ZEND_INIT_MODULE_GLOBALS(aware_zeromq2, php_aware_zeromq2_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	reg_status = PHP_AWARE_STORAGE_REGISTER(zeromq2);

	switch (reg_status) 
	{
		case AwareModuleRegistered:	
			AWARE_ZEROMQ2_G(enabled) = 1;
		break;
		
		case AwareModuleFailed:
			AWARE_ZEROMQ2_G(enabled) = 0;
			return FAILURE;
		break;

		case AwareModuleNotConfigured:
			AWARE_ZEROMQ2_G(enabled) = 0;
		break;	
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(aware_zeromq2) */
PHP_MSHUTDOWN_FUNCTION(aware_zeromq2)
{
	if (AWARE_ZEROMQ2_G(enabled)) {
		zmq_close(AWARE_ZEROMQ2_G(socket));
		zmq_term(AWARE_ZEROMQ2_G(ctx));
	}
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(aware_zeromq2) */
PHP_MINFO_FUNCTION(aware_zeromq2)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "aware_zeromq2 storage", "enabled");
	php_info_print_table_row(2, "aware_zeromq2 version", PHP_AWARE_ZEROMQ2_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry aware_zeromq2_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry aware_zeromq2_module_entry = {
        STANDARD_MODULE_HEADER,
        "aware_zeromq2",
        aware_zeromq2_functions,
        PHP_MINIT(aware_zeromq2),
        PHP_MSHUTDOWN(aware_zeromq2),
        NULL,
        NULL,
        PHP_MINFO(aware_zeromq2),
    	PHP_AWARE_ZEROMQ2_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_AWARE_ZEROMQ2
ZEND_GET_MODULE(aware_zeromq2)
#endif