#!/bin/bash

phpize && ./configure --enable-aware-debug && make && make install && 
cd storage/files &&
phpize && ./configure --enable-aware-debug && make && make install && 
cd ../snmp &&
phpize && ./configure --enable-aware-debug && make && make install
cd ../skeleton &&
phpize && ./configure --enable-aware-debug && make && make install