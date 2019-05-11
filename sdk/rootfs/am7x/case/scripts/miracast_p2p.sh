#!/bin/sh
# shell script create by Realtek WiFi-CE on 2011/12/20.
# 
# I. Shell Script Command Format:
#       #./p2p.sh [wlanInterface] [enable_mode] [intent] [WPS Method] [PINCODE/00000000] [listen_channel] [op_channel] [MAC Address]
#
# 	a.[wlan Interface] is the interface name for the wireless device. 
#	b.[enable_mode]
#		1-> enable with the WiFi Direct Device mode.
#		2-> enable with the WiFi Direct Client mode.
#		3-> enable with the WiFi Direct GO mode.
# 	c.[intent] is the expect value to be Wi-Fi Direct SoftAP.
#   	           The intent value will be between 1 to 15.
#   		   Please set the intent to 15 if this wireless device must be the Wi-Fi Direct SoftAP. 
# 	d.[WPS Method] WPS method you want to use the PIN Code or PBC.Default PIN Code is 12345670.
# 	e.[MAC Address] start the WiFi Direct functionality to connect with target P2P Device.If not set mac address,just wait for P2P handshake.
#	f.[listen_channel] set the listen channel which this device will stay on this channel and can be found
#
# II. Use example:
# 	1. Start the Wi-Fi Direct functionality and proceed the Wi-Fi Direct handshake
#    	   #./p2p.sh wlan0 1 1 PIN 12345670 6 6 70:F1:A1:93:C2:45
#
# 	2. Start the Wi-Fi Direct functionality and wait for the Wi-Fi Direct handshake.
#    	   #./p2p.sh wlan0 1 15 PBC 00000000 6 6
#
# III. Basic command flow:
#
# 	ifconfig wlan0 up
# 	iwpriv wlan0 p2p_set enable=1					### Enable P2P function Device.
# 	iwlist wlan0 scan
# 	iwpriv wlan0 p2p_set prov_disc=$mac_pbc			### $mac is target P2P's Wlan MAC Address.
# 	iwpriv wlan0 p2p_set nego=$mac					### $mac is target P2P's Wlan MAC Address.
# 	iwpriv wlan0 p2p_get status						### Status=10 is completed .
# 	iwpriv wlan0 p2p_get role						### role = 2 is Client, 3 is Go (softAP).
# 	if role = 2 (Client) then execute wpa_supplicant,
# 	The detecting the "COMPLETED" by using the wpa_cli status then launch the DHCP Client.
# 	if role = 3 (SoftAP) then execute the hostapd and launch the DHCP Server.
# 
#     
############################################################################################################################################

pincode=$5

wlan=$1
mode=$2
it=$3
WPS_Method=$4
stay_ch=$6
op=$7
peermac=$8

#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# test ARP.
#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArpTest()
{
    local cnt sip pmac
    
    pmac=$1
    cnt=1
    while :
    do
        cat /proc/net/arp
        sip=`cat /proc/net/arp | grep -i "$pmac" | awk '{print $1}'`
        if [ "$sip" != "" ];then
            echo " >>>>>>>>> ArpTest success! "
            break
        else
            sleep 1
            let cnt=cnt+1
            if [ "$cnt" == "5" ]; then
                echo " >>>>>>>>> ArpTest failed! "
                break;
            fi
        fi
    done
}
echo init > /tmp/p2pstatus
echo -e "p2p.sh version:2.4"

#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# if softap mode or client mode on, first close
# them as the current version of the realtek
# wifi driver does not allow to P2P and other
# mode exist concurrently.
#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
if [ -e "/tmp/hostapd" ]; then
    echo ">>>> softap mode opened, close it first"
    killall hostapd
    killall udhcpd
    ifconfig wlan1 down
    sleep 2
fi

if [ -e "/tmp/wpa_supplicant" ]; then
    echo ">>>> client mode opened, close it first"
    killall wpa_supplicant
    killall udhcpc
    ifconfig wlan0 down
    sleep 2
fi

#rm -rf /tmp/wfd
#mkdir /tmp/wfd

ifconfig $wlan up
iwpriv $wlan p2p_set enable=0
####enable=1 (p2P Device),enable=2 (Client), enable=3 (P2P Group Owner)######

