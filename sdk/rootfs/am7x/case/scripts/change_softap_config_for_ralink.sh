#!/bin/bash
echo 'change_softap_config_for_ralink.sh'

filename="/mnt/user1/softap/RT2870AP.dat";
ssid="SSID=$1";
authmode="AuthMode=$2";
encrypType="EncrypType=$3";
pass="WPAPSK=$4"

o_ssid=`cat $filename|grep "^SSID="`;
o_authmode=`cat $filename|grep "^AuthMode="`;
o_encrypType=`cat $filename|grep "^EncrypType="`;
o_pass=`cat $filename|grep "^WPAPSK="`;
#cat $filename|sed "s/$o_ssid/$ssid/"|sed "s/$o_authmode/$authmode/"|sed "s/$o_encrypType/$encrypType/"|sed "s/$o_pass/$pass/" > "/mnt/user1/softap/rtl_hostapd.conf";
cat $filename|sed "s/$o_ssid/$ssid/"|sed "s/$o_authmode/$authmode/"|sed "s/$o_encrypType/$encrypType/"|sed "s/$o_pass/$pass/" > "/mnt/user1/softap/RT2870AP.dat";
cp -f "/mnt/user1/softap/RT2870AP.dat" "/RT2870AP.dat"

#chmod 755 /RT2870AP.dat
chmod 777 /etc/Wireless/RT2870AP/RT2870AP.dat

echo $ssid
echo $o_ssid
echo $authmode
echo $o_authmode
echo $encrypType
echo $o_encrypType
echo $pass
echo $o_pass

sync

chmod 777 /sbin/iwpriv
iwpriv ra0 set AuthMode=$2
iwpriv ra0 set EncrypType=$3
iwpriv ra0 set SSID=$1
test="wpa_passphrase=@@OPEN@@";
if [ "$4"!="$test" ]
then
	iwpriv ra0 set WPAPSK=$4
fi	
