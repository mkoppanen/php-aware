PHP_ARG_ENABLE(aware-skeleton, whether to enable aware skeleton backend,
[  --enable-aware-skeleton     Enable aware skeleton backend])

PHP_ARG_ENABLE(aware-debug, whether to enable debugging,
[  --enable-aware-debug     Enable debugging], no, no)

if test "$PHP_AWARE_FILES" != "no"; then

  if test "$PHP_AWARE_DEBUG" != "no"; then
    AC_DEFINE([_AWARE_DEBUG_], 1, [Enable debugging])
  fi

  PHP_NEW_EXTENSION(aware_skeleton, aware_skeleton.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(aware_skeleton, aware)
fi