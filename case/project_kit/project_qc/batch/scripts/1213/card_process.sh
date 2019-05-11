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
if [ $# -ne 2 ]
then
	echo "usage: " $0 "message_type"
	exit 1
fi

echo "$1"

case  $1  in
	"sdin") lsmod | grep am7x_sdcard | grep -v carddet > /mnt/udisk/test_sd.txt
		if [ -s /mnt/udisk/test_sd.txt ];then
			cat /mnt/udisk/test_sd.txt
			ls -l /mnt/udisk/
			rmmod am7x_sdcard		
		fi
		rm /mnt/udisk/test_sd.txt	
		
		insmod $lib_dir/am7x_sdcard.ko
		if [ $? -eq 0 ]; then
			if [ ! -d "/mnt/card" ]; then
				mkdir /mnt/card
				echo "mkdir /mnt/card"
			fi
			mdev -s
			if [ $2 = "pc_discon" ];then
				if [ -r "/dev/sd_card_block1" ]; then
					mount  -o iocharset=utf8 /dev/sd_card_block1  /mnt/card
				else
					mount  -o iocharset=utf8 /dev/sd_card_block  /mnt/card
				fi
			fi
		fi
		;;
	"sdout")
#fuser -kc /mnt/card
		if [ $2 = "pc_discon" ];then
			umount /mnt/card
		fi

		mount | awk -F" on " '$1{print$1}' | grep "sd"
		if [ $? -eq 0 ]; then
			echo "card need to be umount..."
			umount -lf /mnt/user1/thttpd/html/airusb/sdcard
		else
			echo "card is umount already..."
		fi

		rmmod $lib_dir/am7x_sdcard.ko
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
			mdev -s
			if [ $2 = "pc_discon" ];then
				if [ -r "/dev/ms_card_block1" ]; then
					mount  -o iocharset=utf8  /dev/ms_card_block1  /mnt/card
				else
					mount  -o iocharset=utf8  /dev/ms_card_block  /mnt/card
				fi
			fi
		fi
			;;
	"msout")
#fuser -kc /mnt/card
		if [ $2 = "pc_discon" ];then
			umount /mnt/card
		fi
		rmmod am7x_mscard.ko
		rm /dev/ms_card_block
		if [ -r "/dev/ms_card_block1" ]; then
			rm /dev/ms_card_block1
		fi
		;;
	"xdin")insmod $lib_dir/am7x_xdcard.ko
		if [ $? -eq 0 ]; then
			if [ ! -d "/mnt/card" ]; then
				mkdir /mnt/card
				echo "mkdir /mnt/card"
			fi
			mdev -s
			if [ $2 = "pc_discon" ];then
				if [ -r "/dev/xd_card_block1" ]; then
					mount  -o iocharset=utf8  /dev/xd_card_block1  /mnt/card
				else
					mount  -o iocharset=utf8  /dev/xd_card_block  /mnt/card
				fi
			fi
		fi
		;;
	"xdout")
#fuser -kc /mnt/card
		if [ $2 = "pc_discon" ];then
			umount /mnt/card
		fi
		rmmod am7x_xdcard.ko
		rm /dev/xd_card_block
		if [ -r "/dev/xd_card_block1" ]; then
			rm /dev/xd_card_block1
		fi
		;;
	"cfin")insmod $lib_dir/am7x_cfcard.ko
		if [ $? -eq 0 ]; then
			if [ ! -d "/mnt/card" ]; then
				mkdir /mnt/card
				echo "mkdir /mnt/card"
			fi
			mdev -s
			if [ $2 = "pc_discon" ];then
				if [ -r "/dev/cf_card_block1" ]; then
					mount  -o iocharset=utf8  /dev/cf_card_block1  /mnt/cf
				else
					mount  -o iocharset=utf8  /dev/cf_card_block  /mnt/cf
				fi
			fi
		fi
		;;
	"cfout")
#fuser -kc /mnt/cf
		if [ $2 = "pc_discon" ];then
			umount /mnt/cf
		fi
		rmmod am7x_cfcard.ko
		rm /dev/cf_card_block
		if [ -r "/dev/cf_card_block1" ]; then
			rm /dev/cf_card_block1
		fi
		;;			
		*)echo   "unknow message type"
		;;
esac

