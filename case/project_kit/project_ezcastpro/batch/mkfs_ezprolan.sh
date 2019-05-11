#/bin/bash
#
# Copyright (C) 2009 Actions MicroEletronics Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# Create filesystem images described by a configuration file, default
# to be AM_FS.cfg.
#
# Changelog:
#
#  2009/11/10: Pan Ruochen <ijkxyz@msn.com>
#				- The first available version
#  2009/11/18: Pan Ruochen <ijkxyz@msn.com>
#				- Support for creating NTFS filesystem image.
#  2010/5/17:  Pan Ruochen <ijkxyz@msn.com>
#				- Support for creating module dependency files for modprobe.
#
stty echo
input_arg=$1
set -e
set -o pipefail
#array=([0]=8188eu [1]=8192cu [2]=8192du [3]=rt3070)
#echo "USEAGE: sh mkfs option"
#echo "option is dongle name,select from ${array[*]}"
make_dev()
{
	if [ ! -f $1 -o ! -d $2 ]; then
		return 0
	fi

	while read line
	do
		if [ -n "$line" ] ; then
			sudo mknod $2/dev/$line
		fi
	done < $1
#	ls -l $2/dev
}

resize_fs_nogood()
{
	real_size=$(wc -c $fs_image | awk '{print $1}');
	echo "real size: $real_size";
	cp -f $fs_image $fs_image.1;
	e2fsck -f $fs_image;
	resize2fs $fs_image $fsize;
	resize2fs $fs_image.1 $((fsize/2));
	dd of=$fs_image.1 if=$fs_image bs=$real_size count=1;
	mv -f $fs_image.1 $fs_image;
}

resize_fs()
{
	local fs_image fsize
	fs_image=$1; fsize=$2

	echo "** resize $fs_image to $fsize"
	set +e
	e2fsck -f -p $fs_image ; resize2fs $fs_image $fsize
	set -e
}

