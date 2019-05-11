#! /bin/bash

#
#process  wifi client and softap mode server
#
#
echo "process  wifi client and softap mode server"
lib_dir="/lib/modules/2.6.27.29"

case  $1  in
		"ralink_softap_mode") echo "ralink softap mode"
				ifconfig ra0 down
				if [ -r "/tmp/wpa_supplicant" ]; then
						killall wpa_supplicant
				fi
				
				if [ -r "/sys/module/rt5370sta" ]; then
						rmmod rt5370sta
				fi
			
#				echo "========================"	
#				echo $2
				if [ $2 -eq 1 ]; then
					rmmod am7x_hcd
				else
					rmmod am7x_hcd_next
				fi
				sleep 1
				if [ $2 -eq 1 ]; then
					insmod $lib_dir/am7x_hcd.ko
				else
					insmod $lib_dir/am7x_hcd_next.ko
				fi
				sleep 1
				modprobe $lib_dir/rtnet3070ap.ko			
				ifconfig ra0 up
				
				ifconfig ra0 192.168.111.1
			
				killall udhcpd	
				udhcpd /etc/ra0-udhcpd.conf
				thttpd -C /etc/thttpd.conf -u root
						
			;;

		"ralink_client_mode") echo "ralink client mode"

				killall udhcpd
				ifconfig ra0 down
				if [ -r "/sys/module/rtnet3070ap" ]; then
						modprobe -r rtnet3070ap
				fi
#				echo "#####################################"	
#				echo $2		
				if [ $2 -eq 1 ]; then
					rmmod am7x_hcd
				else
					rmmod am7x_hcd_next
				fi
				sleep 1
				if [ $2 -eq 1 ]; then
					insmod $lib_dir/am7x_hcd.ko
				else
					insmod $lib_dir/am7x_hcd_next.ko
				fi
#				sleep 1
				insmod $lib_dir/rt5370sta.ko
				ifconfig ra0 up
			
		;;
		
		
				"ralink_close") echo "ralink_close"

				killall udhcpd
				ifconfig ra0 down
				if [ -r "/sys/module/rtnet3070ap" ]; then
						modprobe -r rtnet3070ap
				fi
#				echo "#####################################"	
#				echo $2		
				if [ $2 -eq 1 ]; then
					rmmod am7x_hcd
				else
					rmmod am7x_hcd_next
				fi
			
		;;


		"realtek_softap_mode") echo "realtek_softap_mode"

			ifconfig wlan0 192.168.111.1
			hostapd -B /etc/rtl_hostapd.conf
			killall udhcpd	
			udhcpd /etc/udhcpd.conf
			thttpd -C /etc/thttpd.conf -u root
		;;
		
		"realtek_concurrent_softap_mode") echo "realtek_concurrent_softap_mode"

			ifconfig wlan1 192.168.111.1
			hostapd -B /etc/rtl_hostapd_01.conf
			killall udhcpd	
			udhcpd /etc/udhcpd_01.conf
			thttpd -C /etc/thttpd.conf -u root
		;;
		
		"realtek_concurrent_softap_mode_EZCast") echo "realtek_concurrent_softap_mode_EZCast"

			ifconfig wlan1 192.168.203.1
			hostapd -B /etc/rtl_hostapd_01.conf
			killall udhcpd	
			udhcpd /tmp/udhcpd_01.conf
			thttpd -C /etc/thttpd.conf -u root
		;;
		
		"realtek_concurrent_softap_mode_EZCastPro") echo "realtek_concurrent_softap_mode_EZCastPro"

			ifconfig wlan1 192.168.168.1
			hostapd -B /etc/rtl_hostapd_01.conf
			killall udhcpd	
			udhcpd /tmp/udhcpd_01.conf
			thttpd -C /etc/thttpd.conf -u root
		;;		
		"realtek_concurrent_softap_mode_EZCastPro2") echo "realtek_concurrent_softap_mode_EZCastPro2"
			ifconfig wlan3 192.168.168.1
			hostapd_02 -B /etc/rtl_hostapd_02.conf
			udhcpd /tmp/udhcpd_02.conf

			thttpd -C /etc/thttpd.conf -u root
			
		;;
			"realtek_client_mode") echo "realtek_client_mode"

			killall udhcpd
			killall hostapd
		;;

esac
