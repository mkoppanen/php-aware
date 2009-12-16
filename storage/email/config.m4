PHP_ARG_ENABLE(aware-email, whether to enable aware email backend,
[  --enable-aware-email     Enable aware email backend])

PHP_ARG_ENABLE(aware-debug, whether to enable debugging,
[  --enable-aware-debug     Enable debugging], no, no)

if test "$PHP_AWARE_EMAIL" != "no"; then

  if test "$PHP_AWARE_DEBUG" != "no"; then
    AC_DEFINE([_AWARE_DEBUG_], 1, [Enable debugging])
  fi

  PHP_NEW_EXTENSION(aware_email, aware_email.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(aware_email, aware)
fi