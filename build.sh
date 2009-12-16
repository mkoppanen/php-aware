#!/bin/bash

if [ "x$1" = "xdebug" ]; then 
    CONFIGURE_OPTS="--enable-aware-debug"
fi

export CFLAGS="-Wall -g"
export PHP_AUTOCONF="autoconf2.13"
export PHP_AUTOHEADER="autoheader2.13"

phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd storage/files && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../snmp && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../tokyo && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../stomp && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../email && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../spread && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../skeleton && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install