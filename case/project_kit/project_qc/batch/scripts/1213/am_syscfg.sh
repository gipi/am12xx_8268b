#! /bin/sh

#########################################
# this shell scripts used by config.app
#########################################

# install lcm module
lib_dir="/lib/modules/2.6.27.29"
insmod $lib_dir/i2c-am7x.ko
insmod $lib_dir/edid_i2c_hw.ko
insmod $lib_dir/edid_i2c_gpio.ko
insmod $lib_dir/hdcp_i2c.ko
insmod $lib_dir/am7x_lcm.ko
#open backlight
insmod $lib_dir/ambl.ko
#echo "21" >sys/class/backlight/am7xbl1/brightness 

insmod $lib_dir/am7x_carddet.ko
insmod $lib_dir/am7x_uoc.ko
insmod $lib_dir/am7x_uoc_next.ko
insmod $lib_dir/am7x_keys.ko
insmod $lib_dir/am7x_tp.ko
#insmod $lib_dir/rb_spikey.ko
insmod $lib_dir/am7xx_dac.ko
insmod $lib_dir/am7x_battery.ko
insmod $lib_dir/am7x_cipher.ko

mdev -s
