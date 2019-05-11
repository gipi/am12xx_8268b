#!/bin/sh

lib_dir="/lib/modules/2.6.27.29"
if [ $# -ne 1 ]
then
	echo "usage: " $0 "message_type"
	exit 1
fi

echo "$1"

case  $1  in
	"dev_mass_in") insmod $lib_dir/am7x_udc.ko use_dma=1
		    if [ $? -eq 0 ]; then
			echo "insert udc"
		    fi

		    insmod $lib_dir/g_file_storage.ko \
			file=/dev/partitions/udisk,/dev/sd_card_block
		    if [ $? -eq 0 ]; then
			echo "dev mass attach"
		    fi
			;;

	"dev_mass_out") rmmod g_file_storage
			rmmod $lib_dir/am7x_udc
			echo "dev mass detatch"
			;;
	"host_raw_in") insmod $lib_dir/am7x_hcd.ko
			if [ $? -eq 0 ]; then
				echo "hcd mod ins ok"
			fi
			;;		
	"host_mass_in")insmod $lib_dir/usb-storage.ko
			if [ $? -eq 0 ]; then
				echo "storage mod ins ok"
			fi
			;;		
	"host_mass_scanok")mdev -s 
			for i in 1 2 3 4
			do
			  if [ -e /dev/sda$i ]; then
				mount /dev/sda$i /mnt/usb
			  else
				break
			  fi
			done
			
			if [ $i -eq 1 ]; then
				echo "disk no partition"
				mount /dev/sda /mnt/usb
			fi
			;;
	"host_mass_out")echo "host mass out"
			fuser -k /mnt/usb
	 		umount /mnt/usb
			rmmod usb-storage.ko
			rm  /dev/sda
			for i in 1 2 3 4
			do
			  if [ -e /dev/sda$i ]; then
				rm /dev/sda$i
			  fi
			done			
			;;
	"host_raw_out")	echo "host raw out"
			rmmod am7x_hcd
			;;
		*)echo  "unknow message type"
			;;
esac
