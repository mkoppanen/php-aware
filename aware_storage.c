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

#include "php_aware_private.h"
#include "php_aware_storage.h"

#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_string.h"

ZEND_DECLARE_MODULE_GLOBALS(aware)

#define MAX_MODULES 5

/*
	The module structure follows session module structure quite closely
*/

static php_aware_storage_module *php_aware_storage_modules[MAX_MODULES + 1] = {0};

static zend_bool php_aware_storage_module_is_configured(const char *mod_name TSRMLS_DC) 
{
	zend_bool retval = 0;
	char *pch, *last, *ptr;
	
	/* If aware is not enabled, don't register anything */
	if (!AWARE_G(enabled)) {
		return 0;
	}

	ptr = estrdup(AWARE_G(storage_modules));
	pch = php_strtok_r(ptr, ",", &last);

	while (pch != NULL) {
		char *mod = php_trim(pch, strlen(pch), NULL, 0, NULL, 3 TSRMLS_CC);
		
		if (mod && !strcmp(mod, mod_name)) {
			efree(mod);
			retval = 1;
			break;
		}
		pch = php_strtok_r(NULL, ",", &last);
		efree(mod);
	}
	
	efree(ptr);
	return retval;
}

MY_AWARE_EXPORTS AwareModuleRegisterStatus php_aware_register_storage_module(php_aware_storage_module *mod TSRMLS_DC)
{
	int i, ret = AwareModuleFailed;
	
	aware_printf("Registering storage module: %s\n", mod->name);
	
	if (!php_aware_storage_module_is_configured(mod->name TSRMLS_CC)) {
		aware_printf("Storage module %s is not configured\n", mod->name);
		return AwareModuleNotConfigured;
	}

	for (i = 0; i < MAX_MODULES; i++) {
		if (!php_aware_storage_modules[i]) {
			php_aware_storage_modules[i] = mod;
			ret = AwareModuleRegistered;
			break;
		}
	}
	return ret;	
}

php_aware_storage_module *php_aware_find_storage_module(const char *mod_name) 
{
	int i;
	php_aware_storage_module *ret = NULL;
	
	aware_printf("Finding storage module %s\n", mod_name);
	
	for (i = 0; i < MAX_MODULES; i++) {
		if (php_aware_storage_modules[i] && !strcmp(mod_name, php_aware_storage_modules[i]->name)) {
			aware_printf("Found storage module %s\n", mod_name);
			ret = php_aware_storage_modules[i];
			break;
		}
	}
	return ret;
}

/* Serialize to string */
MY_AWARE_EXPORTS void php_aware_storage_serialize(const char *uuid, zval *event, smart_str *data_var TSRMLS_DC)
{
	php_serialize_data_t var_hash;
	
	if (AWARE_G(use_cache)) {
		if (AWARE_G(serialize_cache_uuid) && !strcmp(AWARE_G(serialize_cache_uuid), uuid)) {
			smart_str_appendl(data_var, AWARE_G(serialize_cache), AWARE_G(serialize_cache_len));
			smart_str_0(data_var);
			return;
		}
	}

	PHP_VAR_SERIALIZE_INIT(var_hash);
	php_var_serialize(data_var, &event, &var_hash TSRMLS_CC);
    PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (AWARE_G(use_cache)) {
		if (!AWARE_G(serialize_cache_uuid) || strcmp(AWARE_G(serialize_cache_uuid), uuid)) {
		
    		if (AWARE_G(serialize_cache_uuid)) {
    			efree(AWARE_G(serialize_cache_uuid));
    		}
		
    		if (AWARE_G(serialize_cache)) {
    			efree(AWARE_G(serialize_cache));
    		}
		
    		AWARE_G(serialize_cache)      = estrndup(data_var->c, data_var->len);
    		AWARE_G(serialize_cache_len)  = data_var->len;
    		AWARE_G(serialize_cache_uuid) = estrdup(uuid);
		}
	}
}

