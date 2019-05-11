#!/bin/sh

# install the new top command to rootfs

cd ../../../rootfs/usr/share/
if [ ! -e newtop ]
then
	mkdir newtop; 
	#echo "newtop not exist,create";
fi

cd ../../../tools/linux_utils/top/
cp -dprf ./top ../../../rootfs/usr/share/newtop/
cp -dprf ./terminfo ../../../rootfs/usr/share/newtop/
cp -dprf ./newtop.sh ../../../rootfs/usr/share/newtop/
cp -dprf ./libproc-3.2.7.so ../../../rootfs/lib/
echo "newtop install ok"

