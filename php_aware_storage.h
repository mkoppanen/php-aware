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

/* {{{ define the string length of the uuid
*/
#define PHP_AWARE_UUID_LEN 36
/* }}} */

/*{{{ typedef enum _AwareOperationStatus
*/
typedef enum _AwareOperationStatus {
	AwareOperationFailed,
	AwareOperationSuccess,
	AwareOperationNotSupported
} AwareOperationStatus;
/* }}} */

/* {{{ typedef enum _AwareModuleRegisterStatus
*/
typedef enum _AwareModuleRegisterStatus {
	AwareModuleNotConfigured,
	AwareModuleRegistered,
	AwareModuleFailed
} AwareModuleRegisterStatus;
/* }}} */

/* {{{ Arguments passed to different stages 
*/
#define PHP_AWARE_CONNECT_ARGS		TSRMLS_D
#define PHP_AWARE_GET_ARGS			const char *uuid, zval *event TSRMLS_DC
#define PHP_AWARE_STORE_ARGS		const char *uuid, zval *event, const char *error_filename, long error_lineno TSRMLS_DC, long type, const char *appname, const char *source_baseurl
#define PHP_AWARE_GET_LIST_ARGS		long start, long limit, zval *events TSRMLS_DC
#define PHP_AWARE_DELETE_ARGS		const char *uuid TSRMLS_DC
#define PHP_AWARE_DISCONNECT_ARGS	TSRMLS_D
/* }}} */

/* {{{ typedef struct _php_aware_storage_module
*/
typedef struct _php_aware_storage_module {
	char *name;
	AwareOperationStatus  (*connect)(PHP_AWARE_CONNECT_ARGS);
	AwareOperationStatus  (*get)(PHP_AWARE_GET_ARGS);
	AwareOperationStatus  (*store)(PHP_AWARE_STORE_ARGS);
	AwareOperationStatus  (*get_list)(PHP_AWARE_GET_LIST_ARGS);
	AwareOperationStatus  (*delete)(PHP_AWARE_DELETE_ARGS);
	AwareOperationStatus  (*disconnect)(PHP_AWARE_DISCONNECT_ARGS);
} php_aware_storage_module;
/* }}} */

/* {{{ php_aware_storage_module *php_aware_find_storage_module(const char *);
*/
php_aware_storage_module *php_aware_find_storage_module(const char *);
/* }}} */

/* {{{ void php_aware_storage_store(php_aware_storage_module *, const char *, zval *, long, const char *, long TSRMLS_DC);
*/
void php_aware_storage_store(php_aware_storage_module *, const char *, zval *, long, const char *, long TSRMLS_DC);
/* }}} */

/* {{{ void php_aware_storage_get(const char *, const char *, zval * TSRMLS_DC);
*/
void php_aware_storage_get(const char *, const char *, zval * TSRMLS_DC);
/* }}} */

/* {{{ void php_aware_storage_get_list(const char *, long , long , zval * TSRMLS_DC);
*/
void php_aware_storage_get_list(const char *, long , long , zval * TSRMLS_DC);
/* }}} */

/* {{{ void php_aware_storage_store_all(const char *, zval *, long, const char *, long  TSRMLS_DC);
*/
void php_aware_storage_store_all(const char *, zval *, long, const char *, long  TSRMLS_DC);
/* }}} */

/* {{{ zend_bool php_aware_storage_delete(const char *mod_name, const char *uuid TSRMLS_DC);
*/
zend_bool php_aware_storage_delete(const char *mod_name, const char *uuid TSRMLS_DC);
/* }}} */

/* {{{ void php_aware_storage_module_list(zval *return_value);
*/
void php_aware_storage_module_list(zval *return_value);
/* }}} */

/* {{{ MY_AWARE_EXPORTS void php_aware_storage_serialize(const char *uuid, zval *event, smart_str *data_var TSRMLS_DC);
*/
MY_AWARE_EXPORTS void php_aware_storage_serialize(const char *uuid, zval *event, smart_str *data_var TSRMLS_DC);
/* }}} */

/* {{{ MY_AWARE_EXPORTS zend_bool php_aware_storage_unserialize(const char *, int , zval * TSRMLS_DC);
*/
MY_AWARE_EXPORTS zend_bool php_aware_storage_unserialize(const char *, int , zval * TSRMLS_DC);
/* }}} */

/* {{{ MY_AWARE_EXPORTS AwareModuleRegisterStatus php_aware_register_storage_module(php_aware_storage_module * TSRMLS_DC);
*/
MY_AWARE_EXPORTS AwareModuleRegisterStatus php_aware_register_storage_module(php_aware_storage_module * TSRMLS_DC);
/* }}} */

/* {{{ Storage function signatures 
*/
#define PHP_AWARE_CONNECT_FUNC(mod_name)	AwareOperationStatus php_aware_storage_connect_##mod_name(PHP_AWARE_CONNECT_ARGS)

#define PHP_AWARE_GET_FUNC(mod_name)		AwareOperationStatus php_aware_storage_get_##mod_name(PHP_AWARE_GET_ARGS)

#define PHP_AWARE_STORE_FUNC(mod_name)		AwareOperationStatus php_aware_storage_store_##mod_name(PHP_AWARE_STORE_ARGS)

#define PHP_AWARE_GET_LIST_FUNC(mod_name)	AwareOperationStatus php_aware_storage_get_list_##mod_name(PHP_AWARE_GET_LIST_ARGS)

#define PHP_AWARE_DELETE_FUNC(mod_name)		AwareOperationStatus php_aware_storage_delete_##mod_name(PHP_AWARE_DELETE_ARGS)

#define PHP_AWARE_DISCONNECT_FUNC(mod_name)	AwareOperationStatus php_aware_storage_disconnect_##mod_name(PHP_AWARE_DISCONNECT_ARGS)
/* }}} */

/* {{{ define storage functions
*/
#define PHP_AWARE_STORAGE_FUNCS(mod_name) \
	PHP_AWARE_CONNECT_FUNC(mod_name); \
	PHP_AWARE_GET_FUNC(mod_name); \
	PHP_AWARE_STORE_FUNC(mod_name); \
	PHP_AWARE_GET_LIST_FUNC(mod_name); \
	PHP_AWARE_DELETE_FUNC(mod_name); \
	PHP_AWARE_DISCONNECT_FUNC(mod_name); 
/* }}} */

/* {{{ Define the func ptrs for the specific module
*/
#define PHP_AWARE_STORAGE_MOD(mod_name) \
	#mod_name, php_aware_storage_connect_##mod_name, php_aware_storage_get_##mod_name, php_aware_storage_store_##mod_name, \
	php_aware_storage_get_list_##mod_name, php_aware_storage_delete_##mod_name, php_aware_storage_disconnect_##mod_name
/* }}} */	
	
/* {{{ Register the storage module. called from MINIT in module
*/
#define PHP_AWARE_STORAGE_REGISTER(mod_name) \
	php_aware_register_storage_module(php_aware_storage_module_##mod_name##_ptr TSRMLS_CC)
/* }}} */

#endif
