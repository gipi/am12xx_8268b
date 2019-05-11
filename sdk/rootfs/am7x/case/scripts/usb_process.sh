#!/bin/sh

lib_dir="/lib/modules/2.6.27.29"
dev_name='/dev/sda'
dev_name_b='/dev/sdb'
dev_name_c='/dev/sdc'
dev_name_d='/dev/sdd'
card_slot1=
card_slot2=
partnum=
partnum1=
j=0
eeprom_type=0
iso_flag=0
#if [ $# -ne 1 ]
#then
#	echo "usage: " $0 "message_type"
#	exit 1
#
#echo "$1"

case  $1  in
	"dev_mass_in")   echo "\$2:$2"
				if [ $2 -eq 1 ]; then
				insmod $lib_dir/am7x_udc.ko 
				else
				insmod $lib_dir/am7x_udc_next.ko 
				fi
				
		    if [ $? -eq 0 ]; then
				echo "insert usb device controller"
		    fi
		    
		    echo "\$3:$3"
				if [ $3 -eq 1 ]; then
			 	mount -t auto -o loop  /mnt/cdrom/*.iso  /mnt/cdrom
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

			if [ $eeprom_type -eq 1 ]; then
			insmod $lib_dir/i2c-am7x.ko 
			insmod $lib_dir/at24_i2c.ko 
			elif [ $eeprom_type -eq 2 ]; then
			insmod $lib_dir/am7x_spi.ko 
			insmod $lib_dir/at93_spi.ko 
			fi
				if [ $3 -eq 1 ]; then
					if [ $2 -eq 1 ]; then
				 			insmod $lib_dir/g_file_storage.ko \
							file=/dev/partitions/udisk,$card_slot1,$card_slot2,/dev/loop0 cdrom=1 autorun=1 msg_identify=0 
					else
				 			insmod $lib_dir/g_file_storage.ko \
							file=/dev/partitions/udisk,$card_slot1,$card_slot2,/dev/loop0 cdrom=1 autorun=1 msg_identify=1
					fi
				else
					if [ $2 -eq 1 ]; then
	  			insmod $lib_dir/g_file_storage.ko \
					file=/dev/partitions/udisk,$card_slot1,$card_slot2 msg_identify=0
					else
					 insmod $lib_dir/g_file_storage.ko \
					file=/dev/partitions/udisk,$card_slot1,$card_slot2 msg_identify=1
					fi
				fi
				
		    if [ $? -eq 0 ]; then
			echo "dev mass attach"
		    fi
			;;		
	"dev_mass_out") rmmod g_file_storage
			if [ -r "/sys/module/am7x_udc" ]; then
				echo "remove am7x udc"
				rmmod am7x_udc
			elif [ -r "/sys/module/am7x_udc_next" ]; then
				echo "remove am7x udc next"
				rmmod am7x_udc_next
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

#bluse.lee add for remove udisk symbox
	"dev_remove_udisk") rmmod g_file_storage
		sleep 1
		insmod $lib_dir/g_file_storage.ko
		;;
	"dev_mass_2_subdisp") rmmod g_file_storage
			if [ -r "/sys/module/am7x_udc" ]; then
					echo "<am7x udc>dev mass 2 subdisp"
			    insmod $lib_dir/g_subdisplay.ko msg_display=0
			elif [ -r "/sys/module/am7x_udc_next" ]; then
					echo "<am7x udc next>dev mass 2 subdisp"
					insmod $lib_dir/g_subdisplay.ko msg_display=1
			fi
			mdev -s
			;;
			
	"dev_mass_2_debug") rmmod g_file_storage
			insmod $lib_dir/g_file_debug.ko
			mdev -s
			echo "dev mass 2 debug"
			;;

	"dev_subdisp_2_mass") fuser -k /dev/usb_subdisp
			sleep 1
			if [ -r "/dev/sd_card_block" ]; then
				if [ -r "/dev/sd_card_block1" ]; then		
				    card_slot1="/dev/sd_card_block1"
				else
				    card_slot1="/dev/sd_card_block"
				fi
		  elif [ -r "dev/cf_card_block" ]; then
				if [ -r "/dev/cf_card_block1" ]; then		
				    card_slot2="cf_card_block1"
				else
				    card_slot2="cf_card_block"
				fi
			fi
			rmmod g_subdisplay.ko
			insmod $lib_dir/g_file_storage.ko \
			file=/dev/partitions/udisk,$card_slot1,$card_slot2
			echo "dev subdisp 2 mass"
			;;

	"dev_subdisp_out") fuser -k /dev/usb_subdisp
			sleep 1
			rmmod g_subdisplay
			if [ -r "/sys/module/am7x_udc" ]; then
					rmmod am7x_udc
					echo "<am7x udc>dev subdisp out"
			elif [ -r "/sys/module/am7x_udc_next" ]; then
					rmmod am7x_udc_next
					echo "<am7x udc next>dev subdisp out"
			fi
			;;
			
	"dev_debug_2_mass") rmmod g_file_debug
			insmod $lib_dir/g_file_storage.ko  \
			file=/dev/partitions/udisk,$card_slot1,$card_slot2
			echo "dev debug 2 mass"
			;;

	"host_raw_in") echo "load hcd controller"
			insmod $lib_dir/am7x_hcd.ko 
			if [ $? -eq 0 ]; then
				echo "hcd mod ins ok"
			fi
			;;
	
	"host_raw_in1") echo "load hcd next controller"
			insmod $lib_dir/am7x_hcd_next.ko 
			if [ $? -eq 0 ]; then
				echo "hcd<1> mod ins ok"
			fi
			;;
			
	"host_mass_in")
			if [ -r "/sys/module/usb_storage" ]; then
					echo "have usb-storage"
			else
					insmod $lib_dir/usb-storage.ko
			fi 
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
			
#			a=$(echo '/dev/sda*')
      a=$(ls /dev/sda*)
			echo $a
			for i in $a 			
			do
				
				if [ "$i" = "$dev_name" ]; then
					echo "this is /dev/sda"
				else
					echo "this is sda"
					partnum=${i##*sda}
					echo "partnum0 is $partnum"	
				#	mount $i /mnt/usb$partnum
					mount $i /mnt/usb$k
					j=1				
				fi
			done
			
			if [ $j -eq 0 ]; then	
				mount /dev/sda /mnt/usb1
			fi
			;;
		
			"host_mass_scanok_sda_hcd")echo "host_mass_scanok_sda_hcd,prepare mount"
			mdev -s	
			j=0
			k=0
#			a=$(echo '/dev/sda*')
      a=$(ls /dev/sda*)
			echo $a
			for i in $a 			
			do
				
				echo "i is $i"
				if [ "$i" = "$dev_name" ]; then
					echo "this is /dev/sda"
				else
					let	k=$k+1
					echo "k is $k"
					echo "this is sda"
					partnum=${i##*sda}
					echo "partnum0 is $partnum"	
				#	mount $i /mnt/usb$partnum
					mount $i /mnt/usb$k
					j=1				
				fi
			done
			
			if [ $j -eq 0 ]; then	
				mount /dev/sda /mnt/usb1
			fi
			;;
		
			"host_mass_scanok_sdb_hcd")echo "host_mass_scanok_sdb_hcd,prepare mount"
			mdev -s	
			j=0
			k=0
#			a=$(echo '/dev/sdb*')
      a=$(ls /dev/sdb*)
			echo $a
			for i in $a 			
			do
				echo "i is $i"
				if [ "$i" = "$dev_name_b" ]; then
					echo "this is /dev/sdb"
				else
					let	k=$k+1
					echo "k is $k"
					echo "this is sdb"
					partnum=${i##*sdb}
					echo "partnum0 is $partnum"	
				#	mount $i /mnt/usb$partnum
					mount $i /mnt/usb$k
					j=1				
				fi
			done
			
			if [ $j -eq 0 ]; then	
				mount /dev/sdb /mnt/usb1
			fi
			;;
			
			"host_mass_scanok_sdb_hcdnext")echo "host_mass_scanok_sdb_hcdnext,prepare mount"
			mdev -s	
			j=0
			k=0
			prev_num=5	
#			a=$(echo '/dev/sdb*')
      a=$(ls /dev/sdb*)
			echo $a
			for i in $a 			
			do
				echo "i is $i"
				if [ "$i" = "$dev_name_b" ]; then
					echo "this is /dev/sdb"
				else
					let	k=$k+1
					echo "k is $k"
					echo "this is sdb"
					partnum=${i##*sdb}
				#	let partnum=$partnum+$prev_num	
					let partnum=$k+$prev_num
					echo "partnum0 is $partnum"
					mount $i /mnt/usb$partnum
					j=1				
				fi
			done
			if [ $j -eq 0 ]; then	
				mount /dev/sdb /mnt/usb4
			fi
			;;
			
		"host_mass_scanok_sda_hcdnext")echo "host_mass_scanok_sda_hcdnext,prepare mount"
			mdev -s	
			j=0
			k=0
			prev_num=5	
#			a=$(echo '/dev/sda*')
      a=$(ls /dev/sda*)
			echo $a
			for i in $a 			
			do
				echo "i is $i"
				if [ "$i" = "$dev_name" ]; then
					echo "this is /dev/sda"
				else
					let	k=$k+1
					echo "k is $k"
					echo "this is sda"
					partnum=${i##*sda}
				#	let partnum=$partnum+$prev_num	
					let partnum=$k+$prev_num
					echo "partnum0 is $partnum"
					mount $i /mnt/usb$partnum
					j=1				
				fi
			done
			if [ $j -eq 0 ]; then	
				mount /dev/sda /mnt/usb4
			fi
			;;
							
	"host_mass_out_hcdnext")
			umount /mnt/usb6
			umount /mnt/usb7
			umount /mnt/usb8
			umount /mnt/usb9
			umount /mnt/usb10
			echo "host mass out<6~10> &umount over"
			;;
			
		"host_mass_out_hcdnextall")
			umount /mnt/usb6
			umount /mnt/usb7
			umount /mnt/usb8
			umount /mnt/usb9
			umount /mnt/usb10	
			rmmod usb_storage
			echo "host mass out<6~10> usb-storage"
			;;
			
	"host_mass_out_hcd")echo "host mass out<1>"
			umount /mnt/usb1
			umount /mnt/usb2
			umount /mnt/usb3
			umount /mnt/usb4
			umount /mnt/usb5
			echo "host mass out<1~5> &umount over"
			;;
		
	"host_mass_out_hcdall")echo "host mass out all"
			umount /mnt/usb1
			umount /mnt/usb2
			umount /mnt/usb3
			umount /mnt/usb4
			umount /mnt/usb5	
			rmmod usb_storage
			echo "host mass out ok usb_storage"
			;;		
	"host_mass_out")echo "host mass out"
			umount /mnt/usb1
			umount /mnt/usb2
			umount /mnt/usb3
			umount /mnt/usb4
			umount /mnt/usb5
			umount /mnt/usb6
			umount /mnt/usb7
			umount /mnt/usb8
			umount /mnt/usb9
			umount /mnt/usb10	
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
					
	"host_raw_out")	echo "hcd out"
				rmmod am7x_hcd
			;;
			
	"host_raw_out_next")	echo "hcd next out "	
				rmmod am7x_hcd_next
			;;

		"host_raw_out_wifi")	echo " hcd wifi out"
			if [	-r "/sys/module/8192cu" ]; then
				rmmod 8192cu
			elif [ -r "/sys/module/8192du" ]; then
				rmmod 8192du
			elif [ -r "/sys/module/8188eu" ]; then
				rmmod 8188eu
			elif [ -r "/sys/module/8192eu" ]; then
				rmmod 8192eu
			elif [ -r "/sys/module/8821au" ]; then
				rmmod 8821au
			elif [ -r "/sys/module/8188fu" ]; then
				rmmod 8188fu
			elif [ -r "/sys/module/rt5370sta" ]; then
				rmmod rt5370sta
			elif [ -r "/sys/module/rtnet3070ap" ]; then
				modprobe -r rtnet3070ap
			fi
			
			rmmod am7x_hcd
			;;
			
	"host_raw_out_next_wifi")	echo "hcd_next wifi out "
			if [	-r "/sys/module/8192cu" ]; then
				rmmod 8192cu
			elif [ -r "/sys/module/8192du" ]; then
				rmmod 8192du
			elif [ -r "/sys/module/8188eu" ]; then
				rmmod 8188eu
			elif [ -r "/sys/module/8192eu" ]; then
				rmmod 8192eu
			elif [ -r "/sys/module/8821au" ]; then
				rmmod 8821au
			elif [ -r "/sys/module/8188fu" ]; then
				rmmod 8188fu
			elif [ -r "/sys/module/rt5370sta" ]; then
				rmmod rt5370sta
			elif [ -r "/sys/module/rtnet3070ap" ]; then
				modprobe -r rtnet3070ap
			fi
			
			rmmod am7x_hcd_next
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
	"host_iph_in") echo "host_iph_in"
			if [ -f /tmp/eth_usb_stop ];then
				rm /tmp/eth_usb_stop
			fi
			sh /am7x/case/scripts/eth_usb.sh iphone in &
			;;
	"host_iph_out") echo "host_iph_out"
			touch /tmp/eth_usb_stop
			sh /am7x/case/scripts/eth_usb.sh iphone out
			;;
	"host_android_in") echo "host_android_in"
			sh /am7x/case/scripts/eth_usb.sh android in &
			;;
	"host_android_out") echo "host_android_out"
			sh /am7x/case/scripts/eth_usb.sh android out
			;;
	"host_adb_start") echo "host_adb_start"
			if [ -f /tmp/eth_usb_stop ];then
				rm /tmp/eth_usb_stop
			fi
			sh /am7x/case/scripts/eth_usb.sh adb start &
			;;
	"host_adb_stop") echo "host_adb_stop"
			sh /am7x/case/scripts/eth_usb.sh adb stop
			;;
	"mtp_device_in") echo "mtp_device_in"
			insmod /lib/modules/2.6.27.29/fuse.ko
			mkdir /mnt/mtpfs
			umount /mnt/mtpfs
			chmod a+x /bin/mtpfs
			mtpfs /mnt/mtpfs
			
			;;
		"mtp_device_out") echo "mtp_device_out"
			killall mtpfs
			rm -rf /mnt/mtpfs/
			rmmod fuse
			;;		
		*)echo  "unknow message type"
			;;
esac
