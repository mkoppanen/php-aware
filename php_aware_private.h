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

#ifndef _PHP_AWARE_PRIVATE_H_
# define _PHP_AWARE_PRIVATE_H_

#include "php_aware.h"
#include "php_aware_storage.h"

void php_aware_capture_error_ex(zval *event, int type, const char *error_filename, const uint error_lineno, zend_bool free_event, const char *format, va_list args);

void php_aware_invoke_handler(int type, const char *error_filename, const uint error_lineno, const char *format, ...);

#endif