#!/bin/bash

if [ "x$1" = "xdebug" ]; then 
    CONFIGURE_OPTS="--enable-aware-debug"
fi

phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && 
cd storage/files &&
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && 
cd ../snmp &&
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install
cd ../skeleton &&
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install