enbDev="iwpriv $wlan p2p_set enable=1"
enbCli="iwpriv $wlan p2p_set enable=2"
enbGO="iwpriv $wlan p2p_set enable=3"
#############################################################################

send_nego="iwpriv $wlan p2p_set nego=$peermac"

if [ "$mode" == "1" ]; then
	`$enbDev`
	echo -e "$enbDev"
elif [ "$mode" == "2" ]; then
	`$enbCli`
	echo -e "$enbCli"
elif [ "$mode" == "3" ]; then
	`$enbGO`
	echo -e "$enbGO"
fi

DeviceName=`ifconfig $wlan | grep "HWaddr" | cut -d " " -f 10`  # for DMP
#DeviceName=`ifconfig $wlan | grep "HWaddr" | cut -d " " -f 9`  # for PC Linux 
echo -e "Current DeviceName : $DeviceName \n"
iwpriv $wlan p2p_set setDN="ActionsMicro$DeviceName"

iwpriv $wlan p2p_set wfd_type=1		# Set to Miracast Sink device

echo -e " Set wps_method = $WPS_Method \n"
	
if [ "$WPS_Method" == "PIN" ]; then
	iwpriv $wlan p2p_set got_wpsinfo=1
	#pincode="12345670"
	echo -e "Use PIN Code : $pincode"
else
	iwpriv $wlan p2p_set got_wpsinfo=3
fi

iwpriv $wlan p2p_set intent=$it
iwpriv $wlan p2p_set listen_ch=$stay_ch
iwpriv $wlan p2p_set op_ch=$op
iwpriv $wlan p2p_set profilefound=0
iwpriv $wlan p2p_set ssid=DIRECT-xy

