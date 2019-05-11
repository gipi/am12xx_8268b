#!/bin/sh

conf_file=/tmp/udhcpd_01.conf
result_file=/tmp/udhcpd_check_result
tmp_file=/tmp/udhcpd_tmp
dns_str=dns
opt_str=opt
option_str=option

if [ -f $result_file ]; then
	rm $result_file
fi

if [ ! -f $conf_file ]; then
	echo "udhcpd config file not exist!!!"
	return 1
else
	cat $conf_file | grep $dns_str > $tmp_file
	cat $tmp_file | while read line
	do
		str1=`echo $line | awk '{print $1}'`
		str2=`echo $line | awk '{print $2}'`
		if [ "$str1" = "$opt_str" -o "$str1" = "$option_str" ]; then
			if [ "$str2" = "$dns_str" ]; then
				echo $line
				rm $tmp_file
				touch $result_file
				return 0
			fi
		fi
	done
fi

if [ -f $tmp_file ]; then
	rm $tmp_file
fi
return 2
