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

#ifndef _PHP_AWARE_H_
# define _PHP_AWARE_H_

#include "php.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php_ini.h"
#include "ext/standard/info.h"

#define PHP_AWARE_EXTVER "0.1.0-dev"

/* Original error callback */
typedef void (*php_aware_orig_error_cb_t)(int, const char *, const uint, const char *, va_list);

ZEND_BEGIN_MODULE_GLOBALS(aware)
	zend_bool enabled;
	zend_bool log_get;
	zend_bool log_post;
	zend_bool log_session;
	zend_bool log_cookie;
	zend_bool log_env;
	zend_bool log_server;
	zend_bool log_generated;
	zend_bool log_backtrace;

	long log_level;
	long depth;
	
	char *storage_modules;

	php_aware_orig_error_cb_t orig_error_cb;

	void (*orig_set_error_handler)(INTERNAL_FUNCTION_PARAMETERS);
	void (*orig_restore_error_handler)(INTERNAL_FUNCTION_PARAMETERS);
	
	zend_ptr_stack user_error_handlers;
	zend_stack user_error_handler_levels;
	
	zval *user_error_handler;
	
	long slow_request_threshold;
	struct timeval request_start;
	
ZEND_END_MODULE_GLOBALS(aware)

ZEND_EXTERN_MODULE_GLOBALS(aware)

#ifdef ZTS
# define AWARE_G(v) TSRMG(aware_globals_id, zend_aware_globals *, v)
#else
# define AWARE_G(v) (aware_globals.v)
#endif

extern zend_module_entry aware_module_entry;
#define phpext_aware_ptr &aware_module_entry

#ifdef _AWARE_DEBUG_
#define aware_printf(...) fprintf (stderr, __VA_ARGS__)
#else
#define aware_printf(...)
#endif

void php_aware_original_error_cb(int type TSRMLS_DC, const char *format, ...);

#endif