MY_AWARE_EXPORTS zend_bool php_aware_storage_unserialize(const char *string, int string_len, zval *retval TSRMLS_DC)
{
	zend_bool unserialized;
	php_unserialize_data_t var_hash;
	const unsigned char *p, *s;

	p = s = (const unsigned char *)string;
	
	PHP_VAR_UNSERIALIZE_INIT(var_hash);
	unserialized = php_var_unserialize(&retval, (const unsigned char **)&p, (const unsigned char *) s + string_len, &var_hash TSRMLS_CC);
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	
	return (unserialized == 0) ? FAILURE : SUCCESS;
}

/* Sends the event to storage module */
void php_aware_storage_store(php_aware_storage_module *mod, const char *uuid, zval *event, const char *error_filename, long error_lineno TSRMLS_DC) 
{
	aware_printf("Storing event to module: %s\n", mod->name);
	
	/* Connect failed, report error and bail out */
	if (mod->connect(TSRMLS_C) == AwareOperationFailure) {
		php_aware_original_error_cb(E_WARNING TSRMLS_CC, "Failed to connect the storage module (%s)", mod->name);
		return;
	}

	if (mod->store(uuid, event, error_filename, error_lineno TSRMLS_CC) == AwareOperationFailure) {
		php_aware_original_error_cb(E_WARNING TSRMLS_CC, "Failed to store the event (%s)", mod->name);
	}

	if (mod->disconnect(TSRMLS_C) == AwareOperationFailure) {
		php_aware_original_error_cb(E_WARNING TSRMLS_CC, "Failed to disconnect storage module (%s)", mod->name);
	}
}

/* Sends the event to all available storage modules */
void php_aware_storage_store_all(const char *uuid, zval *event, const char *error_filename, long error_lineno TSRMLS_DC) 
{
	int i;
	
	for (i = 0; i < MAX_MODULES; i++) {
		if (php_aware_storage_modules[i]) {
			php_aware_storage_store(php_aware_storage_modules[i], uuid, event, error_filename, error_lineno TSRMLS_CC);
		}
	}
}

/* get the event from storage modules */
void php_aware_storage_get(const char *mod_name, const char *uuid, zval *return_value TSRMLS_DC) 
{
	php_aware_storage_module *mod;
	mod = php_aware_find_storage_module(mod_name);

	if (!mod) {
		php_aware_original_error_cb(E_WARNING TSRMLS_CC, "Storage module(%s) does not exist", mod_name);
		return;
	}
	
	/* Connect failed, report error and bail out */
	if (mod->connect(TSRMLS_C) == AwareOperationFailure) {
		php_aware_original_error_cb(E_WARNING TSRMLS_CC, "Failed to connect the storage module (%s)", mod_name);
		return;
	}

	if (mod->get(uuid, return_value TSRMLS_CC) == AwareOperationFailure) {
		php_aware_original_error_cb(E_WARNING TSRMLS_CC, "Failed to get the event (%s)", mod_name);
	}

	if (mod->disconnect(TSRMLS_C) == AwareOperationFailure) {
		php_aware_original_error_cb(E_WARNING TSRMLS_CC, "Failed to disconnect storage module (%s)", mod->name);
	}
}

/* get the event from storage modules */
void php_aware_storage_get_list(const char *mod_name, long start, long limit, zval *return_value TSRMLS_DC) 
{
	php_aware_storage_module *mod;
	mod = php_aware_find_storage_module(mod_name);

	if (!mod) {
		php_aware_original_error_cb(E_WARNING TSRMLS_CC, "Storage module(%s) does not exist", mod_name);
		return;
	}
	
	/* Connect failed, report error and bail out */
	if (mod->connect(TSRMLS_C) == AwareOperationFailure) {
		php_aware_original_error_cb(E_WARNING TSRMLS_CC, "Failed to connect the storage module (%s)", mod_name);
		return;
	}
	
	if (mod->get_list(start, limit, return_value TSRMLS_CC) == AwareOperationFailure) {
		php_aware_original_error_cb(E_WARNING TSRMLS_CC, "Failed to get event list (%s)", mod_name);
	}

	if (mod->disconnect(TSRMLS_C) == AwareOperationFailure) {
		php_aware_original_error_cb(E_WARNING TSRMLS_CC, "Failed to disconnect storage module (%s)", mod->name);
	}
}
