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

#ifndef _PHP_AWARE_SPREAD_H_
# define _PHP_AWARE_SPREAD_H_

#define PHP_AWARE_SPREAD_EXTVER "0.0.1-dev"

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

#include <sp.h>

ZEND_BEGIN_MODULE_GLOBALS(aware_spread)
	
	zend_bool enabled;
	
	char *spread_name;
	char *group_name;
	char *user_name;
	
	mailbox spread_mailbox;
	
	zend_bool connected;
	
ZEND_END_MODULE_GLOBALS(aware_spread)

ZEND_EXTERN_MODULE_GLOBALS(aware_spread)

#ifdef ZTS
# define AWARE_SPREAD_G(v) TSRMG(aware_spread_globals_id, zend_aware_spread_globals *, v)
#else
# define AWARE_SPREAD_G(v) (aware_spread_globals.v)
#endif

/* Hook into aware module */
extern php_aware_storage_module php_aware_storage_module_spread;
#define php_aware_storage_module_spread_ptr &php_aware_storage_module_spread

/* Normal PHP entry */
extern zend_module_entry aware_spread_module_entry;
#define phpext_aware_spread_ptr &aware_spread_module_entry

PHP_AWARE_STORAGE_FUNCS(spread);

#endif