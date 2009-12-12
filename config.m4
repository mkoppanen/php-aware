PHP_ARG_ENABLE(aware, whether to enable error monitoring support,
[  --enable-aware     Enable experimental error monitoring support])

PHP_ARG_ENABLE(aware-debug, whether to enable debugging,
[  --enable-aware-debug     Enable debugging], no, no)

if test "$PHP_AWARE" != "no"; then

  AC_MSG_CHECKING([for libuuid])

  for i in $PHP_DCSESSION /usr /usr/local /opt /opt/local; do
  	if test -r "$i/include/uuid/uuid.h"; then
  		PHP_AWARE_LIBUUID=$i
  		break;
  	fi
  done

  if test -z $PHP_AWARE_LIBUUID; then
  	AC_MSG_ERROR([libuuid not found])	
  fi

  AC_MSG_RESULT([$PHP_AWARE_LIBUUID])

	if test "$PHP_AWARE_DEBUG" != "no"; then
    AC_DEFINE([_AWARE_DEBUG_], 1, [Enable debugging])
  fi

  PHP_ADD_INCLUDE("$PHP_AWARE_LIBUUID/include")
  PHP_ADD_LIBRARY_WITH_PATH(uuid, "$PHP_AWARE_LIBUUID/lib", AWARE_SHARED_LIBADD)
  PHP_SUBST(AWARE_SHARED_LIBADD)

  PHP_NEW_EXTENSION(aware, aware.c aware_storage.c aware_request.c aware_uuid.c, $ext_shared)
  PHP_INSTALL_HEADERS([ext/aware], [php_aware.h php_aware_storage.h])
  
fi