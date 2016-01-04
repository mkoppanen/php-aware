PHP_ARG_ENABLE(aware-elasticsearch, whether to enable aware elasticsearch backend,
[  --enable-aware-elasticsearch     Enable aware elasticsearch backend])

PHP_ARG_ENABLE(aware-debug, whether to enable debugging,
[  --enable-aware-debug     Enable debugging], no, no)

if test "$PHP_AWARE_ELASTICSEARCH" != "no"; then

  if test "$PHP_AWARE_DEBUG" != "no"; then
    AC_DEFINE([_AWARE_DEBUG_], 1, [Enable debugging])
  fi
  for i in /usr /usr/local /opt /opt/local; do
  	if test -r "$i/include/curl/curl.h"; then
  		PHP_AWARE_LIBCURL=$i
  		break;
  	fi
  done

  if test -z $PHP_AWARE_LIBCURL; then
  	AC_MSG_ERROR([libcurl not found])	
  fi

  AC_MSG_RESULT([$PHP_AWARE_LIBCURL])
  PHP_ADD_INCLUDE("$PHP_AWARE_LIBCURL/include")
  PHP_ADD_LIBRARY_WITH_PATH(curl, "$PHP_AWARE_LIBCURL/lib", AWARE_ELASTICSEARCH_SHARED_LIBADD)

  for i in /usr /usr/local /opt /opt/local; do
  	if test -r "$i/include/json-c/json.h"; then
  		PHP_AWARE_LIBJSONC=$i
  		break;
  	fi
  done

  if test -z $PHP_AWARE_LIBJSONC; then
  	AC_MSG_ERROR([libjson-c not found])	
  fi

  AC_MSG_RESULT([$PHP_AWARE_LIBJSONC])
  PHP_ADD_INCLUDE("$PHP_AWARE_LIBJSONC/include")
  PHP_ADD_LIBRARY_WITH_PATH(json-c, "$PHP_AWARE_LIBCURL/lib", AWARE_ELASTICSEARCH_SHARED_LIBADD)
  PHP_SUBST(AWARE_ELASTICSEARCH_SHARED_LIBADD)

  PHP_NEW_EXTENSION(aware_elasticsearch, aware_elasticsearch.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(aware_elasticsearch, aware)
fi
