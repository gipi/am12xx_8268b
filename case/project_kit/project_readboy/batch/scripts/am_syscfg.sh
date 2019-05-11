#! /bin/sh

#########################################
# this shell scripts used by config.app
#########################################

# install lcm module
lib_dir="/lib/modules/2.6.27.29"
insmod $lib_dir/am7x_lcm.ko
#open backlight
insmod $lib_dir/ambl.ko
#echo "21" >sys/class/backlight/am7xbl1/brightness 

insmod $lib_dir/am7x_carddet.ko
insmod $lib_dir/am7x_uoc.ko
insmod $lib_dir/am7x_keys.ko
insmod $lib_dir/am7x_tp.ko
insmod $lib_dir/rb_spikey.ko
insmod $lib_dir/am7xx_dac.ko
insmod $lib_dir/mousedev.ko
insmod $lib_dir/am7x_psmouse.ko
insmod $lib_dir/rb_touchpad.ko
insmod $lib_dir/am7x_battery.ko

mdev -s
