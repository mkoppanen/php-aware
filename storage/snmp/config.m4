PHP_ARG_WITH(aware-snmp,    whether to enable aware snmp extension,
[  --with-aware-snmp[=DIR]     Enable aware snmp extension. DIR is the path to net-snmp-config])

if test "$PHP_AWARE_SNMP" != "no"; then

	AC_MSG_CHECKING(net-snmp-config program)
	
	for i in $PHP_AWARE_SNMP $PHP_AWARE_SNMP/bin /usr/bin /usr/local/bin; do
		if test -f $i/net-snmp-config; then
			NET_SNMP_CONF=$i/net-snmp-config
			AC_MSG_RESULT(found in $NET_SNMP_CONF)
		fi
	done
	
	if test -z $NET_SNMP_CONF; then
		AC_MSG_ERROR(not found)
	fi

	if test ! -x $NET_SNMP_CONF; then
		AC_MSG_ERROR($NET_SNMP_CONF is not executable)
	fi

	PHP_AWARE_SNMP_INCS="$CFLAGS `$NET_SNMP_CONF --cflags`"
	PHP_AWARE_SNMP_LIBS="$LDFLAGS `$NET_SNMP_CONF --netsnmp-libs`"
	
	PHP_EVAL_LIBLINE($PHP_AWARE_SNMP_LIBS, AWARE_SNMP_SHARED_LIBADD)
    PHP_EVAL_INCLINE($PHP_AWARE_SNMP_INCS)

	PHP_NEW_EXTENSION(aware_snmp, aware_snmp.c, $ext_shared)
	PHP_ADD_EXTENSION_DEP(aware_snmp, aware)
	PHP_SUBST(AWARE_SNMP_SHARED_LIBADD)
fi