#! /bin/sh

#########################################
# this shell scripts used by config.app
#########################################

# install lcm module
lib_dir="/lib/modules/2.6.27.29"
insmod $lib_dir/am7x_lcm.ko
insmod $lib_dir/am7x_carddet.ko
insmod $lib_dir/am7x_uoc.ko detect_gpio=28
insmod $lib_dir/am7x_keys.ko
insmod $lib_dir/am7xx_dac.ko

