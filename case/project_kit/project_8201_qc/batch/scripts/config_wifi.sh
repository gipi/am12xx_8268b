#!/bin/bash
#-------------------------------------------------------------------------------
#-
#--  Description : wifi config file
#--
#--------------------------------------------------------------------------------
#--
#--  Version control information, please leave untouched.
#--
#--  $RCSfile: config_wifi.sh,v $
#--  $Revision: 1.1 $
#--  $Date: 2010/10/11 16:17:10 $
#--
#--------------------------------------------------------------------------------



echo "config wifi"

ifconfig wlan0 up

/sbin/wpa_supplicant -B -i wlan0 -c /etc/wpa-example.conf

ifconfig wlan0 192.168.1.102

ping -c 3 192.168.1.101

echo "config complete"


