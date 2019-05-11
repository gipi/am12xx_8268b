#!/bin/sh

MOD_DIR=/lib/modules/2.6.27.29/

# Insert modules
cd $MOD_DIR

insmod am7x_uoc.ko
insmod am7x_udc.ko use_dma=0
insmod g_ether.ko

ifconfig usb0 8.8.8.8 up
ifconfig lo up


