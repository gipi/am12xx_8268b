#!/bin/sh
#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# some post process for the Miracast exit.
# 
#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# Kill the wifi service.
# Note:Miracast operates on wlan0.
#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
KillWifiServ()
{
    #>> if Ap mode has been invoked.
    killall hostapd
    killall udhcpd
    
    #>> if client mode has been invoked.
    killall wpa_supplicant
    killall udhcpc
}


#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# clean up the WFD remained resources.
#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CleanupWfdRes()
{
    rm -rf /tmp/p2pstatus
    rm -rf /tmp/port.log
    rm -rf /tmp/sourceip.log
    rm -rf /tmp/udhcpd.leases
    rm -rf /tmp/wfd/
}

ps 

KillWifiServ
CleanupWfdRes

ps

ifconfig wlan0 down

echo ">>>>>>>>>>>>>>>>>>>>WFD out"
