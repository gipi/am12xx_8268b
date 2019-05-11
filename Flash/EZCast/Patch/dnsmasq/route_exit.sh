#!/bin/bash

echo "kill the named process"
killall dnsmasq
#killall named
#killall mrouted

echo "------------- clear ip talbes"
iptables -F -t nat
iptables -X -t nat
iptables -Z -t nat
iptables -t nat -P PREROUTING  ACCEPT
iptables -t nat -P POSTROUTING ACCEPT
iptables -t nat -P OUTPUT      ACCEPT