clean_dev_dir()
{
	local ent

	for ent in $1/*
	do
		if [ -b $ent -o -c $ent -o -p $ent ]; then
			rm -f $ent
		fi
	done
}

remove_file()
{
	local file path
	for file in `ls $1`
	do
		if [ -d $1"/"$file ];then
			remove_file $1"/"$file;
		elif [ -f $1"/"$file ];then
			path=$1"/"$file;
			echo $path | grep -i "libnss_" | xargs rm -rf;
			echo $path | grep -i "libxt_" | xargs rm -rf;
		fi
	done
}

try_umount()
{
	set +e
	sudo umount $1 >/dev/null 2>&1
	set -e
}

delete_file()
{
        path=$1
        if [  -a $path ];then
                echo "###### Delete "$path
                rm -rf $path
        fi
}

delete_local_function_swf()
{
        echo "#####################################################"
        echo "#            Delete local function swf              #"
        echo "#####################################################"
	rfs_tmp=$TOPDIR/scripts/rfstmp
        swf_path=$rfs_tmp/am7x/case/data
        delete_file $swf_path/osdicon
        delete_file $swf_path/audio.swf
        
 $swf_path/ezhelp.swf
        delete_file $swf_path/ezmedia.swf
        delete_file $swf_path/listd.swf
        delete_file $swf_path/mainmenu.swf
        delete_file $swf_path/officeOSD.swf
        delete_file $swf_path/office.swf
        delete_file $swf_path/photo.swf
        delete_file $swf_path/set_audio.swf
        delete_file $swf_path/set_photo.swf
        delete_file $swf_path/set_sys.swf
        delete_file $swf_path/set_video.swf
        delete_file $swf_path/videoOSD.swf
        delete_file $swf_path/videoSubtitle.swf
        delete_file $swf_path/video.swf
}

customer_rootfs_rm()
{
        echo "#####################################################"
        echo "#                   Product patch                   #"
        echo "#####################################################"
	scripts_path=$TOPDIR/scripts
	rfs_tmp=$TOPDIR/scripts/rfstmp
	MC_FILE=$scripts_path/mconfig.h
	ezcast_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZCAST_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZCAST_ENABLE | awk '{print $3}' || echo 0`
	ezcastpro_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZCASTPRO_MODE && cat mconfig.h | grep MODULE_CONFIG_EZCASTPRO_MODE | awk '{print $3}' || echo 0`
	ezmusic_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZMUSIC_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZMUSIC_ENABLE | awk '{print $3}' || echo 0`
	ezwilan_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZWILAN_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZWILAN_ENABLE | awk '{print $3}' || echo 0`
	ezcast5g_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZCAST5G_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZCAST5G_ENABLE | awk '{print $3}' || echo 0`
	echo "rfs_tmp: "$rfs_tmp
	echo "MC_FILE: "$MC_FILE
	echo "ezcast_flag: "$ezcast_flag
	echo "ezcastpro_flag: "$ezcastpro_flag
	echo "ezmusic_flag: "$ezmusic_flag
	echo "ezwilan_flag: "$ezwilan_flag

	if [ $ezwilan_flag -eq 0 -a $ezmusic_flag -eq 0 -a $ezcastpro_flag -eq 0 ]; then
		echo ">>>>>>> Remove key driver!!!"
		find $rfs_tmp -name 'keymap' -depth| xargs rm -rf;
		find $rfs_tmp -name '*key*' -depth| xargs rm -rf;
	fi
	if [ $ezcastpro_flag -eq 0 ]; then
		delete_local_function_swf
	fi
	if [ $ezcastpro_flag -eq 8251 ]; then
		Patch5G_Path=$TOPDIR/Flash/EZCast/Patch/EZCast5G
		cp -dprf $Patch5G_Path/hostapd $rfs_tmp/sbin
		echo "$Patch5G_Path/hostapd >>>>> $rfs_tmp/sbin"
		cp -dprf $Patch5G_Path/hostapd_cli $rfs_tmp/sbin
		echo "$Patch5G_Path/hostapd_cli >>>>>> $rfs_tmp/sbin"
	fi
	if [ $ezcast5g_flag -eq 0 ]; then
		find $rfs_tmp -name '8821au.ko' -depth| xargs rm -rf;
	else
		Patch5G_Path=$TOPDIR/Flash/EZCast/Patch/EZCast5G
		cp -dprf $Patch5G_Path/hostapd $rfs_tmp/sbin
		echo "$Patch5G_Path/hostapd >>>>> $rfs_tmp/sbin"
		cp -dprf $Patch5G_Path/hostapd_cli $rfs_tmp/sbin
		echo "$Patch5G_Path/hostapd_cli >>>>>> $rfs_tmp/sbin"
		cp -dprf $Patch5G_Path/p2p_hostapd_5G_01.conf $rfs_tmp/etc
		echo "$Patch5G_Path/p2p_hostapd_5G_01.conf >>>>>> $rfs_tmp/etc"
		cp -dprf $Patch5G_Path/p2p_hostapd_5G.conf $rfs_tmp/etc
		echo "$Patch5G_Path/p2p_hostapd_5G.conf >>>>>> $rfs_tmp/etc"
		cp -dprf $Patch5G_Path/dnsmasq_dhcp.conf $rfs_tmp/etc
		echo "$Patch5G_Path/dnsmasq_dhcp.conf >>>>>> $rfs_tmp/etc"
	fi
}

make_fs()
{
	local fs_image fs_source vfs fsize idx_file
	local cp_flags init_size real_size
	local mkfs_flags
	local tmpdir
	local rfs_tmp

	fs_image=$1
	fs_source=$2
	fsize=$3
	vfs=$4
    

	echo ">>>>>>>>>>>>>>> $fs_image $fs_source $vfs $fsize"
    
	if [ "$fs_image" = "rootfs.img" ];
	then
		## remove the .svn directory for rootfs to reduce the image size.
		rfs_tmp="rfstmp"
		ezcast_dir="../case/images/ezcast/"
		rm -rf $rfs_tmp;
		mkdir $rfs_tmp;
		cp -dprf $fs_source/* $rfs_tmp;
		if  [ -n "$input_arg" ];then
			if [ "$input_arg" != "realtek" ];then
				if [ "$input_arg" != "ezcastpro" ];then
					find $rfs_tmp -name '8192cu.ko' | xargs rm -rf;
					find $rfs_tmp -name '8192du.ko' | xargs rm -rf;
					find $rfs_tmp -name '8188eu.ko' | xargs rm -rf;
				fi
			fi
			if [ "$input_arg" != "ralink" ];then
				find $rfs_tmp -name 'rt5370sta.ko' | xargs rm -rf;
				find $rfs_tmp -name 'rt*3070*.ko' | xargs rm -rf;
			fi
			patch_path=$TOPDIR/Flash/EZCast/Patch/dnsmasq
			#if [ -f $patch_path/patch ];then
				echo "EZCast patch dnsmasq!!!"
				cp $patch_path/route_exit.sh $rfs_tmp/am7x/case/scripts
				cp $patch_path/dns_forward.sh $rfs_tmp/am7x/case/scripts
				cp $patch_path/dnsmasq $rfs_tmp/sbin
				chmod a+x $rfs_tmp/sbin/dnsmasq
				rm -rf $rfs_tmp/sbin/named
				cp $patch_path/dnsmasq.conf $rfs_tmp/etc
				cp $patch_path/default.script $rfs_tmp/usr/share/udhcpc
			#fi
			if [ "$input_arg" == "ezcastpro" ];then
				set +e
				grep "MODULE_CONFIG_EZCASTPRO_MODE" ./mconfig.h | grep 8251
				if [ $? == 0 ];then
					echo "ezcastpro 8251 dongle, take off media stuff..........."
					find $rfs_tmp -name 'video*.swf' | xargs rm -rf;
					find $rfs_tmp -name 'audio.swf' | xargs rm -rf;
					find $rfs_tmp -name 'ezmedia.swf' | xargs rm -rf;
					find $rfs_tmp -name 'office.swf' | xargs rm -rf;
					find $rfs_tmp -name 'photo.swf' | xargs rm -rf;
					find $rfs_tmp -name 'set_*.swf' | xargs rm -rf;
					find $rfs_tmp -name 'osdicon' | xargs rm -rf;
					find $rfs_tmp -name '8192cu.ko' | xargs rm -rf;
					find $rfs_tmp -name '8192du.ko' | xargs rm -rf;
					find $rfs_tmp -name '8188eu.ko' | xargs rm -rf;
				fi
			set -e
			fi			
			cp $rfs_tmp/etc/version.conf $ezcast_dir
			find $rfs_tmp -name '.svn' | xargs rm -rf;
			find $rfs_tmp -name 'libnsl*' -depth| xargs rm -rf ;
			#find $rfs_tmp -name 'libnss*' -depth| xargs rm -rf ;libstream.so
			find $rfs_tmp -name 'am7x_tp.ko*' -depth| xargs rm -rf ;
			#find $rfs_tmp -name 'libxt*' -depth| xargs rm -rf;
			find $rfs_tmp -name 'rtc*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name 'dbus*' -depth| xargs rm -rf;
			find $rfs_tmp -name 'lib_id_tif.so' -depth| xargs rm -rf;
			find $rfs_tmp -name 'lib_id_gif.so' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'libresolv*' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'xtables*' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'config*' -depth| xargs rm -rf;
			find $rfs_tmp -name 'ca-certificates.crt' -depth| xargs rm -rf;
			find $rfs_tmp -name 'hostapd' |grep -e "local"| xargs rm -rf;
			find $rfs_tmp -name '*spi*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name '*hid.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name '*mouse*.ko' -depth| xargs rm -rf;
			#find $rfs_tmp -name '*storage*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name 'download' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'libxml2*' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'libstdc++*' -depth| xargs rm -rf;
			#find $rfs_tmp -name '*curl*' -depth| xargs rm -rf;
			find $rfs_tmp -name '*libsqlite*' -depth| xargs rm -rf;
			find $rfs_tmp -name 'libhyfviewer.so' -depth| xargs rm -rf;
			find $rfs_tmp -name '*mtp*' -depth| xargs rm -rf;
			find $rfs_tmp -name 'libnis*' -depth| xargs rm -rf;
			find $rfs_tmp -name 'html' -depth| xargs rm -rf;
			find $rfs_tmp -name '*bluetooth*' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'case/*.bin' -depth| xargs rm -rf;
			find $rfs_tmp -name '*scsi*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name '*ads*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name '*dpp*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name 'am7x_battery.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name '*al*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name '*axp*.ko' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'am7x_upgrade.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name '*tle*.ko' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'libc-2.8.so' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'libiconv*' -depth| xargs rm -rf;
			find $rfs_tmp -name 'libftp_client.so' -depth| xargs rm -rf;
			find $rfs_tmp -name 'rfcomm.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name 'l2cap.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name 'cfg' -depth| xargs rm -rf;
			find $rfs_tmp -name 'lsof' -depth| xargs rm -rf;
			#find $rfs_tmp -name '*libm*' -depth| xargs rm -rf; can not remove
			#find $rfs_tmp -name 'ld*' -depth| xargs rm -rf;    can not remove
			#find $rfs_tmp -name 'librt*' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'libcrypt*' -depth| xargs rm -rf;
			find $rfs_tmp -name 'libglib*' -depth| xargs rm -rf;
			customer_rootfs_rm
			chmod a+x $rfs_tmp/am7x/case/scripts/*.sh
			chmod a+x $rfs_tmp/sbin/*
			init_size=$(du $rfs_tmp | awk '$1{total=$1}END{print (total+2560)*1024}')
			if [ "$input_arg" != "ezcastpro" ];then
				cp $ezcast_dir/udhcpd_01.conf $rfs_tmp/etc/ -dprf
				cp $ezcast_dir/udhcpd.conf $rfs_tmp/etc/ -dprf				
				find $rfs_tmp -name '*key*' -depth| xargs rm -rf;
				find $rfs_tmp -name '*net*' -depth| xargs rm -rf;
				find $rfs_tmp -name '*at2*.ko' -depth| xargs rm -rf;
				find $rfs_tmp -name 'keymap' -depth| xargs rm -rf;
				find $rfs_tmp -name 'libfv.so' -depth| xargs rm -rf;
				find $rfs_tmp -name 'am7x_*card*.ko' -depth| xargs rm -rf ;				
				find $rfs_tmp -name '*fuse*' -depth| xargs rm -rf;				
			else
				cp  ../case/lib/libamsocket.so  $rfs_tmp/usr/lib/
				find $rfs_tmp -name 'libfv.so' -depth| xargs rm -rf;
				find $rfs_tmp -name 'am7x_*card*.ko' -depth| xargs rm -rf ;			
				cp $ezcast_dir/udhcpd_ezcastpro.conf $rfs_tmp/etc/udhcpd_01.conf -f			
				cp $ezcast_dir/udhcpd_ezcastpro.conf $rfs_tmp/etc/udhcpd.conf -f
				cp $ezcast_dir/udhcpd_ezcastpro_nodns_01.conf $rfs_tmp/etc/udhcpd_nodns_01.conf -dprf				
#				find $rfs_tmp -name 'g_file*.ko' -depth| xargs rm -rf;
#				cp -f $SDK_DIR/cdrom/*.iso $rfs_tmp/mnt/cdrom/
#				init_size=$(($init_size+5242880+8*1024*1024));
				echo "mkfs ezcastpro LAN done"
			fi
		else
			echo "make regular rootfs..."
			init_size=$(du $rfs_tmp | awk '$1{total=$1}END{print (total+5120)*1024}')
		fi
		#remove_file  $rfs_tmp;
		#init_size=$(du $rfs_tmp | awk '$1{total=$1}END{print (total+5120)*1024}')
		echo ">>>>>>>>>>>>>>>rootffs init_size = $init_size"
	else
		if [ "$fs_image" = "user1.img" ];
		then
			rfs_tmp="rfstmp"
		        rm -rf $rfs_tmp;
		        mkdir $rfs_tmp;
		        cp -dprf $fs_source/* $rfs_tmp;
			mv $rfs_tmp/thttpd/html/index_ezcast.html $rfs_tmp/thttpd/html/index.html				
			find $rfs_tmp -name '.svn' | xargs rm -rf;
			if  [ -n "$input_arg" ];then
				find $rfs_tmp -name 'download' -depth| xargs rm -rf;
				find $rfs_tmp -name 'wqy-microhei.ttc' -depth| xargs rm -rf;
				find $rfs_tmp -name 'background*' -depth| xargs rm -rf;
				cp $ezcast_dir/udhcpd_nodns_01.conf $rfs_tmp/softap/ -dprf
				if [ "$input_arg" != "ezcastpro" ];then
					cp $ezcast_dir/udhcpd_01.conf $rfs_tmp/softap/ -dprf
				else
					echo "use ezcastpro udhcpd_01.conf"
					cp $ezcast_dir/udhcpd_ezcastpro.conf $rfs_tmp/softap/ -dprf
				fi
			fi
			find $rfs_tmp/thttpd/html/ -name '*.html' | xargs chmod 644
                        find $rfs_tmp/thttpd/html/ -name '*.js' | xargs chmod 644
                        find $rfs_tmp/thttpd/html/ -name '*.png' | xargs chmod 644
                        find $rfs_tmp/thttpd/html/ -name '*.jpg' | xargs chmod 644
                        find $rfs_tmp/thttpd/html/ -name '*.css' | xargs chmod 644
                        find $rfs_tmp/thttpd/html/ -name '*.gif' | xargs chmod 644
			init_size=$(du $rfs_tmp | awk '$1{total=$1}END{print (total+1024)*1024}')
			echo ">>>>>>>>>>>>>>>$fs_image init_size = $init_size"
		else
			init_size=$(du $fs_source | awk '$1{total=$1}END{print (total+1024)*1024}') 
			echo ">>>>>>>>>>>>>>>$fs_image init_size = $init_size"
		fi
	fi
	
	
	case $vfs in
	ext2|ext3)
		cp_flags='-dprf';
		try_umount $MNT_DIR;
		dd if=/dev/zero of=$fs_image bs=$init_size count=1;
		mkfs.$vfs -F -m0 $fs_image;
		sudo mount -t $vfs -o loop $fs_image $MNT_DIR;
		;;
	vfat|ntfs)
		cp_flags='-rf';
		dd if=/dev/zero of=$fs_image bs=$fsize count=1;
		eval mkfs_flags='$'mkfs_flags_$vfs;
		mkfs.$vfs $mkfs_flags $fs_image;
		try_umount $MNT_DIR;
		sudo mount -t $vfs -o loop -o user,rw,auto,umask=0000,uid=$uid,gid=$gid,iocharset=utf8 \
		$fs_image $MNT_DIR;;
	cpio|romfs) ;;
	*)
		echo "Unsupported FS $vfs"; return 0;;
	esac

	case $vfs in
	ext2|ext3|vfat|ntfs)
		if which rsync >/dev/null 2>&1;  then
			if [ "$fs_image" = "rootfs.img" ]; then
				cp -dprf $rfs_tmp/* $MNT_DIR;
			elif [ "$fs_image" = "user1.img" ]; then
				cp -dprf $rfs_tmp/* $MNT_DIR;
			else
				rsync -r -l --exclude '*.svn*' $fs_source/ $MNT_DIR/;
			fi
		else
		    if [ "$fs_image" = "rootfs.img" ]; then
		        cp -dprf $rfs_tmp/* $MNT_DIR;
		    elif [ "$fs_image" = "user1.img" ]; then
            		cp -dprf $rfs_tmp/* $MNT_DIR;
		    else
			tmpdir=__tmpdir_nosvn;
			rm -rf $tmpdir;
			mkdir $tmpdir;
			cp $cp_flags $fs_source/* $tmpdir;
			find $tmpdir -name '.svn' -type d | xargs rm -rf;
			mv -f $tmpdir/* $MNT_DIR;
			rm -rf $tmpdir;
		    fi
		fi;
		if [ $vfs = ext2 -o $vfs = ext3 ]; then
			make_dev $fs_source/etc/mkdev.conf $MNT_DIR;
		fi ;
		
		#**************************************************
		# remove unused modules according to module config.
		#**************************************************
		if [ "$fs_image" = "rootfs.img" ];
		then
			do_module_process $TOPDIR/scripts/mconfig.h 0 ${MNT_DIR}
		fi
		
		sudo umount $MNT_DIR;;
	cpio)	
		make_dev $fs_source/etc/mkdev.conf $fs_source;
		(cd $fs_source && find . ! -regex '.*[.]svn.*' | cpio -o -H newc | gzip -f -9) >$fs_image ;
		clean_dev_dir $fs_source/dev ;;
	romfs)
		make_dev $fs_source/etc/mkdev.conf $fs_source;
		$SDK_DIR/tools/genromfs -d $fs_source -x '.svn' -f $fs_image ;
		clean_dev_dir $fs_source/dev ;;
	esac
		
	case $vfs in
	ext2|ext3)
		fsck.$vfs -p $fs_image;
#resize rootfs to 5MB bigger
		local resize_size
		init_size=`expr $init_size / 1024 / 1024`;
		#echo ">>>>>>>>>>>>>>real size:$init_size"
		fsize=${fsize%M}
		echo "fsize: $fsize"
		if [ $init_size -gt `expr $fsize + 1` ];
			then
				resize_size=$init_size
			else
				resize_size=$fsize
		fi
		if  [ -n "$input_arg" ];then
			resize_size=`expr $resize_size`M
		else
			resize_size=`expr $resize_size + 2`M
		fi
		
		if [	"$fs_image" = "initrd.dat"	];
		then
			resize_size=`expr $fsize`M
		fi
		resize_fs $fs_image $resize_size;
		#echo ">>>>>>>>>>>>>>resize_size:$resize_size"
#		idx_file=$(echo "$fs_image"|sed 's:\([.].\+\)$:.idx:');
#		./cr_e2fsidx $fs_image >$idx_file;;
	esac
	mv -f $fs_image $FS_DIR
#	if [ -f "$idx_file" ]; then mv -f $idx_file $FS_DIR; fi
	echo "create $fs_image from $fs_source successfully"
}

do_copy()
{
	local src dst

	src="$TOPDIR/$1"
	eval dst="$TOPDIR/$2"
	if [ ! -d $dst ] ; then
		return
	fi
	if [ -x $src/etc/do_clean.sh ]; then
		$src/etc/do_clean.sh
	fi
	if ls $src >/dev/null 2>&1; then 
		echo "$src  -->  $dst"
		eval cp -dpfr $src $dst
	fi
}

#
# Install system modules to root filesystem and init ramdisk.
#
do_modules_install()
{
	local line rfs_dir install_mod_path install_mod_dir

	rfs_dir=$SDK_DIR/rootfs
	install_mod_path=$rfs_dir/lib/modules
	install_mod_dir=$install_mod_path/$KERNELVERSION
	if [ ! -d $rfs_dir ]; then
		echo "No such directory: $rfs_dir" >&2
		exit 1
	fi
	if [ ! -d $install_mod_dir ]; then
		mkdir -p $install_mod_dir
	fi
	echo $install_mod_dir


	awk -F'#' '$1!=""{print $1}' modules_install.cfg | while read line
	do
		do_copy $line
	done

	if [ -f $LINUX_DIR/System.map ] ; then
		depmod -a -e -F $LINUX_DIR/System.map -b $rfs_dir -r $KERNELVERSION
	fi
}

ezcast_file()
{
	echo ">>>>>>>>>>>>>>>>>> ezcast file copy"
	rootfs_dir=$SDK_DIR/rootfs
	data_dir=$rootfs_dir/am7x/case/data
	ezcast_dir=$TOPDIR/case/images/ezcast
	
	rm -rf $data_dir/*.swf
	echo "Delete "$data_dir"/*.swf"
	rm -rf $data_dir/bmp
	echo "Delete "$data_dir"/bmp"
	rm -rf $data_dir/fui.XLS
	echo "Delete "$data_dir"/fui.XLS"
	eval cp -dpfr $ezcast_dir/data/* $data_dir
	echo $ezcast_dir"/data/*  -->  "$data_dir
	cripts_path=$TOPDIR/scripts
	rfs_tmp=$TOPDIR/scripts/rfstmp
	MC_FILE=$scripts_path/mconfig.h
	ezcastprobox_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZCASTPRO_MODE && cat mconfig.h | grep MODULE_CONFIG_EZCASTPRO_MODE | awk '{print $3}' || echo 0`
	
	if [ $ezcastpro_flag -eq 8075 ]; then
		echo ">>>>>>ezcastprobox_flag:"$ezcastprobox_flag "8189.ko"
	else
		rm -rf $TOPDIR/sdk/library/net/rtl8189ES_linux_v4.3.10.1_13373.20150129/8189es.ko
	fi
}
source ./get_git_version.sh
ezcastpro_version()
{
	ver_file=./version.conf
	gitver=$(get_version_from_git);
	subver=$(awk '/FIRMWARE/{print $3}' $ver_file)
	if [ $subver -lt 10 ];then
		gitver=$gitver"00"$subver
	elif [ $subver -lt 100 ];then
		gitver=$gitver"0"$subver
	elif [ $subver -gt 100000 ];then
		gitver=$subver		
	else
		gitver=$gitver$subver		
	fi
	echo "$gitver,  $subver,  $ver_file"
#	awk '/FIRMWARE/{sub(0,$gitver,$3)}' $ver_file
	gitver="FIRMWARE = "$gitver
	echo "$gitver,  $subver,  $ver_file"	
	sed -i -e "s/FIRMWARE =.*/$gitver/" $ver_file
	cat $ver_file
	cp -f $ver_file ../sdk/rootfs/etc/version.conf	
}
ezcastpro_softap_add_user()
{
	target="../sdk/user1/softap/rtl_hostapd_01.conf"
	sed -i -e 's/max_num_sta=.*/max_num_sta=16/' $target
}
#
#
#
source ./\$am_set_env.sh
#echo $INSTALL_MOD_PATH - $ROOTFS_DIR; exit

