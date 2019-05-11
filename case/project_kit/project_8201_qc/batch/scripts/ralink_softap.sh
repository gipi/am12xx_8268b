#!/bin/sh

#insmod /lib/modules/2.6.27.29/rtutil3070ap.ko
#insmod /lib/modules/2.6.27.29/rt3070ap.ko
#insmod /lib/modules/2.6.27.29/rtnet3070ap.ko
ifconfig ra0 up
ifconfig ra0 192.168.100.10

/usr/sbin/udhcpd /etc/ra0-udhcpd.conf

#thttpd -C /etc/thttpd.conf