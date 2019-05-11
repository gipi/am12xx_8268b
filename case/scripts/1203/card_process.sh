#!/bin/sh

#**************This script is call by the task manager *********************
#		Description:
#				 A:When card in
#				     1.install driver module
#				     2.mount the disk to mount point
#				 B:When card out
#				     1.kill relative task and umount the disk
#				     2.remove the driver module
#
#**************************************************************************

lib_dir="/lib/modules/2.6.27.29"
if [ $# -ne 1 ]
then
	echo "usage: " $0 "message_type"
	exit 1
fi

echo "$1"

case  $1  in
	"sdin") insmod $lib_dir/am7x_sdcard.ko
		    if [ $? -eq 0 ]; then
		    	if [ ! -d "/mnt/card" ]; then
		    		mkdir /mnt/card
		    		echo "mkdir /mnt/card"
		    	fi
		    	mdev -s
			if [ -r "/dev/sd_card_block1" ]; then
				mount  /dev/sd_card_block1  /mnt/card
			else
				mount  /dev/sd_card_block  /mnt/card
			fi
		    fi
			;;
	"sdout")
#fuser -kc /mnt/card
	 	     umount /mnt/card
	 	     rmmod am7x_sdcard.ko
	 	     rm /dev/sd_card_block
	 	     if [ -r "/dev/sd_card_block1" ]; then
	 	     	rm /dev/sd_card_block1
	 	     fi
			;;
	"msin")insmod $lib_dir/am7x_mscard.ko
		    if [ $? -eq 0 ]; then
		    	if [ ! -d "/mnt/card" ]; then
		    		mkdir /mnt/card
		    		echo "mkdir /mnt/card"
		    	fi
			mount  /dev/MS  /mnt/card
		    fi
			;;
	"msout")
#fuser -kc /mnt/card
	 	     umount /mnt/card
	 	     rmmod am7x_mscard.ko
			;;
	"xdin")insmod $lib_dir/am7x_xdcard.ko
		    if [ $? -eq 0 ]; then
		    	if [ ! -d "/mnt/card" ]; then
		    		mkdir /mnt/card
		    		echo "mkdir /mnt/card"
		    	fi
			mount  /dev/XD  /mnt/card
		    fi
			;;
	"xdout")
#fuser -kc /mnt/card
	 	     umount /mnt/card
	 	     rmmod am7x_xdcard.ko
			;;
	"cfin")insmod $lib_dir/am7x_cfcard.ko
		    if [ $? -eq 0 ]; then
		    	if [ ! -d "/mnt/cf" ]; then
		    		mkdir /mnt/cf
		    		echo "mkdir /mnt/cf"
		    	fi
			mount  /dev/CF  /mnt/cf
		    fi
			;;
	"cfout")
#fuser -kc /mnt/cf
	 	     umount /mnt/cf
	 	     rmmod am7x_cfcard.ko
			;;
		*)echo   "unknow message type"
			;;
esac

