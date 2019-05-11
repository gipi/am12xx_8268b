#! /bin/bash

#
#process  wifi client and softap mode server
#
#
echo "process  wifi client and softap mode server"
lib_dir="/lib/modules/2.6.27.29"

case  $1  in
		"wifi_bridge_add_br0") echo "wifi_bridge_add_br0"
		
			brctl delif br0 wlan0
			brctl delif br0 wlan1
			ifconfig br0 down
			brctl delbr br0
		
			ifconfig wlan0 down
			ifconfig wlan0 0.0.0.0 up
			
			
			brctl addbr br0
			brctl addif br0 wlan0
			brctl setfd br0 0
			
			ifconfig wlan1 192.168.111.1	
			ifconfig br0 0.0.0.0 up
			;;
			
			
	"wifi_bridge_del_br0") echo "wifi_bridge_del_br0"
	
			
			brctl delif br0 wlan0
			brctl delif br0 wlan1
			ifconfig br0 down
			brctl delbr br0
			
			killall hostapd
			ifconfig wlan1 down
			
			ifconfig wlan1 192.168.111.1 up
			sleep 1
			hostapd -B /etc/rtl_hostapd_01.conf

			;;
			
			
		"wifi_bridge_enable_br0") echo "wifi_bridge_enable_br0"
		
					
					
			ifconfig wlan1 down
			ifconfig wlan1 0.0.0.0 up
			brctl addif br0 wlan1
						
		  killall hostapd
			sleep 2
			hostapd -B /etc/rtl_hostapd_01.conf
			sleep 1
			udhcpc -i br0 -t 10 -n
	
			;;
			
			"wifi_bridge_del_wlan1") echo "wifi_bridge_del_wlan1"
					
			brctl delif br0 wlan0
			brctl delif br0 wlan1
			ifconfig br0 down
			brctl delbr br0
														
			ifconfig wlan1 down
			ifconfig wlan1 192.168.111.1 up
			
		  killall hostapd
			sleep 2
			hostapd -B /etc/rtl_hostapd_01.conf

	
			;;
			
			"wifi_bridge_add_wlan1") echo "wifi_bridge_add_wlan1"
					
			ifconfig wlan1 down
			ifconfig wlan1 0.0.0.0 up
			brctl addif br0 wlan1
						
		  killall hostapd
			sleep 2
			hostapd -B /etc/rtl_hostapd_01.conf

	
			;;
			
			"wifi_bridge_add_eth0_and_wlan1") echo "wifi_bridge_add_eth0_and_wlan1"
			ifconfig eth0 down
			ifconfig wlan1 down
			ifconfig eth0 0.0.0.0 up
			ifconfig wlan1 0.0.0.0 up
			
			killall hostapd
			
			brctl addbr br0
			brctl addif br0 eth0
			brctl addif br0 wlan1
			brctl setfd br0 0
			
			hostapd -B /etc/rtl_hostapd_01.conf
			
			ifconfig br0 up
			udhcpc -i br0 -t 10 -n
			
			
			;;
			
			"wifi_bridge_add_eth0_and_wlan0") echo "wifi_bridge_add_eth0_and_wlan0"
			ifconfig eth0 down
			ifconfig wlan0 down
			ifconfig eth0 0.0.0.0 up
			ifconfig wlan0 0.0.0.0 up
			
			killall hostapd
			
			brctl addbr br0
			brctl addif br0 eth0
			brctl addif br0 wlan0
			brctl setfd br0 0
			
			hostapd -B /etc/rtl_hostapd.conf
			
			ifconfig br0 up
			udhcpc -i br0 -t 10 -n
			
			
			;;
			
			"wifi_bridge_del_eth0_and_wlan0") echo "wifi_bridge_del_eth0_and_wlan0"
			brctl delif br0 eth0
			brctl delif br0 wlan0
			ifconfig br0 down
			brctl delbr br0
														
			ifconfig wlan0 down
			ifconfig wlan0 192.168.111.1 up
			
		  killall hostapd
			sleep 2
			hostapd -B /etc/rtl_hostapd.conf
						
			
			;;
			
			"wifi_bridge_del_eth0_and_wlan1") echo "wifi_bridge_del_eth0_and_wlan1"
			brctl delif br0 eth0
			brctl delif br0 wlan1
			ifconfig br0 down
			brctl delbr br0
														
			ifconfig wlan1 down
			ifconfig wlan1 192.168.111.1 up
			
		  killall hostapd
			sleep 2
			hostapd -B /etc/rtl_hostapd_01.conf
						
			
			;;
			"wifi_bridge_add_br0_and_wlan1_wlan3") echo "wifi_bridge_add_br0_and_wlan1_wlan3"

			ifconfig br0 down
			brctl delbr br0
			brctl addbr br0
			brctl addif br0 wlan1
			brctl addif br0 wlan3
			
			ifconfig wlan1 0.0.0.0 up
						
			ifconfig wlan3 0.0.0.0 up
			ifconfig br0 192.168.168.1 netmask 255.255.255.0 up
			killall udhcpd
			udhcpd /tmp/udhcpd_01.conf	

			killall hostapd
			hostapd -B /etc/rtl_hostapd_01.conf
			hostapd_02 -B /etc/rtl_hostapd_02.conf

			thttpd -C /etc/thttpd.conf -u root 
			
			;;

		

			
esac
