#!/bin/sh
echo "----------- Run ATOP user.sh---------------" 
sleep 10
ifconfig lo up
ifconfig lo 127.0.0.1
#ifconfig eth0 up 10.0.165.100 netmask 255.255.0.0
echo "----------- v03---------------" 

if [ -e /mnt/atop/.sys_cfg ]; then
	echo "system file exist"
	sbin/set_eth 0 0 down 0 -1 &
	sleep 2;
	sbin/set_eth 0 0 up 0 -1 &
else
	echo "system file not exist"
	if [ -e /mnt/atop/.sys_cfg.bak ]; then
		echo "system backup file exist"
		sbin/set_eth 0 0 down 0 -1 &
		sleep 2;
		sbin/set_eth 0 0 up 0 -1 &
	else
		echo "system backup file not exist"
		ifconfig eth0 up
        #Mos: if eth0 not plugin, udhdpc will always request, and it will block system event?!
        udhcpc eth0 &
		#ifconfig eth0 up 10.0.21.100 netmask 255.255.0.0
		#sbin/atop_prod_ap &
	fi
fi
rm /etc/resolv.conf
sleep 1
#ln -s /etc/resolv.conf /mnt/atop
ln -s /mnt/atop/resolv.conf /etc/resolv.conf

echo 5 > /proc/sys/net/ipv4/tcp_retries2
#sbin/set_eth 0 0 up &
#sleep 5
echo "----------- Run ATOP Main---------------" 
#mnt/udisk/atop_main &
mnt/atop/atop_main &
echo "----------- Run ATOP GWD---------------" 
sbin/gwd &
