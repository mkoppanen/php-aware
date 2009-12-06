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

#ifndef _PHP_AWARE_STOMP_FUNCS_H_
# define _PHP_AWARE_STOMP_FUNCS_H_

/* {{{ php_aware_stomp_handle */
typedef struct _php_aware_stomp_handle {
	php_stream *stream;
	zend_bool connected;
} php_aware_stomp_handle;
/* }}} */

php_aware_stomp_handle *php_aware_stomp_init();

zend_bool php_aware_stomp_send_frame(php_aware_stomp_handle *handle, 
	const char *command, smart_str *headers, smart_str *body, zend_bool expect_response, smart_str *response TSRMLS_DC);

zend_bool php_aware_stomp_connect(php_aware_stomp_handle *handle, 
	const char *server_uri, const char *username, const char *password, char **err_msg, int *err_code);

zend_bool php_aware_stomp_send(php_aware_stomp_handle *handle, const char *queue_name, const char *message, long message_len);

void php_aware_stomp_disconnect(php_aware_stomp_handle *handle);

void php_aware_stomp_deinit(php_aware_stomp_handle *handle);

#endif