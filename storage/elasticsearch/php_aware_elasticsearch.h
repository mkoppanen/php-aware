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

#ifndef _PHP_AWARE_ELASTICSEARCH_H_
# define _PHP_AWARE_ELASTICSEARCH_H_

#define PHP_AWARE_ELASTICSEARCH_EXTVER "0.0.1-dev"

#include "php.h"
#include "php_ini.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

#include "../../php_aware.h"
#include "../../php_aware_storage.h"
/* json-c (https://github.com/json-c/json-c) */
#include <json-c/json.h>

/* libcurl (http://curl.haxx.se/libcurl/c) */
#include <curl/curl.h>

ZEND_BEGIN_MODULE_GLOBALS(aware_elasticsearch)
	char *host;
	zend_bool enabled;
ZEND_END_MODULE_GLOBALS(aware_elasticsearch)

ZEND_EXTERN_MODULE_GLOBALS(aware_elasticsearch)

#ifdef ZTS
# define AWARE_ELASTICSEARCH_G(v) TSRMG(aware_elasticsearch_globals_id, zend_aware_elasticsearch_globals *, v)
#else
# define AWARE_ELASTICSEARCH_G(v) (aware_elasticsearch_globals.v)
#endif

/* Hook into aware module */
extern php_aware_storage_module php_aware_storage_module_elasticsearch;
#define php_aware_storage_module_elasticsearch_ptr &php_aware_storage_module_elasticsearch

/* Normal PHP entry */
extern zend_module_entry aware_elasticsearch_module_entry;
#define phpext_aware_elasticsearch_ptr &aware_elasticsearch_module_entry

PHP_AWARE_STORAGE_FUNCS(elasticsearch);

#endif
