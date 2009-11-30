PHP_ARG_ENABLE(aware, whether to enable error monitoring support,
[  --enable-aware     Enable experimental error monitoring support])

if test "$PHP_AWARE" != "no"; then
  PHP_NEW_EXTENSION(aware, aware.c storage.c, $ext_shared)
  PHP_INSTALL_HEADERS([ext/aware], [php_aware.h php_aware_storage.h])
fi