#!/bin/sh -e

umount /mnt/udisk
if [ $? -ne 0 ]; then
	echo "Udisk is Busy!"
else
	 mkfs.vfat /dev/partitions/udisk
	 if [ $? -ne 0 ]; then
	 		echo "Format udisk Error!"
	 else
	 		mount dev/partitions/udisk /mnt/udisk
	 fi 
fi