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

#ifdef HAVE_GETTIMEOFDAY
static zend_bool aware_timestamp_get(struct timeval *tp)
{
	if (gettimeofday(tp, NULL)) {
		return 0;
    }
	return 1;
}

/* Capture info about slow request */
static void php_aware_capture_slow_request(long elapsed, long threshold, const char *format, ...)
{
	va_list args;
	zval *event, *slow_request;
	
	ALLOC_INIT_ZVAL(event);
	array_init(event);
	
	ALLOC_INIT_ZVAL(slow_request);
	array_init(slow_request);
	
	
	add_assoc_long(slow_request, "time_elapsed", elapsed);
	add_assoc_long(slow_request, "slow_request_threshold", threshold);	
	
	add_assoc_bool(event, "slow_request", 1);
	add_assoc_zval(event, "_AWARE_REQUEST", slow_request);

	va_start(args, format);
	php_aware_capture_error_ex(event, E_CORE_WARNING, "aware internal report", 0, 1, format, args);
	va_end(args);
}

/* {{{ Initializes the slow request monitor
*/
zend_bool php_aware_init_slow_request_monitor(struct timeval *request_start)
{
	if (!aware_timestamp_get(request_start)) {
		return 0;
	}
	return 1;
}
/* }}} */

void php_aware_monitor_slow_request(struct timeval *request_start, long threshold) 
{
	struct timeval tp;
	if (aware_timestamp_get(&tp)) {
		/* in milliseconds */
		long elapsed;
		
		elapsed  = (long)((tp.tv_sec - request_start->tv_sec) * 1000);
		elapsed += (long)((tp.tv_usec - request_start->tv_usec) / 1000);
		
		if (elapsed > threshold) {
			php_aware_capture_slow_request(elapsed, threshold, "Slow request detected");
		}
	}
}
#endif

/* Capture info about excessive memory usage */
void php_aware_capture_memory_usage(long peak, long threshold, const char *format, ...)
{
	va_list args;
	zval *event, *peak_usage;
	
	ALLOC_INIT_ZVAL(event);
	array_init(event);
	
	ALLOC_INIT_ZVAL(peak_usage);
	array_init(peak_usage);

	add_assoc_long(peak_usage, "memory_peak_usage", peak);
	add_assoc_long(peak_usage, "memory_usage_threshold", threshold);	
	
	add_assoc_zval(event, "_AWARE_MEMORY", peak_usage);
	add_assoc_bool(event, "excessive_memory_usage", 1);

	va_start(args, format);
	php_aware_capture_error_ex(event, E_CORE_WARNING, "aware internal report", 0, 1, format, args);
	va_end(args);
}

void php_aware_monitor_memory_usage(long threshold)
{
	long peak = zend_memory_peak_usage(1 TSRMLS_CC);

	if (peak > threshold) {
		php_aware_capture_memory_usage(peak, threshold, "Excessive memory usage detected");
	}
}
