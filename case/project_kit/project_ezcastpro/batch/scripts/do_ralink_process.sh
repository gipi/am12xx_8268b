#! /bin/bash
#
#process ralink wifi client and softap mode server
#
#

lib_dir="/lib/modules/2.6.27.29"

if [ $# -ne 2]
then
	echo "usage: " $0 "message_type"
	exit 1
fi

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
				
				chmod -R 644 /root/html/
				chmod -R 755 /root/html/cgi-bin/
				ifconfig ra0 192.168.111.1
				
				udhcpd /etc/ra0-udhcpd.conf
				thttpd -C /etc/thttpd.conf
						
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
				sleep 1
				insmod $lib_dir/rt5370sta.ko
				
				ifconfig ra0 up
			
		;;
		
esac