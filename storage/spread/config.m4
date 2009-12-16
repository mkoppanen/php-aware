PHP_ARG_WITH(aware-spread=[DIR], whether to enable aware spread backend,
[  --with-aware-spread=[DIR]     Enable aware spread backend])

PHP_ARG_ENABLE(aware-debug, whether to enable debugging,
[  --enable-aware-debug     Enable debugging], no, no)

if test "$PHP_AWARE_SPREAD" != "no"; then

  if test "$PHP_AWARE_DEBUG" != "no"; then
    AC_DEFINE([_AWARE_DEBUG_], 1, [Enable debugging])
  fi
  
  if test -r "$PHP_AWARE_SPREAD/include/sp.h"; then
		PHP_AWARE_SPREAD_DIR="$PHP_AWARE_SPREAD"
	else
		AC_MSG_CHECKING(for libspread in default path)
		for i in /usr /usr/local; do
			if test -r "$i/include/sp.h"; then
				PHP_AWARE_SPREAD_DIR=$i
				AC_MSG_RESULT(found in $i)
			fi
		done
	fi
	
	PHP_ADD_INCLUDE($PHP_AWARE_SPREAD_DIR/include)

  PHP_ADD_LIBRARY_WITH_PATH(spread, $PHP_AWARE_SPREAD_DIR/$PHP_LIBDIR, AWARE_SPREAD_SHARED_LIBADD)
  AC_DEFINE(HAVE_AWARE_SPREAD,1,[ ])

  PHP_NEW_EXTENSION(aware_spread, aware_spread.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(aware_spread, aware)
  
  PHP_SUBST(AWARE_SPREAD_SHARED_LIBADD)
  
fi