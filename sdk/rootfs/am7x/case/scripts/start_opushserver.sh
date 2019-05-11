#!/bin/sh

#hciconfig hci0 up
echo "===1"
rm -rf /tmp/messagebus.pid
echo "===2"
#rm -rf /tmp/dbus/*
echo "system_bus_socket"
echo "===3"
/usr/local/bin/dbus-daemon --system
echo "===4"
hcid -f /etc/bluetooth/hcid.conf
echo "===5"
sdptool -i hci0 add OPUSH
#sdptool -i hci0 add FTP
echo "===6"
