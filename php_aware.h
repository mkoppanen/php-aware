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

#ifdef ZTS
# include "TSRM.h"
#endif

#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_smart_str.h"

#define PHP_AWARE_EXTVER "0.1.0-dev"

#include <sys/resource.h>

/* Original error callback */
typedef void (*php_aware_orig_error_cb_t)(int, const char *, const uint, const char *, va_list);


ZEND_BEGIN_MODULE_GLOBALS(aware)
	zend_bool enabled;			/* is the module enabled */
	zend_bool log_get;			/* whether to log get values */
	zend_bool log_post;			/* whether to log post values */
	zend_bool log_session;		/* whether to log session values */
	zend_bool log_cookie;		/* whether to log cookies */
	zend_bool log_env;			/* whether to log environment */
	zend_bool log_server;		/* whether to log server values */
	zend_bool log_files;		/* whether to log files array */
	
	zend_bool log_backtrace;	/* whether to store backtrace */

	zend_bool use_cache;		/* use serialize cache? stores repeated serializations */
	
	zend_bool enable_event_trigger;	/* do we want to log user generated events */

	long log_level;					/* what sort of events do we store */
	long depth;						/* how many levels to serialize (arrays) */
	long memory_usage_threshold;	/* how many bytes is the limit of excess usage */
	
	char *storage_modules;			/* what storage modules to enable */

	php_aware_orig_error_cb_t orig_error_cb;	/* the original zend_error_cb */

	void (*orig_set_error_handler)(INTERNAL_FUNCTION_PARAMETERS);	/* the set_error_handle entry */
	void (*orig_restore_error_handler)(INTERNAL_FUNCTION_PARAMETERS);	/* the restore error handler entry */
	
	zend_ptr_stack user_error_handlers;
	zend_stack user_error_handler_levels;
	
	zval *user_error_handler;
	
	long slow_request_threshold;
	struct timeval request_start_tv;
	struct rusage  request_start_rusage;

	HashTable module_error_reporting;
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

/*
	API exports
*/
#ifndef MY_AWARE_EXPORTS
#  ifdef PHP_WIN32
#    define MY_AWARE_EXPORTS __declspec(dllexport)
#  else
#    define MY_AWARE_EXPORTS PHPAPI
#  endif
#endif

MY_AWARE_EXPORTS void php_aware_original_error_cb(int type TSRMLS_DC, const char *format, ...);

#endif
