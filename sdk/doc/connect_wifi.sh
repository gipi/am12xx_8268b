#!/bin/sh

echo -e "\n***************************************************************"
echo "'sh $0' connect the default ssid directly. "
echo "You can change the default ap with options list in the bellow:"
echo -e "Usage: sh $0 [option]"
echo "Valid options are:"
echo -e "-s SSID \tset the SSID"
echo -e "-p PWD  \tset the password"
echo -e "-e SEC  \tset the SECURITY method"
echo -e "-q      \tdisconnect the ap"
echo -e "Example:"
echo -e "\t sh $0 -s CS -p act12345"
echo -e "***************************************************************\n"
				
sleep 1;

DIR_PATH=$(pwd)
#####################################
##   loading variables
#####################################

STA_IFACE="wlan0"
SOFTAP_IFACE="wlan1"
ETH_IFACE="eth0"

CONNECT_TIMEOUT=10

WPA_SUPPLICANT="/sbin/wpa_supplicant"
WPA_CLI="/sbin/wpa_cli"
WPA_CONF="/etc/wireless-tkip.conf"
WPA_CONTROL="/tmp/wpa_supplicant"

IFCONFIG="/sbin/ifconfig"
DHCPC="/sbin/udhcpc"

source $DIR_PATH/wifi_set.conf

while getopts "e:s:p:q" opt
	do
		case $opt in
			e)
				CONNECT_SECURITY=$OPTARG
				echo "set CONNECT_SECURITY=$OPTARG"
				;;
			s)
				CONNECT_SSID=$OPTARG
				echo "set CONNECT_SSID=$OPTARG"
				;;
			p)
				CONNECT_PASS=$OPTARG
				echo "set CONNECT_PASS=$OPTARG"
				;;
			q)
				echo -e "\nkill the dnsmasq*****\n"
				killall dnsmasq
									
				echo -e "\nclear ip talbes******\n"
				iptables -F -t nat
				iptables -X -t nat
				iptables -Z -t nat
				iptables -t nat -P PREROUTING  ACCEPT
				iptables -t nat -P POSTROUTING ACCEPT
				iptables -t nat -P OUTPUT      ACCEPT
					
				echo -e "\nkill wpa udhcpc******\n"
				killall wpa_supplicant
				killall udhcpc
					
				$IFCONFIG $STA_IFACE 0.0.0.0 up
					
				echo -e "\nTHE END !"
				exit 0
				;;
			?)
				echo "Please refer to the Usage!"
				exit 0
				;;
		esac
	done

echo -e "\n\nPart -1: Clear and Clean...\n"
killall dnsmasq
killall wpa_supplicant

#################### loading wifi driver ##########################
#echo -e "\n\nPart 0: Loading wifi driver : "$WIFI_DRIVER
#modprobe $WIFI_DRIVER
sleep 1;

beginT=$(date)

echo -e "\n\nPart 1: \n"

echo "CONNECT_SSID=$CONNECT_SSID"
echo "CONNECT_SECURITY=$CONNECT_SECURITY"

$IFCONFIG $STA_IFACE down

$IFCONFIG $STA_IFACE 0.0.0.0 up

#################### start wpa_supplicant ##########################
echo -e "\n\nPart 2: initialise the wpa_supplicant\n"

$WPA_SUPPLICANT -i $STA_IFACE -c $WPA_CONF -B

echo -e "\n\nPart 2:  scan now... \n"
$WPA_CLI  -i $STA_IFACE -p$WPA_CONTROL scan
sleep 4;

echo -e "\n\nPart 2:  get scan_results now... \n"
$WPA_CLI -i $STA_IFACE -p$WPA_CONTROL scan_results
sleep 2;

$WPA_CLI -i $STA_IFACE -p$WPA_CONTROL list_network
##$WPA_CLI -i $STA_IFACE -p/var/run/wpa_supplicant remove_network 0
NETID=$($WPA_CLI -i $STA_IFACE -p$WPA_CONTROL add_network)
echo "netid = $NETID"

#################### start connection ##########################
echo -e "\n\nPart 3: connect to $CONNECT_SSID now! \n"
	
if [ $CONNECT_SECURITY == "OPEN" ]; then
	$WPA_CLI  -i $STA_IFACE -p$WPA_CONTROL set_network $NETID ssid \"$CONNECT_SSID\"
	$WPA_CLI  -i $STA_IFACE -p$WPA_CONTROL set_network $NETID key_mgmt NONE
	$WPA_CLI  -i $STA_IFACE -p$WPA_CONTROL set_network $NETID scan_ssid 0
else 
	$WPA_CLI  -i $STA_IFACE -p$WPA_CONTROL set_network $NETID ssid \"$CONNECT_SSID\"
	$WPA_CLI  -i $STA_IFACE -p$WPA_CONTROL set_network $NETID scan_ssid 0
	$WPA_CLI  -i $STA_IFACE -p$WPA_CONTROL set_network $NETID psk \"$CONNECT_PASS\"
fi

$WPA_CLI  -i $STA_IFACE -p$WPA_CONTROL enable_network $NETID
#$WPA_CLI  -i $STA_IFACE -p$WPA_CONTROL save_config
$WPA_CLI  -i $STA_IFACE -p$WPA_CONTROL select_network $NETID

echo -e "\n\nPart 3: connecting ... Please wait! \n"
$WPA_CLI  -i $STA_IFACE -p$WPA_CONTROL reconnect
sleep 1;

echo -e "\n\nPart 4: \n"

mkdir -p /tmp/net
count=0
reconBegin=$(date)
while [  $count -le $CONNECT_TIMEOUT  ]
	do
	    $WPA_CLI -i $STA_IFACE -p$WPA_CONTROL status > /tmp/net/WPA.STATUS; touch /tmp/net/WPA.OK
	    cat /tmp/net/WPA.STATUS|grep wpa_state
		wpastate=$(cat /tmp/net/WPA.STATUS|grep wpa_state|sed 's/wpa_state\=//')

		if [ "$wpastate" == "COMPLETED" ]; then
			echo -e "#!/bin/sh \nCONNECT_SECURITY=$CONNECT_SECURITY\nCONNECT_SSID=$CONNECT_SSID\nCONNECT_PASS=$CONNECT_PASS" > $DIR_PATH/wifi_set.conf
			break
		else
			sleep 1
			count=`expr $count + 1`
		fi
	done
echo -e "\n***********************************************"
echo -e "wait-reconnect start: $reconBegin"
echo -e "wait-reconnect end:   $(date)"
echo -e "***********************************************\n"

echo "count=$count"
echo "timeout=$CONNECT_TIMEOUT"

if [ $count -ge $CONNECT_TIMEOUT ]; then
	echo " *********** Connect fail ***********"
	exit 0
fi

$WPA_CLI  -i $STA_IFACE -p$WPA_CONTROL status

#################### start getting DHCP Ip  ##########################
echo -e "\n\nPart 5: get DHCP Ip here..\n"

echo "remove udhcpc now"
killall udhcpc

$IFCONFIG $STA_IFACE 0.0.0.0

echo "cmd=$DHCPC -i $STA_IFACE -t 15 -q -i $STA_IFACE"
$DHCPC -i $STA_IFACE -t 15 -q -i $STA_IFACE &					

sleep 6;				

endT=$(date)

echo -e "\n***********************************************"
echo -e "Script beginT: $beginT"
echo -e "Script endT:  $endT"
echo -e "***********************************************\n"

exit 0