#!/bin/sh

lib_dir="/lib/modules/2.6.27.29"
dev_name='/dev/sda'
dev_name_b='/dev/sdb'
dev_name_c='/dev/sdc'
dev_name_d='/dev/sdd'
dev_name_e='/dev/sde'
dev_name_f='/dev/sdf'
dev_name_g='/dev/sdg'
card_slot1=
card_slot2=
partnum=
j=0

if [ $# -ne 1 ]
then
	echo "usage: " $0 "message_type"
	exit 1
fi

#echo "$1"

case  $1  in
	"dev_mass_in") insmod $lib_dir/am7x_udc.ko 
		    if [ $? -eq 0 ]; then
			echo "insert udc"
		    fi
			
			 if [ -r "/dev/sd_card_block" ]; then
					if [ -r "/dev/sd_card_block1" ]; then		
					    card_slot1="/dev/sd_card_block1"
					else
					    card_slot1="/dev/sd_card_block"
					fi
			  elif [ -r "/dev/ms_card_block" ]; then
					if [ -r "/dev/ms_card_block1" ]; then		
					    card_slot1="/dev/ms_card_block1"
					else
					    card_slot1="/dev/ms_card_block"
					fi
			  elif [ -r "/dev/xd_card_block" ]; then
					if [ -r "/dev/xd_card_block1" ]; then		
					    card_slot1="/dev/xd_card_block1"
					else
					    card_slot1="/dev/xd_card_block"
					fi
			  fi
	
			  if [ -r "/dev/cf_card_block" ]; then
					if [ -r "/dev/cf_card_block1" ]; then		
					    card_slot2="/dev/cf_card_block1"
					else
					    card_slot2="/dev/cf_card_block"
					fi
			  fi
		    
		    echo "$card_slot1"
		    echo "$card_slot2"
		
		    fuser  /mnt/udisk
		    fuser  /mnt/card
		    fuser  /mnt/cf
		    umount /mnt/udisk
		    umount /mnt/card
		    umount /mnt/cf
		    
		    if [ $? -ne 0 ]; then
			echo "umount udisk failed!! please check!"
		    fi

		    insmod $lib_dir/g_file_storage.ko \
			file=/dev/partitions/udisk,$card_slot1,$card_slot2
		    if [ $? -eq 0 ]; then
			echo "dev mass attach"
		    fi
			;;

	"dev_mass_out") rmmod g_file_storage
			rmmod am7x_udc
			
			if [ -r "/dev/sd_card_block" ]; then
				if [ -r "/dev/sd_card_block1" ]; then		
				    card_slot1="/dev/sd_card_block1"
				else
				    card_slot1="/dev/sd_card_block"
				fi
		  elif [ -r "/dev/ms_card_block" ]; then
				if [ -r "/dev/ms_card_block1" ]; then		
				    card_slot1="/dev/ms_card_block1"
				else
				    card_slot1="/dev/ms_card_block"
				fi
		  elif [ -r "/dev/xd_card_block" ]; then
				if [ -r "/dev/xd_card_block1" ]; then		
				    card_slot1="/dev/xd_card_block1"
				else
				    card_slot1="/dev/xd_card_block"
				fi
		  fi

		  if [ -r "/dev/cf_card_block" ]; then
				if [ -r "/dev/cf_card_block1" ]; then		
				    card_slot2="/dev/cf_card_block1"
				else
				    card_slot2="/dev/cf_card_block"
				fi
		  fi
			
			
			
			if [ -r $card_slot1 ];then
				mount  -o iocharset=utf8 $card_slot1  /mnt/card
			fi
			
			if [ -r $card_slot2 ];then
				mount  -o iocharset=utf8 $card_slot2  /mnt/cf
			fi

			mount dev/partitions/udisk /mnt/udisk
			if [ $? -ne 0 ]; then
			    echo "mount udisk failed!! please check!"
			fi
			echo "dev mass detatch"
			;;
			
	"dev_debug_out") rmmod g_file_debug
			rmmod am7x_udc
			echo "dev debug out"
			;;
	
	"dev_mass_2_subdisp") rmmod g_file_storage
			insmod $lib_dir/g_subdisplay.ko
			mdev -s
			echo "dev mass 2 subdisp"
			;;
			
	"dev_mass_2_debug") rmmod g_file_storage
			insmod $lib_dir/g_file_debug.ko
			mdev -s
			echo "dev mass 2 debug"
			;;

	"dev_subdisp_2_mass") rmmod g_subdisplay.ko
			insmod $lib_dir/g_file_storage.ko 
			echo "dev subdisp 2 mass"
			;;

	"dev_subdisp_out") rmmod g_subdisplay
			rmmod am7x_udc
			echo "dev subdisp out"
			;;
			
	"dev_debug_2_mass") rmmod g_file_debug
			insmod $lib_dir/g_file_storage.ko  \
			file=/dev/partitions/udisk,$card_slot1,$card_slot2
			echo "dev debug 2 mass"
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

	"host_hid_in")insmod $lib_dir/hid.ko
			insmod $lib_dir/usbhid.ko mousepoll=32
			if [ $? -eq 0 ]; then
				echo "usb hid drv ins ok"
			fi
			;;
	"host_sta_in")
				echo "host sta in do nothing"
			;;	
			
	"host_softap_in")insmod $lib_dir/rtutil3070ap.ko
			insmod $lib_dir/rt3070ap.ko
			insmod $lib_dir/rtnet3070ap.ko
			if [ $? -eq 0 ]; then
				echo "usb softap drv ins ok"
			fi
			;;	
			
	"host_mass_scanok")echo "host scann ok ,prepare mount"
			mdev -s	
			j=0
			a=$(echo '/dev/sda')
			echo $a
			for i in $a 			
			do
				if [ "$i" = "$dev_name" ]; then
					echo "this is /dev/sda"
				else
					partnum=${i##*sda}	
					mount $i /mnt/usb$partnum
					j=1				
				fi
			done
			
			if [ $j -eq 0 ]; then	
				mount /dev/sda /mnt/usb1
			fi
			;;
			
	"host_mass_scanok1")echo "host scann ok ,prepare mount"
			mdev -s	
			j=0
			a=$(echo '/dev/sdb*')
			echo $a
			for i in $a 			
			do
				if [ "$i" = "$dev_name_b" ]; then
					echo "this is /dev/sdb"
				else
					partnum=${i##*sdb}	
					mount $i /mnt/usb$partnum
					j=1				
				fi
			done
			
			if [ $j -eq 0 ]; then	
				mount /dev/sdb /mnt/usb1
			fi
			;;
			
		"host_mass_scanok2")echo "host scann ok ,prepare mount"
			mdev -s	
			j=0
			a=$(echo '/dev/sdc*')
			echo $a
			for i in $a 			
			do
				if [ "$i" = "$dev_name_c" ]; then
					echo "this is /dev/sdc"
				else
					partnum=${i##*sdc}	
					mount $i /mnt/usb$partnum
					j=1				
				fi
			done
			
			if [ $j -eq 0 ]; then	
				mount /dev/sdc /mnt/usb1
			fi
			;;
			
		"host_mass_scanok3")echo "host scann ok ,prepare mount"
			mdev -s	
			j=0
			a=$(echo '/dev/sdd*')
			echo $a
			for i in $a 			
			do
				if [ "$i" = "$dev_name_d" ]; then
					echo "this is /dev/sdd"
				else
					partnum=${i##*sdd}	
					mount $i /mnt/usb$partnum
					j=1				
				fi
			done
			
			if [ $j -eq 0 ]; then	
				mount /dev/sdd /mnt/usb1
			fi
			;;
			
		"host_mass_scanok4")echo "host scann ok ,prepare mount"
			mdev -s	
			j=0
			a=$(echo '/dev/sde*')
			echo $a
			for i in $a 			
			do
				if [ "$i" = "$dev_name_e" ]; then
					echo "this is /dev/sde"
				else
					partnum=${i##*sde}	
					mount $i /mnt/usb$partnum
					j=1				
				fi
			done
			
			if [ $j -eq 0 ]; then	
				mount /dev/sde /mnt/usb1
			fi
			;;
			
			"host_mass_scanok5")echo "host scann ok ,prepare mount"
			mdev -s	
			j=0
			a=$(echo '/dev/sdf*')
			echo $a
			for i in $a 			
			do
				if [ "$i" = "$dev_name_f" ]; then
					echo "this is /dev/sdf"
				else
					partnum=${i##*sdf}	
					mount $i /mnt/usb$partnum
					j=1				
				fi
			done
			
			if [ $j -eq 0 ]; then	
				mount /dev/sdf /mnt/usb1
			fi
			;;
			
			"host_mass_scanok6")echo "host scann ok ,prepare mount"
			mdev -s	
			j=0
			a=$(echo '/dev/sdg*')
			echo $a
			for i in $a 			
			do
				if [ "$i" = "$dev_name_g" ]; then
					echo "this is /dev/sdg"
				else
					partnum=${i##*sdg}	
					mount $i /mnt/usb$partnum
					j=1				
				fi
			done
			
			if [ $j -eq 0 ]; then	
				mount /dev/sdg /mnt/usb1
			fi
			;;

	"host_mass_out")echo "host mass out"
			umount /mnt/usb1
			umount /mnt/usb2
			umount /mnt/usb3
			umount /mnt/usb4
			rmmod usb_storage
			echo "host mass out &umount over"
			;;

	"host_hid_out")	rmmod usbhid
			rmmod hid
			if [ $? -eq 0 ]; then
				echo "usb hid drv uninstall ok"
			fi
			;;
	"host_sta_out")
				echo "usb softap drv uninstall ok"
			;;
	
	"host_softap_out")rmmod rtnet3070ap
			rmmod rt3070ap
			rmmod rtutil3070ap
			if [ $? -eq 0 ]; then
				echo "usb softap drv uninstall ok"
			fi
			;;
					
	"host_raw_out")	echo "host raw out"
			rmmod am7x_hcd
			;;

	"host_bt_in") echo "host_bt_in"
			insmod /lib/modules/2.6.27.29/bluetooth.ko
			insmod /lib/modules/2.6.27.29/hci_usb.ko
			insmod /lib/modules/2.6.27.29/l2cap.ko
			insmod /lib/modules/2.6.27.29/rfcomm.ko
			;;
	"host_bt_out") echo "host_bt_out"
			rmmod /lib/modules/2.6.27.29/bluetooth.ko
			rmmod /lib/modules/2.6.27.29/hci_usb.ko
			rmmod /lib/modules/2.6.27.29/l2cap.ko
			rmmod /lib/modules/2.6.27.29/rfcomm.ko
			;;
		*)echo  "unknow message type"
			;;
esac
