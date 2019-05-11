#!/bin/bash

echo "--- start DNS server"

killall dnsmasq

/sbin/dnsmasq -d -C /tmp/dnsmasq.conf &
