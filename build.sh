#!/bin/bash

phpize && ./configure && make clean && make && make install && 
cd storage/files &&
phpize && ./configure && make clean && make && make install && 
cd ../snmp &&
phpize && ./configure && make clean && make && make install
cd ../skeleton &&
phpize && ./configure && make clean && make && make install