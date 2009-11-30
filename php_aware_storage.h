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

#ifndef _PHP_AWARE_STORAGE_H_
# define _PHP_AWARE_STORAGE_H_

#include "php_aware.h"
#include "ext/standard/php_smart_str.h"

#define PHP_AWARE_CONNECT_ARGS		TSRMLS_D
#define PHP_AWARE_GET_ARGS		const char *uuid, zval *event TSRMLS_DC
#define PHP_AWARE_STORE_ARGS		const char *uuid, zval *event, const char *error_filename, long error_lineno TSRMLS_DC
#define PHP_AWARE_GET_MULTI_ARGS	long start, long limit, zval *events, long *num_found TSRMLS_DC
#define PHP_AWARE_DISCONNECT_ARGS	TSRMLS_D

typedef enum _AwareOperationStatus {
	AwareOperationFailure,
	AwareOperationSuccess,
	AwareOperationNotSupported
} AwareOperationStatus;

/* Storage structure for storing the events */
typedef struct _php_aware_storage_module {
	char *name;
	AwareOperationStatus  (*connect)(PHP_AWARE_CONNECT_ARGS);
	AwareOperationStatus  (*get)(PHP_AWARE_GET_ARGS);
	AwareOperationStatus  (*store)(PHP_AWARE_STORE_ARGS);
	AwareOperationStatus  (*get_multi)(PHP_AWARE_GET_MULTI_ARGS);
	AwareOperationStatus  (*disconnect)(PHP_AWARE_DISCONNECT_ARGS);
} php_aware_storage_module;

/* Register a storage module */
zend_bool php_aware_register_storage_module(php_aware_storage_module * TSRMLS_DC);

/* Find a storage module */
php_aware_storage_module *php_aware_find_storage_module(const char * TSRMLS_DC);

/* Send the event, takes care of reading configured modules and propagates the event */
void php_aware_storage_store(php_aware_storage_module *, const char *, zval *, const char *, long TSRMLS_DC);

/* Get from storage */
void php_aware_storage_get(const char *, const char *, zval *return_value TSRMLS_DC);

/* Serialize event */
zend_bool php_aware_storage_serialize(zval *event, smart_str *data_var TSRMLS_DC);

/* Function declaration macros */
#define PHP_AWARE_CONNECT_FUNC(mod_name)	AwareOperationStatus php_aware_storage_connect_##mod_name(PHP_AWARE_CONNECT_ARGS)

#define PHP_AWARE_GET_FUNC(mod_name)		AwareOperationStatus php_aware_storage_get_##mod_name(PHP_AWARE_GET_ARGS)

#define PHP_AWARE_STORE_FUNC(mod_name)		AwareOperationStatus php_aware_storage_store_##mod_name(PHP_AWARE_STORE_ARGS)

#define PHP_AWARE_GET_MULTI_FUNC(mod_name)	AwareOperationStatus php_aware_storage_get_multi_##mod_name(PHP_AWARE_GET_MULTI_ARGS)

#define PHP_AWARE_DISCONNECT_FUNC(mod_name)	AwareOperationStatus php_aware_storage_disconnect_##mod_name(PHP_AWARE_DISCONNECT_ARGS)

#define PHP_AWARE_STORAGE_FUNCS(mod_name) \
	PHP_AWARE_CONNECT_FUNC(mod_name); \
	PHP_AWARE_GET_FUNC(mod_name); \
	PHP_AWARE_STORE_FUNC(mod_name); \
	PHP_AWARE_GET_MULTI_FUNC(mod_name); \
	PHP_AWARE_DISCONNECT_FUNC(mod_name); 
		
#define PHP_AWARE_STORAGE_MOD(mod_name) \
	#mod_name, php_aware_storage_connect_##mod_name, php_aware_storage_get_##mod_name, php_aware_storage_store_##mod_name, \
	php_aware_storage_get_multi_##mod_name, php_aware_storage_disconnect_##mod_name

#endif