###################### Pass the persistent group information to WiFi driver
mkdir /tmp/wfd
cd /tmp/wfd/
ls -1 *.conf > /tmp/wfd/list.txt
sleep 1
cd -
n=0
while read curline
do
tmp=1`echo $curline | awk -F '.' '{print $1}'`
if [ ${#tmp} -ge 25 ]; then
	iwpriv $wlan p2p_set profilefound=$tmp
	let n=n+1
fi

done < "/tmp/wfd/list.txt"

# If n is not 0, it means the script found the available profile for persistent group
echo -e "Found $n profiles\n"
iwpriv $wlan p2p_set persistent=1
######################

echo -e "check_nego :WPS $WPS_Method :intent:$it"
iwpriv $wlan p2p_set sa=1

if [ "$mode" == "1" ]; then
	if [ "$peermac" != "" ]; then  # Try to connect to peer P2P device
		while :
		do
			scan_result=`iwlist $wlan scan|grep $peermac`
		
			if [ "$scan_result" != "" ]; then	# Found peer P2P device
				break;
			else								# Not Found
				sleep 1
			fi
		done
		
		if [ "$WPS_Method" == "PBC" ]; then
			provdisc="iwpriv $wlan p2p_set prov_disc=$peermac""_pbc"
			echo -e $provdisc
			iwpriv $wlan p2p_set prov_disc=$peermac"_pbc"
		elif [ "$WPS_Method" == "PIN" ]; then
			provdisc="iwpriv $wlan p2p_set prov_disc=$peermac""_keypad"
			echo -e $provdisc	
			iwpriv $wlan p2p_set prov_disc=$peermac"_keypad"
		fi
		
		sleep 1
		
		while :
		do
			echo -e "execute get P2P Status \n"
			Status=`iwpriv $wlan p2p_get status|grep Status=`
			echo -e "Current check_nego : $Status \n"
			
			if [ "$Status" == "Status=07" ]; then
				$send_nego
				sleep 1
				break;
			fi
			sleep 1
		done
				
	fi
		
	while :
		do
				
		####### check P2P status nego completed########
		#echo -e "execute get P2P Status \n"
		Status=`iwpriv $wlan p2p_get status|grep Status=`
		#echo -e "Current check_nego : $Status \n"
			
		if [ "$Status" == "Status=08" ]; then
			if [ "$WPS_Method" == "PBC" ]; then
				echo -e "..............Push Button................... "
			else
				echo -e "PIN Code : [ $pincode] "
			fi
		fi
		
		if [ "$Status" == "Status=09" ]; then
			echo started > /tmp/p2pstatus
		fi

		if [ "$Status" == "Status=10" ]; then
			echo -e "Now Nego status completed: $Status \n"
			echo p2pok > /tmp/p2pstatus
			break
		fi
		
		if [ "$Status" == "Status=11" ]; then
		   	echo -e "Now status is 11 : $Status \n"
			echo -e "check_nego CMD: $send_nego\n"
			echo failed > /tmp/p2pstatus
			if [ "$peermac" == "" ]; then
				$send_nego
			fi
		fi
		
		if [ "$Status" == "Status=12" ]; then
			#### Peer is P2P GO, I am P2P Client.
			#### Start wpa_supplicant
			echo persistent > /tmp/p2pstatus
			peermac=`iwpriv $wlan p2p_get peer_ifa | grep MAC | awk ' $11 != " awk "{print $2}'`
			echo -e "wpa_supplicant -Dwext -i$wlan -c /tmp/wfd/$peermac.conf -d\n"
			wpa_supplicant -Dwext -i$wlan -c /tmp/wfd/$peermac.conf -d&

			echo -e "execute the dhclient to get IP"
			while :
			do
				wpa_status=`wpa_cli status|grep wpa_state=COMPLETED`
				echo -e "Current wpa_status : $wpa_status \n"
				if [ "$wpa_status" == "wpa_state=COMPLETED" ]; then
					echo success > /tmp/p2pstatus
					iwpriv $wlan p2p_get peer_port > /tmp/port.log
					break
				else
					sleep 1
				fi
			done
			sleep 2
#		    /sbin/udhcpc -n -s ./udhcpc.script -i $wlan
            udhcpc -n -i $wlan
        	if [ "$?" -ne "0" ]; then
#            		/sbin/udhcpc -n -s ./udhcpc.script -i $wlan
                    udhcpc -n -i $wlan
            		if [ "$?" -ne "0" ]; then
#                  			/sbin/udhcpc -n -s ./udhcpc.script -i $wlan
                            udhcpc -n -i $wlan
            		fi
        	fi
			echo success > /tmp/p2pstatus
			echo "---------- i am client success"
			exit
		fi
		
		if [ "$Status" == "Status=18" ]; then
			#### I am P2P GO, peer is P2P Client.
			#### Start hostapd			
			
			echo -e "execute hostapd \n"
#			cp ./rtl_hostapd.conf /tmp/wfd/rtl_hostapd.conf
            cp /etc/p2p_hostapd.conf /tmp/wfd/rtl_hostapd.conf
			/usr/local/bin/hostapd /tmp/wfd/rtl_hostapd.conf -dd &
			#./hostapd /tmp/wfd/rtl_hostapd.conf &
			sleep 2
	
			ifconfig $wlan 192.168.111.1
			touch /tmp/udhcpd.leases
#			cp ./udhcpd.conf /tmp/wfd/udhcpd.conf
#			/usr/sbin/udhcpd /tmp/wfd/udhcpd.conf &
            cp /etc/udhcpd.conf /tmp/wfd/udhcpd.conf
			udhcpd /tmp/wfd/udhcpd.conf &
			
			echo success > /tmp/p2pstatus
			
			sleep 10
			iwpriv $wlan p2p_get peer_port > /tmp/port.log
			echo "--------------- i am group owner success"
			exit
		fi
		
		if [ "$Status" == "Status=19" ]; then
			#### The peer P2P device is requesting us to join an existing P2P Group.
			echo -e "execute get P2P role \n"
			roleres=`iwpriv $wlan p2p_get role |grep Role=`
			peermac=`iwpriv $wlan p2p_get peer_ifa | grep MAC | awk ' $11 != " awk "{print $2}'`
			peerdevmac=`iwpriv $wlan p2p_get inv_peer_deva | grep MAC | awk ' $11 != " awk "{print $2}'`
			echo -e "roleres=$roleres, peermac=$peermac, peerdevmac=$peerdevmac\n"
			while :
			do
					scan_result=`iwlist $wlan scan|grep $peermac`
		
					if [ "$scan_result" != "" ]; then
						break;
					fi
			done
		
			if [ "$WPS_Method" == "PBC" ]; then
				provdisc="iwpriv $wlan p2p_set prov_disc=$peerdevmac""_pbc"
				echo -e $provdisc
				iwpriv $wlan p2p_set prov_disc=$peerdevmac"_pbc"
			elif [ "$WPS_Method" == "PIN" ]; then
				provdisc="iwpriv $wlan p2p_set prov_disc=$peerdevmac""_keypad"
				echo -e $provdisc	
				iwpriv $wlan p2p_set prov_disc=$peerdevmac"_keypad"
			fi
			
			sleep 2

			echo -e "on client \t execute WPS pin/pbc \n"
			cp ./wpa.conf /tmp/wfd/conn.conf
			/tmp/wfd/conn_uu
			sleep 1
			wpa_supplicant -Dwext -i$wlan -c /tmp/wfd/conn.conf -d&
			sleep 2

			if [ "$WPS_Method" == "PIN" ]; then
				echo -e "wps PIN ON"
				info="wpa_cli wps_pin $peermac $pincode"
				echo -e $info
				wpa_cli wps_pin $peermac $pincode	
			fi

			if [ "$WPS_Method" == "PBC" ]; then
				echo -e "wps PBC ON"
				wpa_cli wps_pbc $peermac
			fi

			echo -e "execute the dhclient to get IP"
			line=0
			while :
			do
				wpa_status=`wpa_cli status|grep wpa_state=COMPLETED`
				echo -e "Current wpa_status : $wpa_status \n"
				if [ "$wpa_status" == "wpa_state=COMPLETED" ]; then
					echo success > /tmp/p2pstatus
					iwpriv $wlan p2p_get peer_port > /tmp/port.log
					break
				else
					sleep 1
				fi
			done
			
			sleep 5
#			/sbin/udhcpc -n -s ./udhcpc.script -i $wlan
            udhcpc -n -i $wlan
			if [ "$?" -ne "0" ]; then
#				/sbin/udhcpc -n -s ./udhcpc.script -i $wlan
                udhcpc -n -i $wlan
				if [ "$?" -ne "0" ]; then
#					/sbin/udhcpc -n -s ./udhcpc.script -i $wlan
                    udhcpc -n -i $wlan
				fi
			fi

			wpa_cli save_config
			sleep 1

			bssid=""
			line=1
			while :
			do
				data=`head -n $line /tmp/wfd/conn.conf | tail -n 1`
				if [ "$data" == "}" ]; then
					break
				fi
				tmp=`echo $data | awk -F 'ssid="' '{print $2}' | awk -F '"' '{print $1}'`
				if [ "$tmp" != "" ]; then
					ssid=$tmp
				fi
						
				tmp=`echo $data | awk -F 'bssid=' '{print $2}'`
				if [ "$tmp" != "" ]; then
					bssid=$tmp
				fi
				let line=line+1
			done
			
			### Commented by Albert 20120630
			### For some platform, the network profile won't store the bssid information for the current link.
			### In this case, try to get the bssid from the wpa_cli status
			if [ "$bssid" == "" ]; then
				wpa_cli status > /tmp/wfd/status.inf
				sleep 1
				line=1
				
				while :
				do
					data=`head -n $line /tmp/wfd/status.inf | tail -n 1`
					if [ "$data" == "" ]; then
						### The end of the status.inf file
						break;
					fi
					tmp=`echo $data | awk -F 'bssid=' '{print $2}'`
					if [ "$tmp" != "" ]; then
						bssid=$tmp
						break;
					fi
					let line=line+1
				done
				rm -f /tmp/wfd/status.inf
			fi
			
			cp /tmp/wfd/conn.conf /tmp/wfd/$bssid${#ssid}$ssid.conf
			# Convert the bssid to uppercase string
			bssid="$(echo ${bssid} | tr 'a-z' 'A-Z')"
			cp /tmp/wfd/conn.conf /tmp/wfd/$bssid.conf

			exit
		fi

	done
    
    echo "---------------- status is $Status"
	echo -e "check_Role"
	roleres=0
	while :
		do 
		    echo -e "execute get P2P role \n"
			roleres=`iwpriv $wlan p2p_get role |grep Role=`
		    echo -e "check_Role : $roleres \n"
		if [ "$roleres" != "Role=01" ]; then
			break
		else
			echo -e "check_Role : Role not 2 or 3 \n"
			iwlist $wlan scan		
		fi
	done
	
		echo -e "check_role "
		method=$WPS_Method
		echo -e "check_role :WPS method = $method "
		if [ "$roleres" == "Role=02" ]; then

			echo -e "on client \t execute WPS pin/pbc \n"
			cp /etc/miracast_wpa.conf /tmp/wfd/conn.conf
			wpa_supplicant -Dwext -i$wlan -c /tmp/wfd/conn.conf -d&
			sleep 1

			mac=`iwpriv $wlan p2p_get peer_ifa | grep MAC | awk ' $11 != " awk "{print $2}'`
			
			echo -e "------------- mac is $mac \n"
			
			if [ "$method" == "PIN" ]; then
				echo -e "wps PIN ON"
#				mac=`iwpriv $wlan p2p_get peer_ifa | grep MAC | awk ' $11 != " awk "{print $2}'`
				info="wpa_cli wps_pin $mac $pincode"
                                echo -e $info
				wpa_cli -p /tmp/wpa_supplicant wps_pin $mac $pincode	
			fi

			if [ "$method" == "PBC" ]; then
				echo -e "wps PBC ON"
				wpa_cli -p /tmp/wpa_supplicant wps_pbc $mac
			fi

			echo -e "execute the dhclient to get IP"
			line=0
			while :
			do
				wpa_status=`wpa_cli -p /tmp/wpa_supplicant status|grep wpa_state=COMPLETED`
				echo -e "Current wpa_status : $wpa_status \n"
				if [ "$wpa_status" == "wpa_state=COMPLETED" ]; then
					echo success > /tmp/p2pstatus
					iwpriv $wlan p2p_get peer_port > /tmp/port.log
					break
				fi
				sleep 2
			done

			if [ -e /mnt/vram/wifi ]; then
			    echo "/mnt/vram/wifi exists"
			else
			    mkdir -p /mnt/vram/wifi
			fi
#		        /sbin/udhcpc -n -s ./udhcpc.script -i $wlan
		        udhcpc -n -i $wlan
        		if [ "$?" -ne "0" ]; then
#              			/sbin/udhcpc -n -s ./udhcpc.script -i $wlan
                        udhcpc -n -i $wlan
              			if [ "$?" -ne "0" ]; then
#                    			/sbin/udhcpc -n -s ./udhcpc.script -i $wlan
                                udhcpc -n -i $wlan
              			fi
        		fi

			wpa_cli -p /tmp/wpa_supplicant save_config
			sleep 1
			ifconfig $wlan
			
			bssid=""
			line=1
			while :
			do
				data=`head -n $line /tmp/wfd/conn.conf | tail -n 1`
				if [ "$data" == "}" ]; then
					break;
				fi
				tmp=`echo $data | awk -F 'ssid="' '{print $2}' | awk -F '"' '{print $1}'`
				if [ "$tmp" != "" ]; then
					ssid=$tmp
				fi
						
				tmp=`echo $data | awk -F 'bssid=' '{print $2}'`
				if [ "$tmp" != "" ]; then
					bssid=$tmp
				fi
				let line=line+1
			done
			
			### Commented by Albert 20120630
			### For some platform, the network profile won't store the bssid information for the current link.
			### In this case, try to get the bssid from the wpa_cli status
			if [ "$bssid" == "" ]; then
				wpa_cli -p /tmp/wpa_supplicant status > /tmp/wfd/status.inf
				sleep 1
				line=1
				
				while :
				do
					data=`head -n $line /tmp/wfd/status.inf | tail -n 1`
					if [ "$data" == "" ]; then
						### The end of the status.inf file
						break;
					fi
					tmp=`echo $data | awk -F 'bssid=' '{print $2}'`
					if [ "$tmp" != "" ]; then
						bssid=$tmp
						break;
					fi
					let line=line+1
				done
				rm -f /tmp/wfd/status.inf
			fi
			
			cp /tmp/wfd/conn.conf /tmp/wfd/$bssid${#ssid}$ssid.conf
			# Convert the bssid to uppercase string
			bssid="$(echo ${bssid} | tr 'a-z' 'A-Z')"
			cp /tmp/wfd/conn.conf /tmp/wfd/$bssid.conf
			
			sleep 1
			
			echo -e "------ get infortion about peer \n"
			
			##########################################################
			# Get the peer mac address and source ip address
			##########################################################
			arp_cnt=1
			while :
			do
#			    arp -n -i $wlan
				cat /proc/net/arp
                source_ip=`cat /proc/net/arp | grep -i "$mac" | awk '{print $1}'`
                if [ "$source_ip" != "" ];then
                    echo $source_ip > /tmp/sourceip.log
                    echo " ------ source ip is : $source_ip "
                    break
                else
			        sleep 2
			        let arp_cnt=arp_cnt+1
			        if [ "$arp_cnt" == "5" ]; then
			            break;
			        fi
			    fi
			done
			    
			echo -e "\n\n client mode -------------- peer mac is $mac, ip address is $source_ip"
								
		fi
		if [ "$roleres" == "Role=03" ]; then
            op=`iwpriv wlan0 p2p_get op_ch | grep "Op_ch=" | awk -F '=' '{print $2}'`
			echo success > /tmp/p2pstatus
			
			ifconfig $wlan 192.168.111.1 netmask 255.255.255.0
			
			#>>>>>>>>>>>>>>>>>
			# start hosapd.
			#>>>>>>>>>>>>>>>>>
			cp /etc/miracast_hostapd.conf /tmp/wfd/rtl_hostapd.conf
			echo -e "execute hostapd,op=$op \n"	
			go_ch $op
            /usr/local/bin/hostapd /tmp/wfd/rtl_hostapd.conf -dd & 
			sleep 1
			
			#>>>>>>>>>>>>>>>>
			# check the hostapd start ok.
			#>>>>>>>>>>>>>>>>
			check_cnt=1
			while :
			do
				if [ -e "/tmp/hostapd" ];then
				    break;
				else
				    echo "waiting hostapd start..."
				    sleep 1
				    let check_cnt=check_cnt+1
				    if [ "$check_cnt" == "10" ]; then
				        echo "WARNING:hostapd may start error"
			            break;
			        fi
				fi
				
			done
			
			echo -e "wps ON"
			if [ "$method" == "PIN" ]; then

				info="hostapd_cli wps_pin any $pincode"
                echo -e $info
				hostapd_cli wps_pin any $pincode
			fi
			if [ "$method" == "PBC" ]; then
                echo -e "execute hostapd_cli wps_pbc \n"
				info="hostapd_cli wps_pbc"
                echo -e $info
				hostapd_cli wps_pbc
			fi
			
			sleep 2
			
			iwpriv $wlan p2p_get peer_port > /tmp/port.log
			
			rm -rf /tmp/udhcpd.leases
			touch /tmp/udhcpd.leases
			cp /etc/miracast_udhcpd.conf /tmp/wfd/udhcpd.conf
            echo "--------start udhpcd"
			udhcpd -f /tmp/wfd/udhcpd.conf &
			
			##########################################################
			# Get the peer mac address and source ip address
			##########################################################
			mac=`iwpriv $wlan p2p_get peer_ifa | grep MAC | awk ' $11 != " awk "{print $2}'`
			sleep 1
			
			rm -rf /tmp/sourceip.log
			
			get_dhcpd_peer_ip $mac /tmp/udhcpd.leases	/tmp/sourceip.log
			
			arp_cnt=1
			while :
			do
			    get_dhcpd_peer_ip $mac /tmp/udhcpd.leases	/tmp/sourceip.log
			    if [ -e /tmp/sourceip.log ]
			    then
			        echo " peer ip address:"
			        cat /tmp/sourceip.log
			        break
			    else
			    	sleep 2
			    	let arp_cnt=arp_cnt+1
			        if [ "$arp_cnt" == "10" ]; then
			            echo " could not get peer ip address"
			            break;
			        fi
			    fi
			done
			
			if [ -e /tmp/sourceip.log ]
			then
			    peer_ip=`cat /tmp/sourceip.log`
			    
			    # ping the peer device
			    echo ">>>>>>>> begin ping the peer device:"
			    ping -c 3 $peer_ip
			    
			    # do arp test to make sure peer device in correct status.
			    ArpTest $mac
			    
			fi
			
			echo -e "\n\n ap mode -------------- peer mac is $mac"
			
			# execute stopall first, otherwise the IP for wlan0 will disappear.
		fi
elif [ "$mode" == "2" ]; then
###### P2P Client mode
	while :
	do 
		scan_result=`iwlist $wlan scan|grep $peermac`
		
		if [ "$scan_result" != "" ]; then
			break;
		fi
	done
		
	if [ "$WPS_Method" == "PBC" ]; then
		provdisc="iwpriv $wlan p2p_set prov_disc=$peermac""_pbc"
		echo -e $provdisc
		iwpriv $wlan p2p_set prov_disc=$peermac"_pbc"
	elif [ "$WPS_Method" == "PIN" ]; then
		provdisc="iwpriv $wlan p2p_set prov_disc=$peermac""_keypad"
		echo -e $provdisc	
		iwpriv $wlan p2p_set prov_disc=$peermac"_keypad"
	fi
	
	sleep 2
	
	echo -e "on client \t execute WPS pin/pbc \n"
	cp ./wpa.conf /tmp/wfd/conn.conf
	wpa_supplicant -Dwext -i$wlan -c /tmp/wfd/conn.conf -d&
	sleep 2

	if [ "$WPS_Method" == "PIN" ]; then
		echo -e "wps PIN ON"
		info="wpa_cli wps_pin $peermac $pincode"
		echo -e $info
		wpa_cli wps_pin $peermac $pincode	
	fi

	if [ "$WPS_Method" == "PBC" ]; then
		echo -e "wps PBC ON"
		wpa_cli wps_pbc $peermac
	fi

	echo -e "execute the dhclient to get IP"
	line=0
	while :
	do
		wpa_status=`wpa_cli status|grep wpa_state=COMPLETED`
		echo -e "Current wpa_status : $wpa_status \n"
		if [ "$wpa_status" == "wpa_state=COMPLETED" ]; then
			break
		else
			sleep 1
		fi
	done
	sleep 5
        /sbin/udhcpc -n -s ./udhcpc.script -i $wlan
        if [ "$?" -ne "0" ]; then
              /sbin/udhcpc -n -s ./udhcpc.script -i $wlan
              if [ "$?" -ne "0" ]; then
                    /sbin/udhcpc -n -s ./udhcpc.script -i $wlan
              fi
        fi

	wpa_cli save_config
	sleep 1

	line=1
			while :
			do
				data=`head -n $line /tmp/wfd/conn.conf | tail -n 1`
				if [ "$data" == "}" ]; then
					break;
				fi
				tmp=`echo $data | awk -F 'ssid="' '{print $2}' | awk -F '"' '{print $1}'`
				if [ "$tmp" != "" ]; then
					ssid=$tmp
				fi
						
				tmp=`echo $data | awk -F 'bssid=' '{print $2}'`
				if [ "$tmp" != "" ]; then
					bssid=$tmp
				fi
				let line=line+1
			done
			cp /tmp/wfd/conn.conf /tmp/wfd/$bssid${#ssid}$ssid.conf
			# Convert the bssid to uppercase string
			bssid="$(echo ${bssid} | tr 'a-z' 'A-Z')"
			cp /tmp/wfd/conn.conf /tmp/wfd/$bssid.conf
	
elif [ "$mode" == "3" ]; then
###### P2P GO mode
	echo -e "execute hostapd \n"
	#./hostapd /tmp/wfd/rtl_hostapd.conf/ -dd &
	/usr/local/bin/hostapd /tmp/wfd/rtl_hostapd.conf &
	sleep 2
	
	#### Wait for peer's provision discovery frame.
	while :
		do 

		####### check P2P status nego completed########
		echo -e "execute get P2P Status \n"
		Status=`iwpriv $wlan p2p_get status|grep Status=`
		echo -e "Current check_nego : $Status \n"
			
		if [ "$Status" == "Status=08" ]; then
			if [ "$WPS_Method" == "PBC" ]; then
				echo -e "..............Push Button................... "
			else
				echo -e "PIN Code : [ $pincode] "
			fi
			
			break
		else
			sleep 1
		fi		
	done
		
	echo -e "wps ON"
	if [ "$WPS_Method" == "PIN" ]; then
		info="hostapd_cli wps_pin any $pincode"
        	echo -e $info
		hostapd_cli wps_pin any $pincode
	fi
	if [ "$WPS_Method" == "PBC" ]; then
		info="hostapd_cli wps_pbc"
		echo -e $info
		hostapd_cli wps_pbc
	fi
			
	ifconfig $wlan 192.168.0.1
	touch /tmp/udhcpd.leases
	cp ./udhcpd.conf /tmp/wfd/udhcpd.conf
	/usr/sbin/udhcpd /tmp/wfd/udhcpd.conf &

fi
