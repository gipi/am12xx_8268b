#!/bin/bash
#-------------------------------------------------------------------------------
#
# Description : Scripts for NAT talbe setting.
#
# usage: iptables_nat pub_interface private_interface private_subnet
#     pub_interface        --> The public net interface.
#     private_interface    --> The internal net interface.
#     private_subnet       --> The internal subnet, e.g. 192.168.111.0/24
#--------------------------------------------------------------------------------

if [ $# -ne 3 ]
then
	echo "usage: " $0 "pub_interface private_interface private_subnet"
	exit 1
fi

EXTIF=$1
INIF=$2
INNET=$3

# install some modules
#modules="ip_tables iptable_nat ip_nat_ftp ip_nat_irc ip_conntrack ip_conntrack_ftp ip_conntrack_irc"
#for mod in $modules
#do
#    testmod=`lsmod | grep "^${mod} " | awk '{print $1}'`
#    if [ "$testmod" == "" ]; then
#            modprobe $mod
#    fi
#done


export XTABLES_LIBDIR=/am7x/lib/
export IPTABLES_LIB_DIR=/am7x/lib/

# clear the NAT table
echo "------------- clear ip talbes"
iptables -F -t nat
iptables -X -t nat
iptables -Z -t nat
iptables -t nat -P PREROUTING  ACCEPT
iptables -t nat -P POSTROUTING ACCEPT
iptables -t nat -P OUTPUT      ACCEPT

echo "--------------- set ip tables"
# 3. 若有內部介面的存在 (雙網卡) 開放成為路由器，且為 IP 分享器！
if [ "$INIF" != "" ]; then
    iptables -A INPUT -i $INIF -j ACCEPT
    echo "1" > /proc/sys/net/ipv4/ip_forward
    if [ "$INNET" != "" ]; then
        for innet in $INNET
        do
            iptables -t nat -A POSTROUTING -s $innet -o $EXTIF -j MASQUERADE
        done
    fi
fi

# 如果你的 MSN 一直無法連線，或者是某些網站 OK 某些網站不 OK，
# 可能是 MTU 的問題，那你可以將底下這一行給他取消註解來啟動 MTU 限制範圍
# iptables -A FORWARD -p tcp -m tcp --tcp-flags SYN,RST SYN -m tcpmss \
#          --mss 1400:1536 -j TCPMSS --clamp-mss-to-pmtu

# 4. NAT 伺服器後端的 LAN 內對外之伺服器設定
# iptables -t nat -A PREROUTING -p tcp -i $EXTIF --dport 80 \
#          -j DNAT --to-destination 192.168.1.210:80 # WWW

# 5. 特殊的功能，包括 Windows 遠端桌面所產生的規則，假設桌面主機為 1.2.3.4
# iptables -t nat -A PREROUTING -p tcp -s 1.2.3.4  --dport 6000 \
#          -j DNAT --to-destination 192.168.100.10
# iptables -t nat -A PREROUTING -p tcp -s 1.2.3.4  --sport 3389 \
#          -j DNAT --to-destination 192.168.100.20

# Enable multicast routing.
#killall mrouted
#mrouted &

