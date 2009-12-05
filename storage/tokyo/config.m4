PHP_ARG_ENABLE(aware-tokyo,         whether to enable aware tokyo backend,
[  --enable-aware-tokyo             Enable aware tokyo backend])

PHP_ARG_WITH(aware-tokyo-tyrant,    DIR to Tokyo Tyrant installation,
[ --with-aware-tokyo-tyrant[=DIR]	  DIR to Tokyo Tyrant installation], no)

PHP_ARG_WITH(aware-tokyo-cabinet,   DIR to Tokyo Cabinet installation,
[ --with-aware-tokyo-cabinet[=DIR]	DIR to Tokyo Cabinet installation], no)

PHP_ARG_ENABLE(aware-debug,         whether to enable debugging,
[  --enable-aware-debug             Enable debugging], no, no)

if test "$PHP_AWARE_TOKYO" != "no"; then

  if test "$PHP_AWARE_DEBUG" != "no"; then
    AC_DEFINE([_AWARE_DEBUG_], 1, [Enable debugging])
  fi
  
dnl check pkg-config
  AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  if test -z "$PKG_CONFIG"; then
    AC_MSG_RESULT([pkg-config not found])
    AC_MSG_ERROR([Please reinstall the pkg-config distribution])
  fi

  ORIG_PKG_CONFIG_PATH=$PKG_CONFIG_PATH

dnl Tokyo Tyrant PKG_CONFIG
  if test "$PHP_AWARE_TOKYO_TYRANT" = "no"; then
    export PKG_CONFIG_PATH=/usr/lib/pkgconfig:/usr/local/lib/pkgconfig:/opt/lib/pkgconfig:/opt/local/lib/pkgconfig
  else
    export PKG_CONFIG_PATH=$PHP_AWARE_TOKYO_TYRANT:$PHP_AWARE_TOKYO_TYRANT/lib/pkgconfig
  fi

  AC_MSG_CHECKING([for Tokyo Tyrant])
  if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists tokyotyrant; then
    PHP_TYRANT_INCS=`$PKG_CONFIG tokyotyrant --cflags`
    PHP_TYRANT_LIBS=`$PKG_CONFIG tokyotyrant --libs`
    PHP_TYRANT_VERSION=`$PKG_CONFIG tokyotyrant --modversion`

    PHP_EVAL_LIBLINE($PHP_TYRANT_LIBS, AWARE_TOKYO_SHARED_LIBADD)
    PHP_EVAL_INCLINE($PHP_TYRANT_INCS)
    AC_MSG_RESULT([yes, ${PHP_TYRANT_VERSION}])
  else
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the Tokyo Tyrant distribution])
  fi

  AC_MSG_CHECKING([that Tokyo Tyrant is at least version 1.1.24])
  PHP_TYRANT_VERSION_STRING=`$PKG_CONFIG --atleast-version=1.1.24 tokyotyrant`

  if test $? != 0; then
    AC_MSG_ERROR(no)
  fi
  AC_MSG_RESULT(yes)

  PHP_TYRANT_VERSION_MASK=`echo ${PHP_TYRANT_VERSION_STRING} | awk 'BEGIN { FS = "."; } { printf "%d", ($1 * 1000 + $2) * 1000 + $3;}'`
  AC_DEFINE_UNQUOTED(PHP_AWARE_TOKYO_TYRANT_VERSION, ${PHP_TYRANT_VERSION_MASK}, [ ])

dnl Tokyo Cabinet PKG_CONFIG
  if test "$PHP_AWARE_TOKYO_CABINET_DIR" = "no"; then
    export PKG_CONFIG_PATH=/usr/lib/pkgconfig:/usr/local/lib/pkgconfig:/opt/lib/pkgconfig:/opt/local/lib/pkgconfig
  else
    export PKG_CONFIG_PATH=$PHP_AWARE_TOKYO_CABINET_DIR:$PHP_AWARE_TOKYO_CABINET_DIR/lib/pkgconfig
  fi

  AC_MSG_CHECKING([for Tokyo Cabinet])
  if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists tokyocabinet; then
    PHP_CABINET_INCS=`$PKG_CONFIG tokyocabinet --cflags`
    PHP_CABINET_LIBS=`$PKG_CONFIG tokyocabinet --libs`
    PHP_CABINET_VERSION=`$PKG_CONFIG tokyocabinet --modversion`

    PHP_EVAL_LIBLINE($PHP_CABINET_LIBS, AWARE_TOKYO_SHARED_LIBADD)
    PHP_EVAL_INCLINE($PHP_CABINET_INCS)
    AC_MSG_RESULT([yes, ${PHP_CABINET_VERSION}])
  else
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the Tokyo Cabinet distribution])
  fi

  export PKG_CONFIG_PATH="$ORIG_PKG_CONFIG_PATH"

  PHP_NEW_EXTENSION(aware_tokyo, aware_tokyo.c aware_tokyo_cabinet.c aware_tokyo_tyrant.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(aware_tokyo, aware)

  PHP_SUBST(AWARE_TOKYO_SHARED_LIBADD)
fi