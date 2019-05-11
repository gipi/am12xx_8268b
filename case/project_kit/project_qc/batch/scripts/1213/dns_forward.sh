#!/bin/bash

echo "--- start DNS server"

if [ -e /var/named/ ]
then
    echo "/var/named/ exist"
else
    echo "/var/named/ not exist" 
    mkdir -p /var/named/
fi
killall named
/sbin/named -c /etc/named.conf

