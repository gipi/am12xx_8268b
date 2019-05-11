#!/bin/sh
#-------------------------------------------------------------------------------
#
# Autor : Bink
#
# usage: sh  network_control.sh [IP] [TARGET]
#     [IP]        --> The IP to opration, and if the value is "ALL", then set the default rule of filter-FORWARD to [TARGET].
#     [TARGET]    --> The rule to set for [IP], exp: "ACCEPT" is accept this IP to internet, "DROP" is do not accept the IP to internet, "DEL" is delete the rule for this IP.
#
#--------------------------------------------------------------------------------

if [ $# -ne 2 ]; then
	echo "usage: " $0 "pub_interface private_interface private_subnet"
	exit 1
fi

IP=$1
TARGET=$2
srcOkFile="/tmp/srcOk"
desOkFile="/tmp/desOkFile"

let num=0

if [ "$TARGET" != "ACCEPT" -a "$TARGET" != "DROP" -a "$TARGET" != "DEL" ]; then
	echo "TARGET[$TARGET] not found!!"
	exit 1
fi

export XTABLES_LIBDIR=/am7x/lib/
export IPTABLES_LIB_DIR=/am7x/lib/

if [ "$IP" = "ALL" ]; then
	iptables -L FORWARD -t filter | while read line
        do
		thisTar=`echo $line | awk '{print $1}'`
                thisSrc=`echo $line | awk '{print $4}'`
                thisDes=`echo $line | awk '{print $5}'`
		if [ "$thisTar" == "ACCEPT" -o "$thisTar" == "DROP" ];then
			iptables -D FORWARD 1 -t filter
			echo "Delete source <$thisSrc - $thisDes> from $thisTar"
		fi
	done
	if [ "$TARGET" != "DEL" ]; then
		echo "Set FORWARD "$TARGET
		iptables -t filter -P FORWARD $TARGET
	fi
else
	echo "Set "$IP" "$TARGET
	if [ -f $srcOkFile ]; then
		rm $srcOkFile
	fi
	if [ -f $desOkFile ]; then
		rm $desOkFile
	fi
	iptables -L FORWARD -t filter | while read line
	do
		thisTar=`echo $line | awk '{print $1}'`
		thisSrc=`echo $line | awk '{print $4}'`
		thisDes=`echo $line | awk '{print $5}'`
		if [ "$thisTar" == "target" ]; then
			let num=0
		else
			let num+=1
			echo "num: "$num
			if [ "$thisSrc" == "$IP" ]; then
				if [ "$TARGET" != "DEL" ]; then
					iptables -t filter -R FORWARD $num -p all -s $IP -j $TARGET
					touch $srcOkFile
					echo "Replace FORWARD $num to source $IP --> $TARGET"
				else
					iptables -D FORWARD $num -t filter
					let num-=1
					echo "Delete source[$num] $IP"
				fi
			elif [ "$thisDes" == "$IP" ]; then
				if [ "$TARGET" != "DEL" ]; then
					iptables -t filter -R FORWARD $num -p all -d $IP -j $TARGET
					touch $desOkFile
					echo "Replace FORWARD $num to destination $IP --> $TARGET"
				else
					iptables -D FORWARD $num -t filter
					let num-= 1
					echo "Delete destination[$num] $IP"
				fi
			fi
		fi
		
	done
	
	if [ "$TARGET" != "DEL" ]; then
		if [ -f $srcOkFile ]; then
			rm $srcOkFile
		else
			iptables -t filter -A FORWARD -p all -s $IP -j $TARGET
			echo "source $IP --> $TARGET"
		fi
		if [ -f $desOkFile ]; then
			rm $desOkFile
		else
			iptables -t filter -A FORWARD -p all -d $IP -j $TARGET
			echo "destination $IP --> $TARGET"
		fi
	fi
fi

#iptables --list
