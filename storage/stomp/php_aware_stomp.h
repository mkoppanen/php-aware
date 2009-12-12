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

#ifndef _PHP_AWARE_STOMP_H_
# define _PHP_AWARE_STOMP_H_

#define PHP_AWARE_STOMP_EXTVER "0.0.1-dev"

#include "php.h"
#include "php_ini.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

#include <ext/aware/php_aware.h>
#include <ext/aware/php_aware_storage.h>

#include "php_aware_stomp_funcs.h"

ZEND_BEGIN_MODULE_GLOBALS(aware_stomp)
	zend_bool enabled;
	
	char *server_uri;
	char *queue_name;
	
	char *username;
	char *password;

	php_aware_stomp_handle *handle;
	
ZEND_END_MODULE_GLOBALS(aware_stomp)

ZEND_EXTERN_MODULE_GLOBALS(aware_stomp)

#ifdef ZTS
# define AWARE_STOMP_G(v) TSRMG(aware_stomp_globals_id, zend_aware_stomp_globals *, v)
#else
# define AWARE_STOMP_G(v) (aware_stomp_globals.v)
#endif

/* Hook into aware module */
extern php_aware_storage_module php_aware_storage_module_stomp;
#define php_aware_storage_module_stomp_ptr &php_aware_storage_module_stomp

/* Normal PHP entry */
extern zend_module_entry aware_stomp_module_entry;
#define phpext_aware_stomp_ptr &aware_stomp_module_entry

PHP_AWARE_STORAGE_FUNCS(stomp);

#ifndef ZSTR
# define ZSTR
#endif


#endif