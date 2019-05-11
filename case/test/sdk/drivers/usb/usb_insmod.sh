#!/bin/sh

lib_dir="/lib/modules/2.6.27.29"
echo "insmod usb device driver"
insmod $lib_dir/am7x_uoc.ko detect_gpio=28
insmod $lib_dir/am7x_udc.ko
insmod $lib_dir/g_file_storage.ko file=/dev/partitions/udisk stall=0 removable=1