if [ "$input_arg" = "realtek" -o "$input_arg" = "ralink" -o "$input_arg" = "ezcastpro" ];then
	CFG_FILE=AM_FS_EZCAST.cfg
	echo "Config to EZCast!!!"
else
	CFG_FILE=AM_FS.cfg
fi
if [ "$input_arg" = "ezcastpro" ];then
	echo "CGI for EZCastPro start!!!"
	cd ../case/apps/cgi-bin/;make clean;make release;cd -
	pwd
	echo "CGI for EZCastPro done!!!"
    # Mos: LSDK release will remove entire library folder, that will cause LSDK version mkfs.sh execute failed.
    # If the driver need to rebuild every time, it should be move into linux driver folder
    if [ -f ../sdk/library/net/rtl8192EU_linux_v4.3.8_12406.20140929.tar.gz ];then
	    echo "8192eu driver for EZCastPro!!!"
	    sh build_RealtekDriver.sh 8192eu
	    pwd
#	    cd $TOPDIR/scripts
	    echo "8192eu driver for EZCastPro done!!!"		
    fi
	echo "eeprom for EZPro LAN start!"
	if [ -f ../sdk/bootloader/eeprom/scripts/am_sdk_config.sh ];then
		cd ../sdk/bootloader/eeprom/scripts/;make clean;sh am_sdk_config.sh AM_8251;make;cd -
		pwd
	fi
	echo "eeprom for EZProjector done!"
	ezcastpro_version
	ezcastpro_softap_add_user
