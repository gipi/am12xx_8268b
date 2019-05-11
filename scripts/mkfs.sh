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

strip_file()
{
	if [ $# -lt 1 ]; then
		echo "<strip_file>[$#] Must have one or more file to strip."
		return 0
	fi
    path=$1
	if [  -a $path ];then
    	echo "###### Strip "$@
		mips-linux-gnu-strip $@ || echo "===== Strip fail!!!"
	fi
}

delete_file()
{
        path=$1
        if [  -a $path ];then
                echo "###### Delete "$@
                rm -rf $@
        fi
}

copy_file()
{
	if [ $# -lt 2 ]; then
		echo "<copy_file> args error!!!"
		return 0
	fi
	
	if [ -e $1 ]; then
		eval cp -dprf $@
		if [ $# -eq 2 ]; then
			echo "$1 >>>>>> $2"
		else
			echo "copy: $*"
		fi
	fi

}

function change_mode()
{
	if [ $# -ne 3 ];then
		return 1;
	fi

	local rpath="$1";
	local fname="$2";
	local mode="$3";

	find $rpath -name "$fname" | while read -r line
	do
		if [ "$line"A != ""A ];then
			chmod $mode "$line";
		fi
	done
}

apptheme_pack()
{
    echo ">>>>>>>>>>>>>>>>>> apptheme_pack"
    
    rfs_tmp=$TOPDIR/scripts/rfstmp
    apptheme_dir=$TOPDIR/case/apptheme
    apptheme_output_dir=$TOPDIR/case/apptheme/apptheme_pack
    apptheme_dest_dir=$rfs_tmp/thttpd/html/apptheme

    if [ -d ${apptheme_dest_dir} ]; then
        delete_file ${apptheme_dest_dir}
    fi
    
    cd ${apptheme_dir}
		./do_apptheme_pack.sh
	cd -
    
    copy_file  ${apptheme_output_dir} ${apptheme_dest_dir}
    
    echo ">>>>>>>>>>>>>>>>>> apptheme_pack done"
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
        delete_file $swf_path/ezhelp.swf
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

snor_16m_rm()
{
	echo "#####################################################"
	echo "#              16M snor rootfs handle               #"
	echo "#####################################################"
	rfs_tmp=$TOPDIR/scripts/rfstmp
	Patch=$TOPDIR/Flash/EZCast/Patch
	ez3to1_dir=$TOPDIR/Flash/MiraScreen_Free/SWF

	copy_file $SDK_DIR/initrd/etc/mkdev.conf $rfs_tmp/etc/
	if [ $ezwire_flag -eq 0 ];then
		copy_file $Patch/snor/rcS.ezcast $rfs_tmp/etc/init.d/rcS
	else
		copy_file $Patch/snor/rcS $rfs_tmp/etc/init.d/
	fi
	copy_file $TOPDIR/Flash/EZCast/Patch/snor/msyh.ttf $rfs_tmp/am7x/case/data/

	find $rfs_tmp -name 'libfuse*' | xargs rm -rf;
	find $rfs_tmp -name 'am7x_spdif.ko' | xargs rm -rf;
	find $rfs_tmp -name 'ambl.ko' | xargs rm -rf;
	find $rfs_tmp -name 'am7x_btvd.ko' | xargs rm -rf;
	find $rfs_tmp -name 'am7x_uartchar.ko' | xargs rm -rf;
	find $rfs_tmp -name 'linein.app' | xargs rm -rf;
	find $rfs_tmp -name 'libnetsnmpsrc.so' | xargs rm -rf;
#	find $rfs_tmp -name 'ota_from' | xargs rm -rf;
	find $rfs_tmp -name 'hci_usb.ko' | xargs rm -rf;
	find $rfs_tmp -name 'am7x_net.ko' | xargs rm -rf;
	find $rfs_tmp -name 'amfb.ko' | xargs rm -rf;
	find $rfs_tmp -name 'libip6tc.so*' | xargs rm -rf;
	find $rfs_tmp -name 'am7x_sdcard.ko' | xargs rm -rf;
	find $rfs_tmp -name 'mmc_core.ko' | xargs rm -rf;
	find $rfs_tmp -name 'libortp.so*' | xargs rm -rf;
	find $rfs_tmp -name 'lib_vp8.so' | xargs rm -rf;

	delete_file $rfs_tmp/lib/firmware

#	delete_file $rfs_tmp/bin/curl
	delete_file $rfs_tmp/bin/lrz

	delete_file $rfs_tmp/sbin/iwevent
	delete_file $rfs_tmp/sbin/iwpriv
	delete_file $rfs_tmp/sbin/wpa_passphrase
	delete_file $rfs_tmp/sbin/iperf
	delete_file $rfs_tmp/sbin/iwgetid
	delete_file $rfs_tmp/sbin/iwspy
#	delete_file $rfs_tmp/sbin/thttpd

	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_cfcard.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_eMMCcard.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_it6681.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_mscard.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_xdcard.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/at24_i2c.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/at25.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/cifs.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/g_file_debug.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/g_file_storage.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/g_subdisplay.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/i2c_wm8988.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/i2s_i2c.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/keenhi_rfkey.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_udc_next.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_carddet.ko
#	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_uoc.ko
       delete_file $rfs_tmp/am7x/lib/libusb-0.1.so.4
       if [ $ezwire_type -eq 10 ]; then
		delete_file $rfs_tmp/etc/hostapd.conf
		if [ $usb1_disable -ne 1 ]; then
			delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_hcd.ko
			delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_uoc.ko
		fi
       fi
	if [ $ezwire_type -ne 7 ]; then
		delete_file $rfs_tmp/lib/modules/2.6.27.29/g_ether.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_udc.ko
	else
		delete_file $rfs_tmp/usr/sbin/usbmuxd
		delete_file $rfs_tmp/usr/sbin/idevice*
	fi

	delete_file $rfs_tmp/usr/bin/adbForward.app

	delete_file $rfs_tmp/usr/sbin/snmp_am

	delete_file $rfs_tmp/am7x/bin/rs232_main.app

	delete_file $rfs_tmp/am7x/case/data/AdbService.apk
	if [ $chip_major -ne 3 -o $config_3to1 -ne 0 ];then
		delete_file $rfs_tmp/am7x/case/data/fui.res
		if [ $chip_major -eq 3 ];then
			copy_file $ez3to1_dir/fui.res $rfs_tmp/am7x/case/data/
		fi
	fi
	delete_file $rfs_tmp/am7x/case/data/connectPC.swd
	delete_file $rfs_tmp/am7x/case/data/slitemplate

#delete_file $rfs_tmp/am7x/case/data/test.swf

#	delete_file $rfs_tmp/am7x/lib/lib_asf.so
#	delete_file $rfs_tmp/am7x/lib/lib_avi.so
#	delete_file $rfs_tmp/am7x/lib/lib_flv.so
#	delete_file $rfs_tmp/am7x/lib/lib_mkv.so
#	delete_file $rfs_tmp/am7x/lib/lib_mjpg.so
#	delete_file $rfs_tmp/am7x/lib/lib_mov.so
#	delete_file $rfs_tmp/am7x/lib/lib_mpeg2.so
#	delete_file $rfs_tmp/am7x/lib/lib_mpeg4.so
#	delete_file $rfs_tmp/am7x/lib/lib_rm.so
#	delete_file $rfs_tmp/am7x/lib/lib_rv.so
#	delete_file $rfs_tmp/am7x/lib/lib_ts.so
#	delete_file $rfs_tmp/am7x/lib/lib_vc1.so
#	delete_file $rfs_tmp/am7x/lib/lib_vp8.so

#	delete_file $rfs_tmp/am7x/lib/libflac.so
#	delete_file $rfs_tmp/am7x/lib/libdts.so
#	delete_file $rfs_tmp/am7x/lib/libamr.so
#	delete_file $rfs_tmp/am7x/lib/libape.so

#	mkdir $rfs_tmp/am7x/ezdata
#	copy_file $SDK_DIR/user1/ezdata/ezcast_icon* $rfs_tmp/am7x/ezdata/
#	copy_file $SDK_DIR/user1/ezdata/ap_link_1.png $rfs_tmp/am7x/ezdata/

#	delete_file $rfs_tmp/am7x/lib/libdlna*
#	delete_file $rfs_tmp/am7x/lib/libiconv.so*

#delete_file $rfs_tmp/am7x/lib/libxml2.so*
#delete_file $rfs_tmp/am7x/lib/wpa_cli.so
#delete_file $rfs_tmp/am7x/lib/libfreetype.so*

	echo "===== To copy user1 ====="
	cp -dprf $TOPDIR/sdk/user1/thttpd $rfs_tmp/usr/share/
#	cp -dprf $TOPDIR/sdk/user1/ezdata $rfs_tmp/usr/share/
	cd $rfs_tmp/usr/share/thttpd/html/
	ls | grep -v cgi-bin |xargs rm -rf
	ln -sf cgi-bin/pushdongleinfo.cgi dongleInfo.json
	cd $rfs_tmp/usr/share/thttpd/html/cgi-bin/

	ls | grep -v "websetting.cgi" | xargs rm -rf
	cd $rfs_tmp/usr/share/thttpd/html/cgi-bin/
	ln -sf websetting.cgi wifi_info_GET.cgi
	ln -sf websetting.cgi password_POST.cgi
	ln -sf websetting.cgi add_network_POST.cgi
	ln -sf websetting.cgi set_wifi_POST.cgi
	ln -sf websetting.cgi delete_password_POST.cgi
	ln -sf websetting.cgi set_defaultmode_POST.cgi
	ln -sf websetting.cgi get_my_mac.cgi
	ln -sf websetting.cgi upload.cgi
	ln -sf websetting.cgi pushdongleinfo.cgi
	ln -sf websetting.cgi set_resolution_POST.cgi
	ln -sf websetting.cgi factory_test.cgi
	ln -sf websetting.cgi set_devicename.cgi
	ln -sf websetting.cgi set_devicename_reboot.cgi
	ln -sf websetting.cgi set_lan_POST.cgi

	if [ $ezwire_flag -eq 0 ];then
#		ls | grep -v "get_my_mac.cgi" | grep -v "upload.cgi" | grep -v "pushdongleinfo.cgi" | grep -v "wifi_info_GET.cgi" | grep -v "password_POST.cgi" | grep -v "add_network_POST.cgi" | grep -v "set_wifi_POST.cgi" | grep -v "delete_password_POST.cgi" | grep -v "set_defaultmode_POST.cgi" | xargs rm -rf
		copy_file $TOPDIR/Flash/MiraScreen_Free/websetting/* $rfs_tmp/usr/share/thttpd/html/
		
		cd $rfs_tmp/usr/share/thttpd/html
		ln -sf websetting.html ./index.html
		copy_file $TOPDIR/Flash/EZCast_smaller/config/thttpd.conf $rfs_tmp/etc/
	fi
	ls -lh

	cd $rfs_tmp/usr/share/ezdata
	
	if [ $ezcast5g_flag -eq 0 -a $mirascreent5g_flag -eq 0 ];then
		ls | grep "ezcast_icon_5g.png" | xargs rm -rf
		delete_file $rfs_tmp/lib/modules/2.6.27.29/8821cu.ko
	else
		if [ $config_3to1 -eq 0 -o $chromecast_flag -eq 1 ];then
			delete_file $rfs_tmp/lib/modules/2.6.27.29/8188fu.ko
		fi
		delete_file $rfs_tmp/lib/modules/2.6.27.29/8821au.ko
	fi
	if [ $ezwire_flag -eq 0 ];then
		ls | grep -v "ezcast_icon_" | grep -v "ezosdpalette.plt" | grep -v "music_cover.jpg" | xargs rm -rf
		ls | grep "ezcast_icon_14.png" | xargs rm -rf
		ls | grep "ezcast_icon_audio.png" | xargs rm -rf
		ls | grep "ezcast_icon_connecting[1-3].png" | xargs rm -rf
		ls | grep "ezcast_icon_ip_hotspot.png" | xargs rm -rf
		ls | grep "ezcast_icon_ip_prompt.png" | xargs rm -rf
		ls | grep "ezcast_icon_mouse.png" | xargs rm -rf
		ls | grep "ezcast_icon_qc.png" | xargs rm -rf
		ls | grep "ezcast_icon_wire.png" | xargs rm -rf
	else
		ls | grep -v "ezcast_icon_" | grep -v "ezosdpalette.plt" | grep -v "music_cover.jpg" |  grep -v "ezwire_" | xargs rm -rf
		ls | grep "ezcast_icon_[2-9].png" | xargs rm -rf
		ls | grep "ezcast_icon_1[0-4].png" | xargs rm -rf
		delete_file $rfs_tmp/usr/share/ezdata/ezcast_icon_qc.png
		delete_file $rfs_tmp/usr/share/ezdata/ezcast_icon_pop.png
	fi

	if [ $ezwire_flag -eq 0 -o $ezwire_type -ne 10 ]; then
		delete_file $rfs_tmp/alsa
		delete_file $rfs_tmp/usr/bin/audio_time
		delete_file $rfs_tmp/usr/bin/do_cmd
		delete_file $rfs_tmp/usr/bin/getDacTime.app
		delete_file $rfs_tmp/usr/bin/adb
		delete_file $rfs_tmp/am7x/case/data/amscreen*
		delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-hwdep.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-page-alloc.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-pcm.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-rawmidi.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-timer.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-usb-audio.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-usb-lib.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/snd.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/soundcore.ko
		delete_file $rfs_tmp/am7x/lib/libasound.so*
		delete_file $rfs_tmp/usr/share/ezdata/ezcast_icon_audio.png
		if [ $chip_major -ne 3 -o $config_3to1 -ne 0 ];then
			delete_file $rfs_tmp/am7x/case/data/setting.swf
		fi
	fi
	
	find $rfs_tmp -name '8192cu.ko' | xargs rm -rf;
	find $rfs_tmp -name '8192du.ko' | xargs rm -rf;
	find $rfs_tmp -name '8192eu.ko' | xargs rm -rf;
	find $rfs_tmp -name '8188eu.ko' | xargs rm -rf;
#	find $rfs_tmp -name '8188fu.ko' | xargs rm -rf;

	if [ $ezwire_flag -eq 0 ];then
		find $rfs_tmp -name 'libimirror*' | xargs rm -rf;
		find $rfs_tmp -name 'libusb*' | xargs rm -rf;
#		find $rfs_tmp -name 'usb-storage*' | xargs rm -rf;
		find $rfs_tmp -name 'libwirePlayer*' | xargs rm -rf;

		delete_file $rfs_tmp/sbin/fsck.ext2
		delete_file $rfs_tmp/lib/modules/2.6.27.29/g_ether.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_udc.ko
#		delete_file $rfs_tmp/lib/modules/2.6.27.29/modules.*
		delete_file $rfs_tmp/lib/modules/2.6.27.29/modules.alias*
		delete_file $rfs_tmp/lib/modules/2.6.27.29/modules.ccwmap
		delete_file $rfs_tmp/lib/modules/2.6.27.29/modules.ieee1394map
		delete_file $rfs_tmp/lib/modules/2.6.27.29/modules.symbols*
		delete_file $rfs_tmp/lib/modules/2.6.27.29/modules.usbmap
		delete_file $rfs_tmp/lib/modules/2.6.27.29/rndis_host.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/usbnet.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/ipheth.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/btvd_i2c.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/zaurus.ko
		delete_file $rfs_tmp/usr/sbin/usbmuxd
		delete_file $rfs_tmp/sbin/mrouted
		delete_file $rfs_tmp/usr/sbin/idevice*

#ls *.swf | grep -v main.swf | grep -v test.swf |xargs rm -f
#		ls *.swf | grep -v main_smaller.swf | grep -v ezcast.swf | grep -v ezcast_background_smaller.swf | grep -v test_smaller.swf | grep -v main_customize_smaller.swf | grep -v Main_top_bar_smaller.swf | grep -v miracast_smaller.swf | grep -v miracast_lite_smaller.swf |xargs rm -f
#		mv ./main_smaller.swf ./main.swf
#		mv ./test_smaller.swf ./test.swf
#		mv ./ezcast_background_smaller.swf ./ezcast_background.swf  
#		mv ./main_customize_smaller.swf ./main_customize.swf
#		mv ./Main_top_bar_smaller.swf ./Main_top_bar.swf
#		mv ./miracast_smaller.swf ./miracast.swf
#		mv ./miracast_lite_smaller.swf ./miracast_lite.swf

#		rm ./test.swf
#		rm ./ezcast_background.swf
		cd -

#		delete_file $rfs_tmp/usr/share/ezdata/*.png

		mkdir $rfs_tmp/usr/share/wifi
		cd $rfs_tmp/usr/share/wifi
		cp -dprf $TOPDIR/sdk/user1/softap/rtl_hostapd.conf $rfs_tmp/usr/share/wifi/
		cp -dprf $TOPDIR/sdk/user1/softap/rtl_hostapd_01.conf $rfs_tmp/usr/share/wifi/
		cp -dprf $TOPDIR/sdk/user1/dns/resolv.conf $rfs_tmp/usr/share/wifi/
		cp -dprf $TOPDIR/sdk/user1/named/named.conf $rfs_tmp/usr/share/wifi/
		cd $rfs_tmp/etc/
		ln -sf /tmp/named.conf ./named.conf
		ln -sf  /tmp/rtl_hostapd_01.conf  ./rtl_hostapd_01.conf
		ln -sf  /tmp/rtl_hostapd.conf  ./rtl_hostapd.conf
	else
		find $rfs_tmp -name '8188fu.ko' | xargs rm -rf;
		delete_file $rfs_tmp/am7x/bin/miracast.app
		delete_file $rfs_tmp/sbin/hostapd
		delete_file $rfs_tmp/sbin/iwlist
		delete_file $rfs_tmp/sbin/iwconfig
		delete_file $rfs_tmp/sbin/wpa_cli
		delete_file $rfs_tmp/sbin/hostapd_cli
		delete_file $rfs_tmp/sbin/wpa_supplicant
		delete_file $rfs_tmp/sbin/fsck.fat
		delete_file $rfs_tmp/am7x/lib/libxt*
		delete_file $rfs_tmp/am7x/case/data/*_smaller.swf
		if [ $usb1_disable -ne 1 ]; then
			delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_hcd.ko
		fi
		if [ $ezwire_type -ne 7 ]; then
			delete_file $rfs_tmp/sbin/dnsmasq
		fi
	fi

	if [ -d $rfs_tmp/usr/share/thttpd/html/ ];then
		change_mode "$rfs_tmp/usr/share/thttpd/html/" "*.html" "a-x"
		change_mode "$rfs_tmp/usr/share/thttpd/html/" "*.js" "a-x"
		change_mode "$rfs_tmp/usr/share/thttpd/html/" "*.svg" "a-x"
		change_mode "$rfs_tmp/usr/share/thttpd/html/" "*.png" "a-x"
		change_mode "$rfs_tmp/usr/share/thttpd/html/" "*.cgi" "a+x"
		change_mode "$rfs_tmp/usr/share/thttpd/html/" "*.jpg" "a+x"
		change_mode "$rfs_tmp/usr/share/thttpd/html/" "*.css" "a+x"
		change_mode "$rfs_tmp/usr/share/thttpd/html/" "*.gif" "a+x"
	fi

	chmod a+x $rfs_tmp/debug.app

	cd $TOPDIR/scripts
}

ezwire_8m_or_4m_rm()
{
	echo "#####################################################"
	echo "#              ezwire_8m_or_4m_rm               #"
	echo "#####################################################"
	rfs_tmp=$TOPDIR/scripts/rfstmp
	Patch=$TOPDIR/Flash/EZCast/Patch

	copy_file $SDK_DIR/initrd/etc/mkdev.conf $rfs_tmp/etc/
	copy_file $Patch/snor/rcS.fat $rfs_tmp/etc/init.d/rcS
#	copy_file $Patch/capture/wire_256.plt $rfs_tmp/am7x/case/data/
#	copy_file $Patch/capture/wire_256_RGB_Default.bin $rfs_tmp/am7x/case/data/
	copy_file $Patch/snor/osd/osdIconPalette.plt $rfs_tmp/am7x/case/data/
	copy_file $Patch/snor/osd/OSD_RGB_Default.bin $rfs_tmp/am7x/case/data/
	
	copy_file $TOPDIR/Flash/EZCast_smaller/patch_ezwire_snor/thttpd $rfs_tmp/usr/share/
	copy_file $TOPDIR/Flash/EZCast_smaller/config/thttpd.conf $rfs_tmp/etc/
	if [ -d $rfs_tmp/usr/share/thttpd/html/ ];then
		change_mode "$rfs_tmp/usr/share/thttpd/html/" "*.html" "a-x"
		change_mode "$rfs_tmp/usr/share/thttpd/html/" "*.js" "a-x"
		cd $rfs_tmp/usr/share/thttpd/html
		ln -sf websetting.html ./index.html
		find ./cgi-bin/ -name '*.cgi' | xargs chmod 755
		cd -
		cd $rfs_tmp/usr/share/thttpd/html/cgi-bin
		ln -sf  websetting.cgi set_defaultmode_POST.cgi
		ln -sf  websetting.cgi get_defaultmode.cgi
		cd -
	fi

	copy_file $Patch/capture/red_dot* $rfs_tmp/am7x/case/data/
	if [ $ezwire_type -eq 10 ];then
		if [ $adui_enable -eq 1 ];then
			copy_file $Patch/capture/mirawire_ad_bg.jpg $rfs_tmp/am7x/case/data/bg.jpg
			copy_file $Patch/capture/tethering_mode.jpg $rfs_tmp/am7x/case/data/
			copy_file $Patch/capture/pnp_mode.jpg $rfs_tmp/am7x/case/data/
#			delete_file $rfs_tmp/usr/ota_from
		else
			copy_file $Patch/capture/miraplug_bg.jpg $rfs_tmp/am7x/case/data/
			copy_file $Patch/capture/plugOn.jpg $rfs_tmp/am7x/case/data/
			copy_file $Patch/capture/plugOff.jpg $rfs_tmp/am7x/case/data/
			copy_file $Patch/capture/airplayOn.jpg $rfs_tmp/am7x/case/data/
			copy_file $Patch/capture/airplayOff.jpg $rfs_tmp/am7x/case/data/
		fi
		delete_file $rfs_tmp/am7x/lib/libxml2.so.2.7.8
		delete_file $rfs_tmp/lib/modules/2.6.27.29/ipv6.ko
		
	else
		if [ $adui_enable -eq 1 ];then
			copy_file $Patch/capture/mirawire_ad_bg.jpg $rfs_tmp/am7x/case/data/bg.jpg
			copy_file $Patch/capture/tethering_mode.jpg $rfs_tmp/am7x/case/data/
			copy_file $Patch/capture/pnp_mode.jpg $rfs_tmp/am7x/case/data/
#			delete_file $rfs_tmp/usr/ota_from
		else
			copy_file $Patch/capture/bg.jpg $rfs_tmp/am7x/case/data/
			copy_file $Patch/capture/miralink_15.jpg $rfs_tmp/am7x/case/data/
			copy_file $Patch/capture/miralink_18.jpg $rfs_tmp/am7x/case/data/
		fi
		delete_file $rfs_tmp/am7x/lib/libswfdec.so
		delete_file $rfs_tmp/am7x/lib/libgraph.so
		delete_file $rfs_tmp/am7x/lib/libezcast.so
		delete_file $rfs_tmp/am7x/lib/libwifi_subdisplay.so

	fi
	copy_file $Patch/capture/0.jpg $rfs_tmp/am7x/case/data/
	copy_file $Patch/capture/1.jpg $rfs_tmp/am7x/case/data/
	copy_file $Patch/capture/2.jpg $rfs_tmp/am7x/case/data/
	copy_file $Patch/capture/3.jpg $rfs_tmp/am7x/case/data/
	copy_file $Patch/capture/4.jpg $rfs_tmp/am7x/case/data/
	copy_file $Patch/capture/5.jpg $rfs_tmp/am7x/case/data/
	copy_file $Patch/capture/6.jpg $rfs_tmp/am7x/case/data/
	copy_file $Patch/capture/7.jpg $rfs_tmp/am7x/case/data/
	copy_file $Patch/capture/8.jpg $rfs_tmp/am7x/case/data/
	copy_file $Patch/capture/9.jpg $rfs_tmp/am7x/case/data/
	if [ $adui_enable -eq 0 ];then
		copy_file $Patch/capture/miralink_03.jpg $rfs_tmp/am7x/case/data/
		copy_file $Patch/capture/miralink_05.jpg $rfs_tmp/am7x/case/data/
		copy_file $Patch/capture/miralink_09.jpg $rfs_tmp/am7x/case/data/
		copy_file $Patch/capture/miralink_07.jpg $rfs_tmp/am7x/case/data/
		copy_file $Patch/capture/miralink_11.jpg $rfs_tmp/am7x/case/data/
		copy_file $Patch/capture/miralink_13.jpg $rfs_tmp/am7x/case/data/
		copy_file $Patch/capture/warn_aoa.jpg $rfs_tmp/am7x/case/data/
		copy_file $Patch/capture/warn_adb.jpg $rfs_tmp/am7x/case/data/
		copy_file $Patch/capture/warn_empty.jpg $rfs_tmp/am7x/case/data/
	fi
	copy_file $TOPDIR/Flash/EZCast/Patch/snor/msyh.ttf $rfs_tmp/am7x/case/data/

	delete_file $rfs_tmp/bin/lrz
	delete_file $rfs_tmp/bin/curl

	delete_file $rfs_tmp/sbin/hostapd
	delete_file $rfs_tmp/sbin/iwconfig
	delete_file $rfs_tmp/sbin/iwlist
	delete_file $rfs_tmp/sbin/wpa_cli
	delete_file $rfs_tmp/sbin/hostapd_cli
	delete_file $rfs_tmp/sbin/iwevent
	delete_file $rfs_tmp/sbin/iwpriv
	delete_file $rfs_tmp/sbin/wpa_passphrase
	delete_file $rfs_tmp/sbin/iperf
	delete_file $rfs_tmp/sbin/iwgetid
	delete_file $rfs_tmp/sbin/iwspy
	delete_file $rfs_tmp/sbin/wpa_supplicant
#	delete_file $rfs_tmp/sbin/thttpd
#	delete_file $rfs_tmp/sbin/fsck.fat
	delete_file $rfs_tmp/sbin/fsck.ext2
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_cfcard.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_eMMCcard.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_it6681.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_mscard.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_xdcard.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/at24_i2c.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/at25.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/cifs.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/g_file_debug.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/g_file_storage.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/g_subdisplay.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/i2c_wm8988.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/i2s_i2c.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/keenhi_rfkey.ko
	#delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_keys.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_cec.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/amfb.ko
	#delete_file $rfs_tmp/am7x/lib/libezota.so
	delete_file $rfs_tmp/am7x/lib/libwebsockets.so.8.1
	#delete_file $rfs_tmp/sbin/thttpd
	delete_file $rfs_tmp/am7x/lib/libwav.so
	#delete_file $rfs_tmp/am7x/case/scripts/eth_usb.sh
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_uartchar.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_btvd.ko
	delete_file $rfs_tmp/usr/lib/libamsocket.so
	#unlink $rfs_tmp/lib/llibstdc++.so
	#unlink $rfs_tmp/lib/libstdc++.so.6
	#delete_file $rfs_tmp/lib/libstdc++.so.6.0.10
	unlink $rfs_tmp/sbin/iptables
	unlink $rfs_tmp/sbin/iptables-restore
	unlink $rfs_tmp/sbin/iptables-save
	delete_file $rfs_tmp/sbin/xtables-multi
	delete_file $rfs_tmp/lib/modules/2.6.27.29/modules.alias.bin
	delete_file $rfs_tmp/lib/modules/2.6.27.29/modules.alias
	delete_file $rfs_tmp/am7x/case/scripts/miracast_p2p.sh
	#delete_file $rfs_tmp/am7x/case/scripts/usb_process.sh
	#delete_file $rfs_tmp/usr/ota_from
	#delete_file $rfs_tmp/am7x/lib/libstream.so
	delete_file $rfs_tmp/lib/libnss*
		
	delete_file $rfs_tmp/lib/libcrypt.so.1
	#delete_file $rfs_tmp/lib/modules/2.6.27.29/ipheth.ko
	#delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_spdif.ko
	delete_file $rfs_tmp/bin/get_dhcpd_peer_ip
	delete_file $rfs_tmp/lib/modules/2.6.27.29/ambl.ko
	delete_file $rfs_tmp/am7x/sdk/standby.bin
	delete_file $rfs_tmp/lib/modules/2.6.27.29/actions_mci.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/mbcache.ko
	delete_file $rfs_tmp/lib/libresolv-2.8.so
	delete_file $rfs_tmp/am7x/case/scripts/wifi*
	#delete_file $rfs_tmp/debug.app
	delete_file $rfs_tmp/bin/go_ch
	delete_file $rfs_tmp/etc/wpa-tkip.conf
	delete_file $rfs_tmp/etc/wpa*
	delete_file $rfs_tmp/etc/udhcpd*
	delete_file $rfs_tmp/mnt/user1/softap/rtl_hostapd.conf
	#delete_file $rfs_tmp/lib/libm-2.8.so
	#delete_file $rfs_tmp/am7x/bin/pthsystem.app
	delete_file $rfs_tmp/lib/modules/2.6.27.29/btvd_i2c.ko
	delete_file $rfs_tmp/case/scripts/udisk_format.sh


		
	delete_file $rfs_tmp/mnt/user1/softap/rtl_hostapd_01.conf
	delete_file $rfs_tmp/mnt/bak/rtl_hostapd.conf
	delete_file $rfs_tmp/mnt/bak/rtl_hostapd_01.conf
	delete_file $rfs_tmp/mnt/vram/wifi/resolv*
	delete_file $rfs_tmp/am7x/case/scripts/ralink_softap.sh
	delete_file $rfs_tmp/etc/p2p*
	delete_file $rfs_tmp/am7x/case/scripts/route_exit.sh
	delete_file $rfs_tmp/etc/miracast*
	delete_file $rfs_tmp/etc/wireless-tkip.conf
	delete_file $rfs_tmp/mnt/bak/wireless-tkip.conf
	#delete_file $rfs_tmp/etc/thttpd.conf
	delete_file $rfs_tmp/mnt/user1/clientap/RT2870STA.dat
	delete_file $rfs_tmp/mnt/user1/softap/RT2870AP.dat
	delete_file $rfs_tmp/mnt/bak/RT2870AP.dat
	delete_file $rfs_tmp/mnt/user1/dns/resolv.conf
	delete_file $rfs_tmp/etc/ralink.dat
	delete_file $rfs_tmp/etc/ra0-udhcpd.conf
	delete_file $rfs_tmp/bestchannel_has_choose.txt
	delete_file $rfs_tmp/mnt/bak/bestchannel_has_choose.txt
	delete_file $rfs_tmp/mnt/user1/softap/bestchannel_has_choose.txt
	delete_file $rfs_tmp/am7x/case/scripts/card_process.sh
	delete_file $rfs_tmp/am7x/case/scripts/change_softap_config_for_ralink.sh
	delete_file $rfs_tmp/am7x/case/scripts/change_softap_config_for_realtek.sh
	delete_file $rfs_tmp/am7x/case/scripts/config_wifi.sh
	delete_file $rfs_tmp/am7x/case/scripts/udisk_format.sh
	delete_file $rfs_tmp/etc/ssid_pre
	delete_file $rfs_tmp/am7x/case/scripts/iptalbes_nat.sh
	#delete_file $rfs_tmp/am7x/bin/install-usbnet.sh
	delete_file $rfs_tmp/bin/gdbreplay
	delete_file $rfs_tmp/am7x/case/scripts/network_para.sh
	delete_file $rfs_tmp/am7x/case/scripts/network_control.sh
	#delete_file $rfs_tmp/lib/modules/2.6.27.29/edid_i2c_gpio.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/ext2.ko
	
	if [ $ezwire_type -eq 8 ];then
		delete_file $rfs_tmp/sbin/fsck.fat
		delete_file $rfs_tmp/lib/modules/2.6.27.29/ipv6.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/asix.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/sit.ko
		#delete_file $rfs_tmp/lib/modules/2.6.27.29/net1080.ko
		#delete_file $rfs_tmp/lib/modules/2.6.27.29/tunnel4.ko
		#delete_file $rfs_tmp/lib/modules/2.6.27.29/xfrm6_mode_beet.ko
		#delete_file $rfs_tmp/lib/modules/2.6.27.29/xfrm6_mode_tunnel.ko
		#delete_file $rfs_tmp/lib/modules/2.6.27.29/zaurus.ko
		delete_file $rfs_tmp/am7x/lib/libxml2.so.2.7.8
		delete_file $rfs_tmp/am7x/lib/libusb-0.1.so.4
		delete_file $rfs_tmp/lib/modules/2.6.27.29/edid_i2c_gpio.ko
		
		delete_file $rfs_tmp/lib/modules/2.6.27.29/net1080.ko
		delete_file $rfs_tmp/lib/firmware/zd1211/zd1211b*
		
		delete_file $rfs_tmp/lib/modules/2.6.27.29/xfrm6_mode*
		delete_file $rfs_tmp/lib/modules/2.6.27.29/tunnel4.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29z/aurus.ko
		if [ $adb_mirroronly -eq 1 ];then
			delete_file $rfs_tmp/am7x/lib/libcrypto.so.1.0.0
			delete_file $rfs_tmp/am7x/lib/libssl.so.1.0.0
			delete_file $rfs_tmp/usr/sbin/ideviceinfo
			delete_file $rfs_tmp/am7x/lib/libimirror.so	
			delete_file $rfs_tmp/usr/sbin/usbmuxd
		else
             		delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-hwdep.ko
               	delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-page-alloc.ko
               	delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-pcm.ko
               	delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-rawmidi.ko
			delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-timer.ko
               	delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-usb-audio.ko              
               	delete_file $rfs_tmp/lib/modules/2.6.27.29/snd-usb-lib.ko
               	delete_file $rfs_tmp/lib/modules/2.6.27.29/snd.ko
			delete_file $rfs_tmp/lib/modules/2.6.27.29/soundcore.ko
             	 	delete_file $rfs_tmp/usr/bin/adb
               	delete_file $rfs_tmp/usr/bin/audio_time
               	delete_file $rfs_tmp/usr/bin/do_cmd
               	delete_file $rfs_tmp/usr/bin/getDacTime.app-               
               	delete_file $rfs_tmp/am7x/case/data/amscreen*              
               	delete_file $rfs_tmp/am7x/lib/libasound.so*
               	delete_file $rfs_tmp/lib/modules/2.6.27.29/cdc_ether.ko
               	delete_file $rfs_tmp/lib/modules/2.6.27.29/cdc_subset.ko
               	delete_file $rfs_tmp/lib/modules/2.6.27.29/usbnet.ko
               	delete_file $rfs_tmp/lib/modules/2.6.27.29/hci_usb.ko
               	delete_file $rfs_tmp/alsa
               	delete_file $rfs_tmp/am7x/case/data/warn*.jpg 

		fi
	fi
	if [ $usb1_disable -ne 1 ];then
		delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_uoc.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_hcd.ko
	fi
	delete_file $rfs_tmp/usr/bin/adbForward.app
	

	
#	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_hcd.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_udc_next.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_carddet.ko
#	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_uoc.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_udc.ko
	delete_file $rfs_tmp/sbin/dnsmasq
	delete_file $rfs_tmp/lib/modules/2.6.27.29/g_ether.ko



	delete_file $rfs_tmp/usr/sbin/snmp_am

	delete_file $rfs_tmp/am7x/bin/miracast.app
	delete_file $rfs_tmp/am7x/bin/rs232_main.app

	cd $rfs_tmp/am7x/case/data
	delete_file $rfs_tmp/am7x/case/data/AdbService.apk
	delete_file $rfs_tmp/am7x/case/data/fui.res
	delete_file $rfs_tmp/am7x/case/data/connectPC.swd
	delete_file $rfs_tmp/am7x/case/data/slitemplate
	delete_file $rfs_tmp/am7x/case/data/test.swf
	delete_file $rfs_tmp/am7x/case/data/setting.swf
	ls *.swf | grep -v main.swf |xargs rm -f
	cd -

	delete_file $rfs_tmp/am7x/lib/libxt*

	delete_file $rfs_tmp/am7x/lib/lib_asf.so
	delete_file $rfs_tmp/am7x/lib/lib_avi.so
	delete_file $rfs_tmp/am7x/lib/lib_flv.so
	delete_file $rfs_tmp/am7x/lib/lib_mkv.so
	delete_file $rfs_tmp/am7x/lib/lib_mjpg.so
	delete_file $rfs_tmp/am7x/lib/lib_mov.so
	delete_file $rfs_tmp/am7x/lib/lib_mpeg2.so
	delete_file $rfs_tmp/am7x/lib/lib_mpeg4.so
	delete_file $rfs_tmp/am7x/lib/lib_rm.so
	delete_file $rfs_tmp/am7x/lib/lib_rv.so
	delete_file $rfs_tmp/am7x/lib/lib_ts.so
	delete_file $rfs_tmp/am7x/lib/lib_vc1.so
	delete_file $rfs_tmp/am7x/lib/lib_vp8.so

	delete_file $rfs_tmp/am7x/lib/libflac.so
	delete_file $rfs_tmp/am7x/lib/libdts.so
	delete_file $rfs_tmp/am7x/lib/libamr.so
	delete_file $rfs_tmp/am7x/lib/libape.so

#	mkdir $rfs_tmp/am7x/ezdata
#	copy_file $SDK_DIR/user1/ezdata/ezcast_icon* $rfs_tmp/am7x/ezdata/
#	copy_file $SDK_DIR/user1/ezdata/ap_link_1.png $rfs_tmp/am7x/ezdata/

	delete_file $rfs_tmp/am7x/lib/libdlna*
	delete_file $rfs_tmp/am7x/lib/libiconv.so*

#	delete_file $rfs_tmp/am7x/lib/libxml2.so*
	delete_file $rfs_tmp/am7x/lib/wpa_cli.so
	delete_file $rfs_tmp/am7x/lib/libfreetype.so*
	delete_file $rfs_tmp/am7x/lib/libfairplay.so
	delete_file $rfs_tmp/am7x/lib/libhyfviewer.so
#	delete_file $rfs_tmp/am7x/lib/libcrypto.so*
	delete_file $rfs_tmp/am7x/lib/libsqlite3.so*
	delete_file $rfs_tmp/am7x/lib/lib_id_tif.so
	delete_file $rfs_tmp/am7x/lib/libmtp.so*
	delete_file $rfs_tmp/am7x/lib/libcurl.so*
#	delete_file $rfs_tmp/am7x/lib/libssl.so*
#	delete_file $rfs_tmp/am7x/lib/lib_load_photo.so*
	delete_file $rfs_tmp/am7x/lib/libavahi-core.so*
	delete_file $rfs_tmp/am7x/lib/libduktape.so
	delete_file $rfs_tmp/am7x/lib/lib_id_jpg.so
	delete_file $rfs_tmp/am7x/lib/lib_id_png.so
	delete_file $rfs_tmp/am7x/lib/libortp.so*
	delete_file $rfs_tmp/am7x/lib/libhdcp2.so
	delete_file $rfs_tmp/am7x/lib/libsub00000000xy.so
	delete_file $rfs_tmp/am7x/lib/librtmp.so*
#	delete_file $rfs_tmp/am7x/lib/libvideo_player.so
#	delete_file $rfs_tmp/am7x/lib/libstream.so
	delete_file $rfs_tmp/am7x/lib/libmusic.so
	delete_file $rfs_tmp/am7x/lib/lib_id_bmp.so
	delete_file $rfs_tmp/am7x/lib/libmms.so*
	delete_file $rfs_tmp/am7x/lib/libcook.so*
	delete_file $rfs_tmp/am7x/lib/libmp3.so
	delete_file $rfs_tmp/am7x/lib/libavahi-common.so*
	delete_file $rfs_tmp/am7x/lib/libxtables.so*
	delete_file $rfs_tmp/am7x/lib/libaac-eld.so
	delete_file $rfs_tmp/am7x/lib/libogg.so
	delete_file $rfs_tmp/am7x/lib/lib_ps.so
	delete_file $rfs_tmp/am7x/lib/libac3.so
	delete_file $rfs_tmp/am7x/lib/libmnavi.so
	delete_file $rfs_tmp/am7x/lib/libplist.so
	delete_file $rfs_tmp/am7x/lib/libsqldb.so
	delete_file $rfs_tmp/am7x/lib/libip6tc.so*
	delete_file $rfs_tmp/am7x/lib/libip4tc.so*
	if [ $adui_enable -eq 0 ];then
		delete_file $rfs_tmp/am7x/lib/libqrcode.so
		delete_file $rfs_tmp/am7x/lib/lib_enc_buf.so
	fi
	delete_file $rfs_tmp/am7x/lib/libfuse.so*
	delete_file $rfs_tmp/am7x/lib/libiw.so*
	delete_file $rfs_tmp/am7x/lib/libxt_*
	delete_file $rfs_tmp/am7x/lib/libatrac3.so
	delete_file $rfs_tmp/am7x/lib/libftp_client.so
	delete_file $rfs_tmp/am7x/lib/lib_image.so
	delete_file $rfs_tmp/am7x/lib/libmirartp_stream.so
#	delete_file $rfs_tmp/am7x/lib/libaudio_player.so
	delete_file $rfs_tmp/am7x/lib/libape.so
	delete_file $rfs_tmp/am7x/lib/libnetsnmpsrc.so
	delete_file $rfs_tmp/am7x/lib/libflac.so
	delete_file $rfs_tmp/am7x/lib/libmultilang.so
	delete_file $rfs_tmp/am7x/lib/libfilelist.so
	delete_file $rfs_tmp/am7x/lib/librecorder.so
	delete_file $rfs_tmp/am7x/lib/libipt_*
#	delete_file $rfs_tmp/am7x/lib/liblib_ds.so
	delete_file $rfs_tmp/am7x/lib/libamsocket.so
	delete_file $rfs_tmp/am7x/lib/libinput.so
	delete_file $rfs_tmp/am7x/lib/liblinear.so
	delete_file $rfs_tmp/am7x/lib/libvariance.so
	delete_file $rfs_tmp/am7x/lib/libdejitter.so
	delete_file $rfs_tmp/am7x/lib/libpthres.so
	delete_file $rfs_tmp/am7x/lib/libiptc.so*
	delete_file $rfs_tmp/am7x/bin/framectrl.app
	delete_file $rfs_tmp/am7x/bin/fui.app
	delete_file $rfs_tmp/am7x/bin/linein.app
#	delete_file $rfs_tmp/am7x/bin/pthsystem.app
	delete_file $rfs_tmp/am7x/case/data/HDCPPkey.bin
	delete_file $rfs_tmp/am7x/case/data/channel_region.bin
	delete_file $rfs_tmp/am7x/case/data/fb.bin
	delete_file $rfs_tmp/am7x/case/data/fileviewer.cfg
	delete_file $rfs_tmp/am7x/case/data/gamma.bin
#	delete_file $rfs_tmp/am7x/case/data/keyDriver.bin
	delete_file $rfs_tmp/am7x/case/data/global_value.bin
	delete_file $rfs_tmp/am7x/case/data/global_value.h
	delete_file $rfs_tmp/am7x/case/data/keymap/globalswfkey.bin
	delete_file $rfs_tmp/am7x/case/data/keymap
	delete_file $rfs_tmp/am7x/case/data/main.swf
	delete_file $rfs_tmp/am7x/case/data/msyh.ttf
	delete_file $rfs_tmp/am7x/lib/libaac.so
	delete_file $rfs_tmp/am7x/lib/liblinein.so
	delete_file $rfs_tmp/am7x/lib/libts.so
	delete_file $rfs_tmp/am7x/lib/libwma.so
	delete_file $rfs_tmp/etc/hostapd.conf
	delete_file $rfs_tmp/sbin/brctl
	delete_file $rfs_tmp/sbin/mrouted
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_sdcard.ko
#	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_keys.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_graph.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/mii.ko
	delete_file $rfs_tmp/lib/modules/2.6.27.29/mmc_core.ko
	cd $rfs_tmp/usr/share/
	touch tmp.tmp
	ls | grep -v "udhcpc" | grep -v "thttpd" | xargs rm -rf

	chmod a+x $rfs_tmp/debug.app
	
	cd $TOPDIR/scripts
}

customer_rootfs_rm()
{
	echo "#####################################################"
	echo "#              Product patch rootfs                 #"
	echo "#####################################################"

	rfs_tmp=$TOPDIR/scripts/rfstmp
	images_dir=$TOPDIR/case/images
	ezcast_dir=$images_dir/ezcast
	ezwire_dir=$images_dir/ezwire
	ez3to1_dir=$TOPDIR/Flash/MiraScreen_Free/SWF
	ezcast_snor_dir=$TOPDIR/Flash/EZCast_snor/SWF
	data_dir=$rfs_tmp/am7x/case/data
	echo "rfs_tmp: "$rfs_tmp
	
	if [ $ezcast_flag -eq 1 ];then
		delete_file $data_dir/*.swf
		delete_file $data_dir/bmp
		delete_file $data_dir/fui.XLS
		delete_file $data_dir/fui.res
		
		copy_file $ezcast_dir/data/* $data_dir/
		echo $ezcast_dir"/data/*  -->  "$data_dir
		
		if [ $ezwire_flag -eq 0 ];then
			if [ $config_3to1 -eq 1 ];then
				delete_file  $data_dir/*.swf
				copy_file  $ez3to1_dir/*.swf $data_dir/
				delete_file $data_dir/miracast.swf
				if [ $chip_type -eq 33 ];then
					copy_file  $ez3to1_dir/../FLA/main_customize_plus.swf $data_dir/main_customize.swf
					copy_file  $ez3to1_dir/../FLA/Main_top_bar_plus.swf $data_dir/Main_top_bar.swf
				fi
				copy_file $ez3to1_dir/../patch/red_dot*.png $rfs_tmp/am7x/case/data/
				copy_file $ez3to1_dir/../patch/ezcast_icon_*.png $rfs_tmp/usr/share/ezdata/
				delete_file $rfs_tmp/usr/share/ezdata/ezcast_icon_setting.png
				echo "$ez3to1_dir/*.swf  -->  "$data_dir
			elif [  $config_3to1 -eq 2 ];then
				delete_file  $data_dir/*.swf
				copy_file $TOPDIR/Flash/EZCast_smaller/SWF/* $data_dir/
				copy_file $ez3to1_dir/../patch/red_dot*.png $rfs_tmp/am7x/case/data/
				#delete_file $rfs_tmp/am7x/case/data/OTA_download.swf
				#delete_file $rfs_tmp/am7x/case/data/test.swf
				#delete_file $rfs_tmp/am7x/case/data/miracast.swf
				#delete_file $rfs_tmp/am7x/case/data/main_customize.swf
				delete_file $data_dir/miracast_lite.swf
			elif [ $chip_major -eq 3 ];then
				delete_file  $data_dir/*.swf
				copy_file  $ezcast_snor_dir/main.swf $data_dir/
				copy_file  $ezcast_snor_dir/miracast.swf $data_dir/
				copy_file  $ezcast_snor_dir/ezcast_background.swf $data_dir/
				copy_file  $ezcast_snor_dir/ezcast.swf $data_dir/
				copy_file  $ezcast_snor_dir/main_customize.swf $data_dir/
				copy_file  $ezcast_snor_dir/Main_top_bar.swf $data_dir/
				copy_file  $ezcast_snor_dir/miracast_lite.swf $data_dir/
				copy_file  $ezcast_snor_dir/OTA_download.swf $data_dir/
				copy_file  $ezcast_snor_dir/setting.swf $data_dir/
				copy_file  $ezcast_snor_dir/test.swf $data_dir/
				copy_file  $ezcast_snor_dir/testOSD.swf $data_dir/
			elif [ $flash_type != 0 ];then
				delete_file  $data_dir/*.swf
				copy_file $TOPDIR/Flash/EZCast_smaller/SWF/* $data_dir/
				#delete_file $rfs_tmp/am7x/case/data/OTA_download.swf
				#delete_file $rfs_tmp/am7x/case/data/test.swf
				#delete_file $rfs_tmp/am7x/case/data/miracast.swf
				#delete_file $rfs_tmp/am7x/case/data/main_customize.swf
				delete_file $data_dir/miracast_lite.swf
			fi
		else
			if [ $chip_type -eq 37 ]; then
				copy_file $ez3to1_dir/../patch/red_dot*.png $rfs_tmp/am7x/case/data/
			fi
			delete_file  $data_dir/*.swf
			copy_file  $ezwire_dir/data/* $data_dir
			copy_file  $ezcast_dir/data/OTA_download.swf $data_dir
			echo $ezwire_dir"/data/*  -->  "$data_dir
		fi
	fi

	if [ $ezwilan_flag -eq 0 -a $ezmusic_flag -eq 0 -a $ezcastpro_flag -eq 0 -a $ezwire_flag -eq 0 -a $flash_type -eq 0 ]; then
		echo ">>>>>>> Remove key driver!!!"
		find $rfs_tmp -name 'keymap' -depth| xargs rm -rf;
		find $rfs_tmp -name '*key*' -depth| xargs rm -rf;
	fi
	if [ $ezmusic_flag -eq 1 ]; then
		echo ">>>>>>>EZMusic:remove 8192cu/du/eu driver!"
		find $rfs_tmp -name '8192cu.ko' | xargs rm -rf;
		find $rfs_tmp -name '8192du.ko' | xargs rm -rf;
		find $rfs_tmp -name '8192eu.ko' | xargs rm -rf;
	fi
	if [ $ezwilan_flag -eq 1 ]; then
                echo ">>>>>>>ezwilan:remove 8192cu/du/eu driver!"
                find $rfs_tmp -name '8192du.ko' | xargs rm -rf;
                find $rfs_tmp -name '8188eu.ko' | xargs rm -rf;
                find $rfs_tmp -name '8188fu.ko' | xargs rm -rf;
        fi
	if [ $ezcastpro_flag -eq 0 ]; then
                find $rfs_tmp -name '8192cu.ko' | xargs rm -rf;
		delete_local_function_swf
	fi

	find $rfs_tmp -name '1.txt' | xargs rm -rf;

	if [ $ezcastpro_flag -eq 8075 ]; then
	
		Probox5G_Path=$TOPDIR/Flash/EZCastPro/ProBox
		ezcast_dir=$TOPDIR/case/images/ezcast
		cp -dprf  $Probox5G_Path/udhcpd_02.conf  $rfs_tmp/etc 
		echo "$Probox5G_Path/udhcpd_02.conf >>>>>> $rfs_tmp/etc "
		cp -dprf  $Probox5G_Path/udhcpd.conf  $ezcast_dir/udhcpd_ezcastpro.conf 
	    	echo "$Probox5G_Path/udhcpd.conf >>>>> $ezcast_dir/udhcpd_ezcastpro.conf "
		echo   ">>>>>>>Probox  rm file !!!<<<<<<<<"
		find $rfs_tmp -name '8192cu.ko' | xargs rm -rf;
		find $rfs_tmp -name '8192du.ko' | xargs rm -rf;
		find $rfs_tmp -name '8192eu.ko' | xargs rm -rf;
		find $rfs_tmp -name '8188eu.ko' | xargs rm -rf;
		find $rfs_tmp -name 'i2c_wm8988.ko' | xargs rm -rf;
		#find $rfs_tmp -name 'i2s_i2c.ko' | xargs rm -rf;
		find $rfs_tmp -name 'mainmenu.swf ' | xargs rm -rf;
		find $rfs_tmp -name 'set_audio.swf ' | xargs rm -rf;
		find $rfs_tmp -name 'set_video.swf ' | xargs rm -rf;
		find $rfs_tmp -name ' set_photo.swf ' | xargs rm -rf;  
	fi
	if [ $ezcastpro128_flag  -eq 1 ]; then
	
		echo   ">>>>>>> Projector  rm file !!!<<<<<<<<"
		#find $rfs_tmp -name '8192cu.ko' | xargs rm -rf;
		find $rfs_tmp -name '8192du.ko' | xargs rm -rf;
		find $rfs_tmp -name '8192eu.ko' | xargs rm -rf;
		find $rfs_tmp -name '8188eu.ko' | xargs rm -rf;
		find $rfs_tmp -name '8188fu.ko' | xargs rm -rf;
		find $rfs_tmp -name 'i2c_wm8988.ko' | xargs rm -rf;
		#find $rfs_tmp -name 'i2s_i2c.ko' | xargs rm -rf;
		#find $rfs_tmp -name 'set_audio.swf' | xargs rm -rf;
		#find $rfs_tmp -name 'set_video.swf' | xargs rm -rf;
		#find $rfs_tmp -name 'set_photo.swf' | xargs rm -rf; 
		#find $rfs_tmp -name 'set_sys.swf' | xargs rm -rf;
		find $rfs_tmp -name 'miracast_probox.swf' | xargs rm -rf;
		find $rfs_tmp -name 'miracast_lite.swf' | xargs rm -rf; 
		find $rfs_tmp -name 'slibnetsnmpmibs.so.30.0.3' | xargs rm -rf; 
		#find $rfs_tmp -name 'libdlnadmr.so' | xargs rm -rf;
		find $rfs_tmp -name 'libnetsnmp.so.30.0.3' | xargs rm -rf;
		find $rfs_tmp -name 'libnetsnmpagent.so.30.0.3' | xargs rm -rf;
		rm -rf $rfs_tmp/usr/share/snmp/mibs/
		
	fi
	if [ $ezcastpro_flag -eq 8251 ]; then
		Patch5G_Path=$TOPDIR/Flash/EZCast/Patch/EZCast5G
		cp -dprf $Patch5G_Path/hostapd $rfs_tmp/sbin
		echo "$Patch5G_Path/hostapd >>>>> $rfs_tmp/sbin"
		cp -dprf $Patch5G_Path/hostapd_cli $rfs_tmp/sbin
		echo "$Patch5G_Path/hostapd_cli >>>>>> $rfs_tmp/sbin"
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
		find $rfs_tmp -name 'libfv.so' -depth| xargs rm -rf;
		find $rfs_tmp -name 'am7x_*card*.ko' -depth| xargs rm -rf ;			
		
	fi
	if [ $ezcast5g_flag -eq 0 ]; then
		find $rfs_tmp -name '8821au.ko' -depth| xargs rm -rf;
	else
		find $rfs_tmp -name '8192du.ko' | xargs rm -rf;
		find $rfs_tmp -name '8192eu.ko' | xargs rm -rf;
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

	if [ $ezwire_flag -ne 0 ]; then
		Patch=$TOPDIR/Flash/EZCast/Patch
		if [ $ezwire_type -ne 0 -a $ezwire_type -ne 4 ]; then
			copy_file $rfs_tmp/am7x/case/data/main_customize_mirawire.swf $rfs_tmp/am7x/case/data/main_customize.swf
		fi

		case $ezwire_type in
		1)
			copy_file $rfs_tmp/am7x/case/data/ezwire_bg_mirawire.swf $rfs_tmp/am7x/case/data/ezwire_bg.swf
			copy_file $rfs_tmp/am7x/case/data/ezwire_logo_mirawire.swf $rfs_tmp/am7x/case/data/ezwire_logo.swf
			;;
		2|3|10)
			if [ $chip_type -eq 37 ]; then 
				copy_file $rfs_tmp/am7x/case/data/ezwire_bg_miraplug_8268w.swf $rfs_tmp/am7x/case/data/ezwire_bg.swf
				copy_file $rfs_tmp/am7x/case/data/ezwire_logo_miraplug_8268w.swf $rfs_tmp/am7x/case/data/ezwire_logo.swf
			else
				copy_file $rfs_tmp/am7x/case/data/ezwire_bg_miraplug.swf $rfs_tmp/am7x/case/data/ezwire_bg.swf
				copy_file $rfs_tmp/am7x/case/data/ezwire_logo_miraplug.swf $rfs_tmp/am7x/case/data/ezwire_logo.swf
			fi
			;;
		4)
			copy_file $rfs_tmp/am7x/case/data/ezwire_bg_ezdock.swf $rfs_tmp/am7x/case/data/ezwire_bg.swf
			copy_file $rfs_tmp/am7x/case/data/ezwire_logo_ezdock.swf $rfs_tmp/am7x/case/data/ezwire_logo.swf
			;;
		5)
			copy_file $rfs_tmp/am7x/case/data/ezwire_bg_mirawire.swf $rfs_tmp/am7x/case/data/ezwire_bg.swf
			copy_file $rfs_tmp/am7x/case/data/ezwire_logo_mirawire.swf $rfs_tmp/am7x/case/data/ezwire_logo.swf
			;;
		6)
			copy_file $rfs_tmp/am7x/case/data/ezwire_bg_miralink.swf $rfs_tmp/am7x/case/data/ezwire_bg.swf
			copy_file $rfs_tmp/am7x/case/data/ezwire_logo_miralink.swf $rfs_tmp/am7x/case/data/ezwire_logo.swf
			;;
		7)
			copy_file $rfs_tmp/am7x/case/data/ezwire_bg_miralink.swf $rfs_tmp/am7x/case/data/ezwire_bg.swf
			copy_file $rfs_tmp/am7x/case/data/ezwire_logo_miralink.swf $rfs_tmp/am7x/case/data/ezwire_logo.swf
			;;
		esac

#if [ $ezwire_type -ne 6 -a $ezwire_type -ne 7 ]; then
#	copy_file $Patch/capture/imirror $rfs_tmp/usr/sbin/
#fi
		
		if [ $ezwire_type -ne 5 -a $ezwire_type -ne 7 ]; then
			copy_file $Patch/app/AdbService.apk $rfs_tmp/am7x/case/data/
			copy_file $Patch/adb_stream/adb $rfs_tmp/usr/bin/
#			copy_file $Patch/adb_stream/adbForward.app $rfs_tmp/usr/bin/
			copy_file $Patch/adb_stream/getDacTime.app $rfs_tmp/usr/bin/
			copy_file $Patch/app/amscreen.* $rfs_tmp/am7x/case/data
		fi
		if [ $ezwire_type -ne 0 -a $ezwire_type -ne 4 ]; then
			copy_file $Patch/snor/osd/osdIconPalette.plt $rfs_tmp/am7x/case/data/
			copy_file $Patch/snor/osd/OSD_RGB_Default.bin $rfs_tmp/am7x/case/data/
		fi
		
		delete_file $rfs_tmp/sbin/hostapd
		delete_file $rfs_tmp/sbin/iwconfig
		delete_file $rfs_tmp/sbin/iwlist
		delete_file $rfs_tmp/sbin/wpa_cli
		delete_file $rfs_tmp/sbin/hostapd_cli
		delete_file $rfs_tmp/sbin/iwevent
		delete_file $rfs_tmp/sbin/iwpriv
		delete_file $rfs_tmp/sbin/wpa_passphrase
		delete_file $rfs_tmp/sbin/iwgetid
		delete_file $rfs_tmp/sbin/iwspy
		delete_file $rfs_tmp/sbin/wpa_supplicant
		delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_it6681.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/at24_i2c.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/at25.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/g_subdisplay.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/i2c_wm8988.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/i2s_i2c.ko
		delete_file $rfs_tmp/lib/modules/2.6.27.29/keenhi_rfkey.ko
		delete_file $rfs_tmp/am7x/bin/miracast.app
		delete_file $rfs_tmp/am7x/bin/rs232_main.app
	fi

	if [ $ezcastpro_flag -eq 8075 -o $ezcastpro_flag -eq 8074 ]; then
		Patch=$TOPDIR/Flash/EZCast/Patch
		copy_file $Patch/adb_stream/adb $rfs_tmp/usr/bin/
		copy_file $Patch/adb_stream/adbForward.app $rfs_tmp/usr/bin/
		copy_file $Patch/adb_stream/getDacTime.app $rfs_tmp/usr/bin/
		copy_file $Patch/app/amscreen.* $rfs_tmp/am7x/case/data
	fi
	
	delete_file $rfs_tmp/am7x/case/data/main_customize_mirawire.swf
	delete_file $rfs_tmp/am7x/case/data/ezwire_bg_*.swf
	delete_file $rfs_tmp/am7x/case/data/ezwire_logo_*.swf

	delete_file $rfs_tmp/usr/share/ezdata/wqy-microhei.ttc
	delete_file $rfs_tmp/usr/share/ezdata/background*

	if [ $flash_type -eq 1 ];then
		snor_16m_rm
	else
		delete_file $rfs_tmp/am7x/case/data/*_smaller.swf
	fi
	if [ $flash_type -eq 2 ];then
		ezwire_8m_or_4m_rm
	fi
	if [ $usb1_disable -eq 1 ];then
               delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_uoc_next.ko     
               delete_file $rfs_tmp/lib/modules/2.6.27.29/am7x_hcd_next.ko 
	fi

	chmod a+x $rfs_tmp/usr/share/udhcpc/default.script

	strip_file $rfs_tmp/sbin/hostapd
	strip_file $rfs_tmp/sbin/wpa_supplicant
	strip_file $rfs_tmp/am7x/bin/fui.app
	strip_file $rfs_tmp/sbin/dnsmasq
	strip_file $rfs_tmp/am7x/bin/miracast.app
	strip_file $rfs_tmp/sbin/wpa_cli
	strip_file $rfs_tmp/sbin/thttpd
	strip_file $rfs_tmp/sbin/xtables-multi
	strip_file $rfs_tmp/sbin/brctl
	strip_file $rfs_tmp/sbin/iwlist
	strip_file $rfs_tmp/usr/share/thttpd/html/cgi-bin/websetting.cgi

	strip_file $rfs_tmp/am7x/lib/*.so*
	strip_file $rfs_tmp/lib/*.so*
	strip_file $rfs_tmp/usr/lib/*.so*

	if [ $config_3to1 -eq 1 ];then
		delete_file $rfs_tmp/am7x/case/data/ezcast.swf
		delete_file $rfs_tmp/am7x/case/data/ezcast_background.swf
		delete_file $rfs_tmp/am7x/case/scripts/eth_usb.sh
	fi

	
#	sh lsdir.sh $rfs_tmp/ | sort -h > $TOPDIR/scripts/rootfs_file.txt
}

customer_user1_rm()
{
        echo "#####################################################"
        echo "#               Product patch user1                 #"
        echo "#####################################################"
	rfs_tmp=$TOPDIR/scripts/rfstmp
	Patch=$TOPDIR/Flash/EZCast/Patch
	echo "rfs_tmp: "$rfs_tmp

	if [ $ezcast_flag -ne 0 -a $ezmusic_flag -eq 0 -a $ezcastpro_flag -eq 0 ]; then
		#echo ">>>>>>> Remove all html!!!"
		#mv $rfs_tmp/thttpd/html/index.html $rfs_tmp/thttpd/html/index.backup
		#mv $rfs_tmp/thttpd/html/ezcastmain.html $rfs_tmp/thttpd/html/ezcastmain.backup
		#find $rfs_tmp/thttpd/html/ -name '*.html' -depth| xargs rm -rf;
		#mv $rfs_tmp/thttpd/html/index.backup $rfs_tmp/thttpd/html/index.html
		#mv $rfs_tmp/thttpd/html/ezcastmain.backup $rfs_tmp/thttpd/html/ezcastmain.html
		rm -rf $rfs_tmp/thttpd/html/img_rs232
		find $rfs_tmp/thttpd/html/ -name 'upload.html' -depth| xargs rm -rf;
	fi

	if [ $ezwire_flag -ne 0 ]; then
		delete_file $rfs_tmp/ezdata/background*.jpg
		
		if [ $flash_type -eq 1 -o $flash_type -eq 2 ];then
			cd $rfs_tmp/
			rm ./* -rf
			mkdir user1
			mkdir ./user1/system_setting
			echo "MY_DPF" > ./user1/system_setting/hostname.dat
			cd $rfs_tmp/user1/
			ln -s /usr/share/thttpd ./thttpd
			ln -s /usr/share/ezdata ./ezdata
		fi
		
		if [ $ezwire_type -eq 0 -o $ezwire_type -eq 4 ];then
			cp -dprf $Patch/webdownload/dwnl $rfs_tmp/thttpd/html/
			chmod 644 $rfs_tmp/thttpd/html/dwnl/*
			copy_file $Patch/webdownload/Apps.html $rfs_tmp/thttpd/html/
			chmod 644 $rfs_tmp/thttpd/html/Apps.html
			cd $rfs_tmp/thttpd/html/
			ln -s Apps.html app
			cp -dprf $Patch/webdownload/apps $rfs_tmp/thttpd/html/img/
		fi
	fi
    
    # app theme pack
    if [ $ezcastpro_apptheme_flag -ne 0 ]; then
        apptheme_pack
    fi
    
	cd $TOPDIR/scripts
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
		ezcast_dir="../case/images/ezcast"
		rm -rf $rfs_tmp;
		mkdir $rfs_tmp;
		cp -dprf $fs_source/* $rfs_tmp;
		if  [ -n "$input_arg" ];then
			if [ "$input_arg" != "realtek" ];then
				if [ "$input_arg" != "ezcastpro" ];then
				find $rfs_tmp -name '8192cu.ko' | xargs rm -rf;
				find $rfs_tmp -name '8192du.ko' | xargs rm -rf;
				find $rfs_tmp -name '8188eu.ko' | xargs rm -rf;
				find $rfs_tmp -name '8192eu.ko' | xargs rm -rf;
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
				if [ $config_3to1 -eq 1 ];then
					copy_file $TOPDIR/Flash/MiraScreen_Free/patch/default.script $rfs_tmp/usr/share/udhcpc
					copy_file $TOPDIR/Flash/MiraScreen_Free/patch/ezcast_icon_setting.png $rfs_tmp/usr/share/ezdata/
				else
					copy_file $patch_path/default.script $rfs_tmp/usr/share/udhcpc
				fi

                                # Store server build time as system default time,
                                # Chromecast needs actual time to work.
                                if [ $chromecast_flag -eq 1 ]; then
                                    sed -i -e "s/#\/usr\/bin\/sync_ntp_time\.sh/\/usr\/bin\/sync_ntp_time\.sh/g" $rfs_tmp/usr/share/udhcpc/default.script
                                    date -u +%Y%m%d%H%M.%S > $rfs_tmp/usr/share/date_default.txt
                                    #date -u +%Y%m%d%H%M.%S > $rfs_tmp/mnt/vram/date.txt
                                fi
			#fi
			if [ "$input_arg" != "ezcastpro" ];then
				cp $ezcast_dir/udhcpd_01.conf $rfs_tmp/etc/ -dprf
				cp $ezcast_dir/udhcpd_nodns_01.conf $rfs_tmp/etc/ -dprf
				cp $ezcast_dir/udhcpd.conf $rfs_tmp/etc/ -dprf
				cp $TOPDIR/case/lib/libamsocket.so  $rfs_tmp/usr/lib/
			fi
			cp $rfs_tmp/etc/version.conf $ezcast_dir
			find $rfs_tmp -name '.svn' | xargs rm -rf;
			find $rfs_tmp -name 'libnsl*' -depth| xargs rm -rf ;
			#find $rfs_tmp -name 'libnss*' -depth| xargs rm -rf ;libstream.so
			find $rfs_tmp -name 'am7x_tp.ko*' -depth| xargs rm -rf ;
			#find $rfs_tmp -name 'libxt*' -depth| xargs rm -rf;
#			find $rfs_tmp -name 'am7x_*card*.ko' -depth| xargs rm -rf ;
			find $rfs_tmp -name 'rtc*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name 'dbus*' -depth| xargs rm -rf;
			find $rfs_tmp -name 'lib_id_tif.so' -depth| xargs rm -rf;
			find $rfs_tmp -name 'lib_id_gif.so' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'libresolv*' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'xtables*' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'config*' -depth| xargs rm -rf;
			if [ $cloud_control_flag -eq 0 ]; then
				find $rfs_tmp -name 'ca-certificates.crt' -depth| xargs rm -rf;
				find $rfs_tmp -name 'libwebsockets.so*' -depth| xargs rm -rf;
			fi
			if [ $ezchannel_js_flag -eq 0 ]; then
				find $rfs_tmp -name 'libduktape.so*' -depth| xargs rm -rf;
			fi
			if [ $ezwifi_mms_flag -eq 0 ]; then
				find $rfs_tmp -name 'libmms.so*' -depth| xargs rm -rf;
			fi
			if [ $ezwifi_rtmp_flag -eq 0 ]; then
				find $rfs_tmp -name 'librtmp.so*' -depth| xargs rm -rf;
			fi
			if [ $chromecast_flag -eq 0 ]; then
				find $rfs_tmp -name 'libChromecastSDK.so*' -depth| xargs rm -rf;
				find $rfs_tmp -name 'libChromecastSDK_HW.so*' -depth| xargs rm -rf;
			fi
			find $rfs_tmp -name 'hostapd' |grep -e "local"| xargs rm -rf;
			find $rfs_tmp -name '*spi*.ko' -depth| xargs rm -rf;
			if [ $ezcastpro_flag -ne 8075 ];then 
 				find $rfs_tmp -name '*hid.ko' -depth| xargs rm -rf;
			fi
			find $rfs_tmp -name '*mouse*.ko' -depth| xargs rm -rf;
			if [ "$input_arg" != "ezcastpro" ];then
				find $rfs_tmp -name '*fuser*' -depth| xargs rm -rf;
				find $rfs_tmp -name 'fuse.ko' -depth| xargs rm -rf;
				#find $rfs_tmp -name 'g_file*.ko' -depth| xargs rm -rf;
				find $rfs_tmp -name 'libfv.so' -depth| xargs rm -rf;
			fi
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
			#find $rfs_tmp -name 'keymap' -depth| xargs rm -rf;
			#find $rfs_tmp -name 'case/*.bin' -depth| xargs rm -rf;
			#find $rfs_tmp -name '*key*' -depth| xargs rm -rf;
			#find $rfs_tmp -name '*net*' -depth| xargs rm -rf;
			#find $rfs_tmp -name '*at2*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name '*scsi*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name '*ads*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name '*dpp*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name 'am7x_battery.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name 'al1601.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name '*axp*.ko' -depth| xargs rm -rf;
			find $rfs_tmp -name 'am7x_upgrade.ko' -depth| xargs rm -rf;
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
			if [ $ezcastpro128_flag -eq 1 ];then 
 				rm -rf  $rfs_tmp/mnt/atop/;
 			fi
			sh lsdir.sh $rfs_tmp/ | sort -h > $TOPDIR/scripts/rootfs_file.txt && echo "Save rootfs file list information success." || echo "Can not save file list information."
			init_size=$(du $rfs_tmp | awk '$1{total=$1}END{print (total+2560)*1024}')
			if [ "$input_arg" == "ezcastpro" ];then
				cp $TOPDIR/case/lib/libamsocket.so  $rfs_tmp/usr/lib/
				cp $ezcast_dir/udhcpd_ezcastpro.conf $rfs_tmp/etc/udhcpd_01.conf -f			
				cp $ezcast_dir/udhcpd_ezcastpro.conf $rfs_tmp/etc/udhcpd.conf -f			
				cp $ezcast_dir/udhcpd_ezcastpro_nodns_01.conf $rfs_tmp/etc/udhcpd_nodns_01.conf -dprf				
#				find $rfs_tmp -name 'g_file*.ko' -depth| xargs rm -rf;
				if [ $ezcastpro128_flag -eq 1 ];then 
					echo "128M ezprojector  done";
				elif [ $ezcastpro_flag -eq 8075 -o  $ezcastpro_flag -eq 8074 ];then 
 					cp -f $SDK_DIR/cdrom/*.iso $rfs_tmp/mnt/cdrom/
					init_size=$(($init_size+5242880+8*1024*1024));
				elif [	$ezcastpro_flag -eq 8251 -a $ezwilan_flag -eq 0 ];then
					find $rfs_tmp -name '*fuse*' -depth| xargs rm -rf;
					find $rfs_tmp -name 'g_file*.ko' -depth| xargs rm -rf;
					init_size=$(($init_size+5242880));
				fi

                #Mos: enable syslogd in ezcastpro series, to clarify thttpd close issue
                echo "#Mos: enable syslogd in ezcastpro series, to clarify thttpd close issue" >> $rfs_tmp/etc/init.d/rcS
                echo "syslogd -s 1024" >> $rfs_tmp/etc/init.d/rcS
				
				echo "mkfs ezcastpro done"
			fi				
		else
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
#				find $rfs_tmp -name 'wqy-microhei.ttc' -depth| xargs rm -rf;
#				find $rfs_tmp -name 'background*' -depth| xargs rm -rf;
				delete_file $rfs_tmp/ezdata/wqy-microhei.ttc
				delete_file $rfs_tmp/ezdata/background*
				if [ "$input_arg" == "ezcastpro" ];then
					cp $ezcast_dir/udhcpd_nodns_01.conf $rfs_tmp/softap/ -dprf
					echo "use ezcastpro udhcpd_01.conf"
					if [	$ezcastpro_flag -eq 8251 -a $ezwilan_flag -eq 0 ];then
						cp $ezcast_dir/udhcpd_ezcastpro.conf $rfs_tmp/softap/udhcpd_01.conf -dprf
					else
						cp $ezcast_dir/udhcpd_ezcastpro.conf $rfs_tmp/softap/ -dprf
					fi
				fi
			fi
			
			customer_user1_rm
			
			if [ -d $rfs_tmp/thttpd/html/ ];then
				find $rfs_tmp/thttpd/html/ -name '*.html' | xargs chmod 644
				find $rfs_tmp/thttpd/html/ -name '*.js' | xargs chmod 644
				find $rfs_tmp/thttpd/html/ -name '*.png' | xargs chmod 644
				find $rfs_tmp/thttpd/html/ -name '*.jpg' | xargs chmod 644
				find $rfs_tmp/thttpd/html/ -name '*.css' | xargs chmod 644
				find $rfs_tmp/thttpd/html/ -name '*.gif' | xargs chmod 644
				find $rfs_tmp/thttpd/html/ -name '*.svg' | xargs chmod 755
			fi
			if [ -d $rfs_tmp/thttpd/html/cgi-bin/ ];then
				find $rfs_tmp/thttpd/html/cgi-bin/ -name '*.cgi' | xargs chmod 755
			fi
			if [ $ezcastpro128_flag -eq 1 ];then 
				echo ">>>>>>>>>>>>>>>rm html"
				#rm -rf $rfs_tmp/thttpd/html/
				cd $rfs_tmp/thttpd/html/
				ls | grep -v airusb | xargs rm -rf
				cd -
			fi
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
	cpio|romfs|squashfs) ;;
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
	squashfs)
		rm -rf $MNT_DIR/*
		cp -dprf $rfs_tmp/* $MNT_DIR;
		if [ "$fs_image" = "rootfs.img" ];
		then
			do_module_process $TOPDIR/scripts/mconfig.h 0 ${MNT_DIR}
			sudo cp /dev/console $MNT_DIR/dev/ -dprf
			ls $MNT_DIR/dev/ -l
		fi

		./mksquashfs $MNT_DIR $fs_image

		rm -rf $MNT_DIR/*
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

ezcast_version(){
#echo "Get minor version"
	minor_file=$TOPDIR/case/images/ezcast/Minor_Version
	if [ ! -f "$minor_file" ]; then
		echo "Minor_Version=000" > $minor_file
	fi
	_minor_version=`cat $minor_file | grep Minor_Version= | cut -d= -f2` && echo "_minor_version="$_minor_version || _minor_version="000"
	echo $_minor_version > ./minor_version
	__minor_version=`sed 's/.*\(...\)/\1/' ./minor_version | awk '{printf("%03d", $1)}'`
	echo "__minor_version="$__minor_version

#echo "Get major version"	
	major_file=$TOPDIR/scripts/Major_Version
	version_conf=$SDK_DIR/rootfs/etc/version.conf
	tmp_conf=./version_new.conf
	__major_version=`cat $major_file | grep Major_Version= | cut -d= -f2` && echo "__major_version="$__major_version || __major_version=`cat ./version_cfg.h | grep VERSION_FIRMWARE | awk '{printf($3)}'`
	echo "__major_version: "$__major_version
	
	new_version=$__major_version$__minor_version
	echo "  ########## New version is: "$new_version
	if [ -f $tmp_conf ]; then
		rm $tmp_conf
	fi
	if [ -f $version_conf ]; then
		cat $version_conf | while read line
		do
			if echo "$line" | grep -q "FIRMWARE"
			then
				echo "FIRMWARE = $new_version" >> $tmp_conf
			else
				echo $line >> $tmp_conf
			fi
		done
		mv $tmp_conf $version_conf
		cp $version_conf ./version.conf
		cp $version_conf ../case/images/ezcast/version.conf
		echo ""
		echo ">>>>> $version_conf"
		cat $version_conf
		echo "<<<<<<"
		echo ""
	fi
}

ezcastpro_version()
{
	source ./get_git_version.sh
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
probox_thttpd_port_changge()
{
	bak_dir=$SDK_DIR/bak	
	sed -i -e "s/port=.*/port=80/" $bak_dir/thttpd.conf
	cp -f  $bak_dir/thttpd.conf $SDK_DIR/rootfs/etc/thttpd.conf
	cp -f  $bak_dir/thttpd.conf $SDK_DIR/user1/softap/thttpd.conf 
}
ezcast_file()
{
	echo ">>>>>>>>>>>>>>>>>> ezcast file copy"

	rootfs_dir=$SDK_DIR/rootfs
	data_dir=$rootfs_dir/am7x/case/data
	images_dir=$TOPDIR/case/images
	ezcast_dir=$images_dir/ezcast
	ezwire_dir=$images_dir/ezwire

    #Mos: Add build information into version.conf to tracking creator
    ver_file=./version.conf
    echo BUILD_HASH = `git rev-parse --short HEAD` >> $ver_file
    echo BUILD_BRANCH = `git rev-parse --abbrev-ref HEAD` >> $ver_file
    echo BUILD_USER = $USER >> $ver_file

	if [ "$input_arg" = "ezcastpro" ];then
		ezcastpro_version
	else
		ezcast_version
	fi

	rfs_tmp=$TOPDIR/scripts/rfstmp
	MC_FILE=$TOPDIR/scripts/mconfig.h
	if [ $ezcastpro_flag -eq 8075 ]; then
		echo ">>>>>>ezcastpro_flag:"$ezcastpro_flag "save8189.ko"
		Patch5G_Path=$TOPDIR/Flash/EZCastPro/ProBox
		echo "ProBox cp rtl_hostapd_01.Config"
		sed -i -e 's/max_num_sta=.*/max_num_sta=16/' $Patch5G_Path/rtl_hostapd_01.conf
		sed -i -e 's/max_num_sta=.*/max_num_sta=16/' $Patch5G_Path/rtl_hostapd_02.conf
		cp -dprf $Patch5G_Path/rtl_hostapd_01.conf $SDK_DIR/user1/softap/
		cp -dprf $Patch5G_Path/rtl_hostapd_01.conf $SDK_DIR/bak/
		cp -dprf $Patch5G_Path/rtl_hostapd_02.conf $SDK_DIR/user1/softap/
		cp -dprf $Patch5G_Path/rtl_hostapd_02.conf $SDK_DIR/bak/
		cp -dprf $Patch5G_Path/udhcpd_02.conf $SDK_DIR/bak/ 
		probox_thttpd_port_changge
	else
		delete_file $TOPDIR/sdk/library/net/rtl8189ES_linux_v4.3.10.1_13373.20150129/8189es.ko
	fi

	if [ $ezwire_flag -eq 1 -a $ezwire_type -ne 0 -a $ezwire_type -ne 4 ]; then
		copy_file  $ezcast_dir/welcome_mirascreen.bin $ezcast_dir/welcome.bin
	elif [ $ezcastpro_flag -eq 0 ]; then
		copy_file  $ezcast_dir/welcome_ezcast.bin $ezcast_dir/welcome.bin
	fi
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

scripts_path=$TOPDIR/scripts
MC_FILE=$scripts_path/mconfig.h
EZWIFI_CONFIG_FILE=$scripts_path/ezwifi_config.h  

ezcast_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZCAST_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZCAST_ENABLE | awk '{print $3}' || echo 0`
ezcastpro_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZCASTPRO_MODE && cat mconfig.h | grep MODULE_CONFIG_EZCASTPRO_MODE | awk '{print $3}' || echo 0`
ezcastpro_apptheme_flag=`cat $EZWIFI_CONFIG_FILE | grep EZWIFI_CONFIG_APP_THEME | awk '{print $3}' || echo 0`
ezcastpro128_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZCASTPRO128_MODE && cat mconfig.h | grep MODULE_CONFIG_EZCASTPRO128_MODE | awk '{print $3}' || echo 0`
ezmusic_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZMUSIC_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZMUSIC_ENABLE | awk '{print $3}' || echo 0`
ezwilan_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZWILAN_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZWILAN_ENABLE | awk '{print $3}' || echo 0`
ezcast5g_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZCAST5G_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZCAST5G_ENABLE | awk '{print $3}' || echo 0`
mirascreent5g_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_MIRASCREEN5G_ENABLE && cat mconfig.h | grep MODULE_CONFIG_MIRASCREEN5G_ENABLE | awk '{print $3}' || echo 0`
ezwire_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZWIRE_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZWIRE_ENABLE | awk '{print $3}' || echo 0`
ezwire_type=`cat $MC_FILE | grep -q MODULE_CONFIG_EZWIRE_TYPE && cat mconfig.h | grep MODULE_CONFIG_EZWIRE_TYPE | awk '{print $3}' || echo 0`
flash_type=`cat $MC_FILE | grep -q MODULE_CONFIG_FLASH_TYPE && cat mconfig.h | grep MODULE_CONFIG_FLASH_TYPE | awk '{print $3}' || echo 0`
usb1_disable=`cat $MC_FILE | grep -q MODULE_CONFIG_USB1_DISABLE && cat mconfig.h | grep MODULE_CONFIG_USB1_DISABLE | awk '{print $3}' || echo 0`
adb_mirroronly=`cat $MC_FILE | grep -q MODULE_CONFIG_ADB_MIRROR_ONLY && cat mconfig.h | grep MODULE_CONFIG_ADB_MIRROR_ONLY | awk '{print $3}' || echo 0`
cloud_control_flag=`cat $EZWIFI_CONFIG_FILE | grep -w EZWIFI_CONFIG_CLOUD_CONTROL | awk '{print $3}' || echo 0`
ezchannel_js_flag=`cat $EZWIFI_CONFIG_FILE | grep -w EZWIFI_CONFIG_EZCHANNEL_JS | awk '{print $3}' || echo 0`
ezwifi_mms_flag=`cat $EZWIFI_CONFIG_FILE | grep -w EZWIFI_CONFIG_MMSX_STREAMING | awk '{print $3}' || echo 0`
ezwifi_rtmp_flag=`cat $EZWIFI_CONFIG_FILE | grep -w EZWIFI_CONFIG_RTMP_STREAMING | awk '{print $3}' || echo 0`
chromecast_flag=`cat $MC_FILE | grep -w MODULE_CONFIG_CHROMECAST | awk '{print $3}' || echo 0`
adui_enable=`cat $MC_FILE | grep -q MODULE_CONFIG_AD_UI_ENABLE && cat mconfig.h | grep MODULE_CONFIG_AD_UI_ENABLE | awk '{print $3}' || echo 0`
config_3to1=`cat $MC_FILE | grep -q MODULE_CONFIG_3TO1 && cat mconfig.h | grep MODULE_CONFIG_3TO1 | awk '{print $3}' || echo 0`
chip_type=`cat $MC_FILE | grep -q MODULE_CONFIG_CHIP_TYPE && cat mconfig.h | grep MODULE_CONFIG_CHIP_TYPE | awk '{print $3}' || echo 0`


echo "MC_FILE: "$MC_FILE
echo "ezcast_flag: "$ezcast_flag
echo "ezcastpro_flag: "$ezcastpro_flag
echo "ezcastpro_apptheme_flag: "$ezcastpro_apptheme_flag
echo "ezcastpro128_flag: "$ezcastpro128_flag
echo "ezmusic_flag: "$ezmusic_flag
echo "ezwilan_flag: "$ezwilan_flag
echo "ezcast5g_flag: "$ezcast5g_flag
echo "mirascreent5g_flag: "$mirascreent5g_flag
echo "ezwire_flag: "$ezwire_flag
echo "ezwire_type: "$ezwire_type
echo "flash_type: "$flash_type
echo "usb1_disable: "$usb1_disable
echo "adb_mirroronly: "$adb_mirroronly
echo "cloud_control_flag: "$cloud_control_flag
echo "ezchannel_js_flag: "$ezchannel_js_flag
echo "ezwifi_mms_flag: "$ezwifi_mms_flag
echo "ezwifi_rtmp_flag: "$ezwifi_rtmp_flag
echo "chromecast_flag: "$chromecast_flag
echo "adui_enable: "$adui_enable
echo "config_3to1: "$config_3to1
echo "chip_type: "$chip_type

if [ $chip_type -ge 10 ];then
	chip_major=`expr ${chip_type} / 10`
else
	chip_major=0
fi
echo "chip_major: "$chip_major

uid=$(id -u)
gid=$(id -g)
mkfs_flags_ntfs='-F'
alias cp=$(which cp)
alias mv=$(which mv)

do_modules_install

if [ "$input_arg" = "realtek" -o "$input_arg" = "ralink" -o "$input_arg" = "ezwire" -o "$input_arg" = "ezcastpro" ];then
	ezcast_file
	if [ $ezcastpro_flag -eq 8075 ];then
		CFG_FILE=AM_FS_EZPROBOX.cfg		
	elif [ $ezcastpro128_flag -eq 1 ];then
		CFG_FILE=AM_FS_EZProjecitor_128.cfg	
	elif [ $flash_type -eq 1 -o $flash_type -eq 2 ];then
		CFG_FILE=AM_FS_16M.cfg		

		echo "######################################"
		echo "#         To build squashfs          #"
		echo "######################################"

		cd $SDK_DIR/tools/squashfs3.4
		./buildTools.sh
		cd -

		echo "######################################"
		echo "#      build squashfs complete       #"
		echo "######################################"
	elif [ $ezwire_flag -ne 0 ];then
		CFG_FILE=AM_FS_EZWire.cfg		
	else
		CFG_FILE=AM_FS_EZCAST.cfg
	fi
	echo "Config to EZCast!!!"
else
	CFG_FILE=AM_FS.cfg
fi
if [ "$input_arg" = "ezcastpro" -a  $ezcastpro_flag -ne 8075 ];then
	ezcastpro_softap_add_user
fi
echo ">>>>>>>> CFG_FILE: "$CFG_FILE
awk -F'#' '$1{print $1}' $CFG_FILE | while read args
do
	echo ">>>>>>> args: "$args
	make_fs $args
done


