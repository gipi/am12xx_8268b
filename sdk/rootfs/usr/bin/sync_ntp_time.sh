#!/bin/sh

#Mos: Find which NTP server reachable
NTP_SERVER_List="time.google.com pool.ntp.org ntp.ubuntu.com"
for i in $NTP_SERVER_List
do
    ping -c1 -W1 -q $i &>/dev/null
    status=$( echo $? )
    if [[ $status == 0 ]] ; then
        echo "Try $i...OK"
        NTPDSVR="$i"

        ntpd -nqp $NTPDSVR
        status=$( echo $? )
        if [[ $status == 0 ]] ; then
            #ntp sync time sucessfully
            date > /tmp/date.txt
            date +%Y%m%d%H%M.%S > /mnt/vram/date.txt
            sync
            break
        else
            echo "Sync time with $i fail!"
        fi

    else
        echo "Try $i...Fail"
    fi
done

