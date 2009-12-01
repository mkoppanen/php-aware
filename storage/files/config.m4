PHP_ARG_ENABLE(aware-files, whether to enable aware files backend,
[  --enable-aware-files     Enable aware files backend])

PHP_ARG_ENABLE(aware-debug, whether to enable debugging,
[  --enable-aware-debug     Enable debugging], no, no)

if test "$PHP_AWARE_FILES" != "no"; then

  if test "$PHP_AWARE_DEBUG" != "no"; then
    AC_DEFINE([_AWARE_DEBUG_], 1, [Enable debugging])
  fi

  PHP_NEW_EXTENSION(aware_files, aware_files.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(aware_files, aware)
fi