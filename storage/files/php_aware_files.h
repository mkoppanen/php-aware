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

#ifndef _PHP_AWARE_FILES_H_
# define _PHP_AWARE_FILES_H_

#define PHP_AWARE_FILES_EXTVER "0.0.1-dev"

#include "php.h"
#include "php_ini.h"

#include <ext/aware/php_aware.h>
#include <ext/aware/php_aware_storage.h>

ZEND_BEGIN_MODULE_GLOBALS(aware_files)
	char *storage_path;
ZEND_END_MODULE_GLOBALS(aware_files)

ZEND_EXTERN_MODULE_GLOBALS(aware_files)

#ifdef ZTS
# define AWARE_FILES_G(v) TSRMG(aware_files_globals_id, zend_aware_files_globals *, v)
#else
# define AWARE_FILES_G(v) (aware_files_globals.v)
#endif

/* Hook into aware module */
extern php_aware_storage_module php_aware_storage_module_files;
#define php_aware_storage_module_files_ptr &php_aware_storage_module_files

/* Normal PHP entry */
extern zend_module_entry aware_files_module_entry;
#define phpext_aware_files_ptr &aware_files_module_entry

PHP_AWARE_STORAGE_FUNCS(files);

#endif