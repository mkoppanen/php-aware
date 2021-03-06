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

#include "php_aware_cache.h"

/* {{{ void php_aware_cache_init(php_aware_serialize_cache *cache)
*/
void php_aware_cache_init(php_aware_serialize_cache *cache)
{
	cache->has_item = 0;
	cache->data     = NULL;
	cache->data_len = 0;
	
	cache->uuid = malloc(PHP_AWARE_UUID_LEN + 1);
	memset(cache->uuid, 0, PHP_AWARE_UUID_LEN + 1);
}
/* }}} */

/* {{{ void php_aware_cache_clean(php_aware_serialize_cache *cache)
*/
void php_aware_cache_clean(php_aware_serialize_cache *cache)
{
	free(cache->data);
	memset(cache->uuid, 0, PHP_AWARE_UUID_LEN + 1);
	
	cache->data     = NULL;
	cache->data_len = 0;
	cache->has_item = 0;
}
/* }}} */

/* {{{ zend_bool php_aware_cache_get(php_aware_serialize_cache *cache, const char *uuid, smart_str *retval)
*/
zend_bool php_aware_cache_get(php_aware_serialize_cache *cache, const char *uuid, smart_str *retval)
{
	if (cache->has_item && !strcmp(cache->uuid, uuid)) {
		smart_str_appendl(retval, cache->data, cache->data_len);
		smart_str_0(retval);
		return 1;
	}
	return 0;
}
/* }}} */

/* {{{ void php_aware_cache_store(php_aware_serialize_cache *cache, const char *uuid, smart_str *data)
*/
void php_aware_cache_store(php_aware_serialize_cache *cache, const char *uuid, smart_str *data)
{
	if (cache->has_item)
		php_aware_cache_clean(cache);
	
	cache->data     = strdup(data->c);
	cache->data_len = data->len;
	
	memcpy(cache->uuid, uuid, PHP_AWARE_UUID_LEN);
	cache->uuid[PHP_AWARE_UUID_LEN] = '\0';

	cache->has_item = 1;
}
/* }}} */

/* {{{ void php_aware_cache_deinit(php_aware_serialize_cache *cache)
*/
void php_aware_cache_deinit(php_aware_serialize_cache *cache)
{
	if (cache->has_item)
		php_aware_cache_clean(cache);
		
	free(cache->uuid);	
}
/* }}} */