fi
ARGV=($(getopt -l file: -o 'f:' -- "$@")) || exit 1
for((i=0;i<=${#ARGV[@]};i++)) {
	eval opt=${ARGV[$i]}
	case $opt in
	--file|-f)
		((i++));
		eval CFG_FILE=${ARGV[$i]};;
	esac
}

WORK_DIR=$(cd $(dirname $0) && pwd)
if [ -z "$WORK_DIR" ]; then
	echo "Bad dir" >&2
	exit 1
fi
TOPDIR=$(cd $WORK_DIR/.. && pwd)
SDK_DIR=$TOPDIR/sdk
MNT_DIR=$WORK_DIR/tmpmnt
FS_DIR=$SDK_DIR/tools/filesystems
LINUX_DIR=$TOPDIR/$GNU_LINUX_DIR
if [ -d $LINUX_DIR ]; then
KERNELVERSION=$($AM_MAKE -C $LINUX_DIR kernelversion)
else
KERNELVERSION=2.6.27.29
fi

uid=$(id -u)
gid=$(id -g)
mkfs_flags_ntfs='-F'
alias cp=$(which cp)
alias mv=$(which mv)

do_modules_install

if [ "$input_arg" = "realtek" -o "$input_arg" = "ralink" -o "$input_arg" = "ezcastpro" ];then
	ezcast_file
fi

echo ">>>>>>>> CFG_FILE: "$CFG_FILE
awk -F'#' '$1{print $1}' $CFG_FILE | while read args
do
	echo ">>>>>>> args: "$args
	make_fs $args
done


