#!/bin/bash
echo 'change_softap_config_for_realtek.sh'

filename="/mnt/user1/softap/rtl_hostapd_01.conf";
ssid="ssid=$1";
#wpa="wpa=$2";
pass="wpa_passphrase=$2"
#pass="wpa_passphrase=@@OPEN@@";
echo $pass;
#test="wpa_passphrase=@@OPEN@@";
#if [ "$3"=="$test" ]
#then
#	pass="wpa_passphrase=        "
#fi	
#echo $pass
#echo ${#pass}


o_ssid=`cat $filename|grep "ssid="`;
#o_wpa=`cat $filename|grep "wpa="`;
o_pass=`cat $filename|grep "wpa_passphrase="`;
###test for symbolic
#cat $filename|sed "s/$o_ssid/$ssid/"|sed "s/$o_wpa/$wpa/"|sed "s/$o_pass/$pass/" > "/mnt/user1/softap/rtl_hostapd_1.conf";

#mv "/mnt/user1/softap/rtl_hostapd_1.conf" "/mnt/user1/softap/rtl_hostapd.conf"
#cp -f "/mnt/user1/softap/rtl_hostapd.conf" "/rtl_hostapd.conf"
#cp -f "/mnt/user1/softap/rtl_hostapd.conf" "/etc/rtl_hostapd.conf"
###test for symbolic
cat $filename|sed "s/$o_ssid/$ssid/"|sed "s/$o_pass/$pass/" > "/mnt/user1/softap/rtl_hostapd_01.conf";
cp -f "/mnt/user1/softap/rtl_hostapd_01.conf" "/rtl_hostapd_01.conf"

#chmod 755 /rtl_hostapd_01.conf

echo $ssid
echo $o_ssid
echo $wpa
echo $o_wpa
echo $pass
echo $o_pass

sync
#restart softap



#killall udhcpd

killall hostapd

sleep 2

ifconfig wlan1 192.168.111.1

sleep 2

hostapd -B /etc/rtl_hostapd_01.conf

sleep 2

#su

#udhcpd  /etc/udhcpd.conf



echo 'OK'