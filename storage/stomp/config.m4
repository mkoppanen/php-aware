PHP_ARG_ENABLE(aware-stomp, whether to enable aware stomp backend,
[  --enable-aware-stomp     Enable aware stomp backend])

PHP_ARG_ENABLE(aware-debug, whether to enable debugging,
[  --enable-aware-debug     Enable debugging], no, no)

if test "$PHP_AWARE_STOMP" != "no"; then

  if test "$PHP_AWARE_DEBUG" != "no"; then
    AC_DEFINE([_AWARE_DEBUG_], 1, [Enable debugging])
  fi

  PHP_NEW_EXTENSION(aware_stomp, aware_stomp.c aware_stomp_funcs.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(aware_stomp, aware)
fi