#!/bin/bash

phpize && ./configure && make && make install && 
cd storage/files &&
phpize && ./configure && make && make install && 
cd ../snmp &&
phpize && ./configure && make && make install