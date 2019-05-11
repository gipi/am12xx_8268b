#!/bin/sh
#
# This program is a patch to build a EZCast firmware.
# Before run this patch, please run linuxtool.exe and export configs.
# If the project is am8252, please run this program by a parameter "am8252".
# If you only need to copy swf to data, please run this program by a parameter "am8252_copy"

source ./get_git_version.sh

am8252_copy_swf()
{
	source=$TOPDIR/Flash/AM8252/SWF 
	target=$TOPDIR/case/images/ezcast/data
	echo $source" >>>> "$target
	cp $source/*.swf $target/				
	#  copy swf */
}

function linuxtool_config()
{
	if [ $# -lt 3 ];then
		echo "#########[linuxtool config] Have not enough parameter!!! ##########"
		return 1
	fi
	file=$1
	item=$2
	new_char=$3
	if [ ! -f $file ];then
		echo "######## "$file" is not a file  ########"
		return 1
	fi
	echo "#  linuxtool config patch	"
	echo "#  file: "$file
	echo "#  item: "$item
	echo "#  new_char: "$new_char
	old_val=`cat $file | grep $item | cut -d[ -f2 | cut -d] -f1`
	old=$item"\["$old_val"\]"
	new=$item"\["$new_char"\]"
	sed -i 's/'$old'/'$new'/g' $file
}

am8252_head_config()
{
	path=$TOPDIR/scripts
	file=$path/ezwifi_config.h
	tmp_head=$TOPDIR/heal.h
	tmp_file=$TOPDIR/tmp
	echo "#  head patch"
	echo "#  file: "$file
	if [ -f $file ]; then
		cat $file | while read line
		do
			ret=0
			if echo "$line" | grep -q "EZWIFI_CONFIG_MODEL_NAME"
			then
				ret=1
			fi
			if echo "$line" | grep -q "EZWIFI_CONFIG_AIRPLAY_MIRROR"
			then
				ret=1
			fi
			if [ $ret -eq 1 ]; then
				ret=0
				val=0
				item=EZWIFI_CONFIG_AIRPLAY_MIRROR
				if echo "$line" | grep -q "EZWIFI_CONFIG_MODEL_NAME"
				then
					val=\"ezcast-lite\"
					item=EZWIFI_CONFIG_MODEL_NAME
				else
					val=0
					item=EZWIFI_CONFIG_AIRPLAY_MIRROR
				fi
				echo "## item: "$item
				echo $line > $tmp_file
				old=`cat $tmp_file | awk '{print $3}'`
				sed -i "s/$old/$val/g" $tmp_file
				cat $tmp_file | grep $item >> $tmp_head
				rm $tmp_file
			else
				echo $line >> $tmp_head
			fi
		done
		mv $tmp_head $file
		cat $file
	fi
}
function exchange_config()
{
	if [ $# -lt 3 ]; then
		echo "## Have not enough parameter!!"
		return 1
	fi
	
	file=$1
	item=$2
	val=$3
	tmp_file=./patch_tmp
	
	if [ ! -f $file ]; then
		echo "## "$file" is not a file"
		return 1
	fi

	cat $file | grep -n $item > $tmp_file
	cat $tmp_file | while read line
	do
		if echo "$line" | grep -q "$item"
		then
			l=`echo $line | cut -d: -f1`
			sed -i "$l s:.*:	{ $item,		$val},:g" $file
			echo "## line: "$l"  ile: "$file"  "$item" >>>> "$val
		fi
	done
	
	rm $tmp_file
	return 0
}

am8252_config()
{
	echo "######  path Version_8251_EZCast.txt ######"
	path=$TOPDIR/case/project_kit/project_qc/config
	file=$path/version/Version_8251_EZCast.txt
	linuxtool_config $file MODEL= ezcast_am8252

	echo "######  path ezconfig_qc_ezcast.txt ######"
	file=$path/ezwifi_config/ezconfig_qc_ezcast.txt
	linuxtool_config $file MODEL_NAME= \"ezcast-lite\"
	linuxtool_config $file AIRPLAY_MIRROR= 0

	echo "######  path ezwifi_config.h ######"
	am8252_head_config

	echo "######  patch ez_remoteconfig.h ######"
	file=$TOPDIR/sdk/library/wifi_subdisplay/ez_remoteconfig.h
	patch=$TOPDIR/Flash/AM8252/Patch/ez_remoteconfig.h
	echo $patch" >>>> "$file
	cp -dprf $patch $file
	
	#exchange_config $file EZREMOTE_SERVICE_PHOTO OS_BMP_NONE
	

}

patch_mkfs()
{
	file=$TOPDIR/scripts/mkfs.sh
	char=\*key\*
	if ! cat $file | grep $char | grep \#
	then
		echo "## Open key driver!!!"
		line=`cat $file | grep -n $char | cut -d: -f1`
		sed -i "$line s/^/#/" $file
	fi
	char=keymap
	if ! cat $file | grep $char | grep \#
	then
		echo "## Open keymap!!!"
		line=`cat $file | grep -n $char | cut -d: -f1`
		sed -i "$line s/^/#/" $file
	fi
}
patch_AM_FS_EZCAST_cfg()
{
	file=$TOPDIR/scripts/AM_FS_EZCAST.cfg
	char=cdrom
	if ! cat $file | grep $char | grep \#
	then
		echo "## remove cdrom.img!!!"
		line=`cat $file | grep -n $char | cut -d: -f1`
		sed -i "$line s/^/#/" $file
	fi

	file=$TOPDIR/scripts/AM_FS_EZCAST.cfg
	memory_config $file user1 12M 1 3
}
patch_am_syscfg_sh()
{
	file=$TOPDIR/case/project_kit/project_qc/batch/scripts/1213/am_syscfg.sh
	char=edid_i2c_hw
	if ! cat $file | grep $char | grep \#
	then
		echo "## rmmod edid_i2c_hw.ko!!!"
		line=`cat $file | grep -n $char | cut -d: -f1`
		sed -i "$line s/^/#/" $file
	fi
	
	char=edid_i2c_gpio
	if ! cat $file | grep $char | grep \#
	then
		echo "## rmmod edid_i2c_gpio.ko!!!"
		line=`cat $file | grep -n $char | cut -d: -f1`
		sed -i "$line s/^/#/" $file
	fi
	
	char=hdcp_i2c
	if ! cat $file | grep $char | grep \#
	then
		echo "## rmmod hdcp_i2c.ko!!!"
		line=`cat $file | grep -n $char | cut -d: -f1`
		sed -i "$line s/^/#/" $file
	fi

	char=am7x_lcm
	if ! cat $file | grep $char | grep \#
	then
		echo "## rmmod am7x_lcm.ko!!!"
		line=`cat $file | grep -n $char | cut -d: -f1`
		sed -i "$line s/^/#/" $file
	fi
}
open_card_driver_for_ezmusic()
{
	file=$TOPDIR/scripts/mkfs.sh
	newchar=\*card\*
	if ! cat $file | grep -n $newchar | grep \#
	then
		echo "## Open card driver!!!"
		line=`cat $file | grep -n $newchar | cut -d: -f1`
		sed -i "$line s/^/#/" $file
	fi
}
am8252_patch(){
	echo "#####################################################"
	echo "# 			   do am8252 patch	 				  #"
	echo "#####################################################"
	
	am8252_copy_swf
	am8252_config
	patch_mkfs
}
am8252n_patch()
{
	echo "it is 8252n patch"
	cp $TOPDIR/sdk/tools/linux_utils/e2fsprogs-1.42.8/mkfs.ext2 $TOPDIR/sdk/rootfs/sbin
	cp $TOPDIR/Flash/EZCast/Patch/build/do_config.sh $TOPDIR/sdk/library/alsa-lib/build/

}
check_patch_version()
{
	#cur_version=`svn info |grep "Last Changed Rev:" |awk   '{print $4}'`
	#old_version=`cat $PATCH_DIR/old_version`
	#echo "  ########## current version: "$cur_version
	#echo "  ########## old version: "$old_version
	#if [ $cur_version -eq $old_version ]; then
	#	echo "####### Same version!!!"
	#	return 0
	#else
	#	echo "####### Different version!!!"
	#	return 1
	#fi

	cmp $PATCH_DIR/logic_brec_layer.c $MTD_DIR/logic_brec_layer.c
	if [ $? -eq 0 ]; then
		return 0
	else
		return 1
	fi
}

do_patch_copy()
{
	cp -dprf $PATCH_DIR/logic_brec_layer.c $MTD_DIR/logic_brec_layer.c
	#svn info |grep "Last Changed Rev:" |awk   '{print $4}' > $PATCH_DIR/old_version
	echo "	########## "$PATCH_DIR"/logic_brec_layer.c >> "$MTD_DIR"/logic_brec_layer.c"
}

patch_code()
{
	check_patch_version
	if [ $? -ne 0 ]; then
		echo "  ########## Different: "$PATCH_DIR"/logic_brec_layer.c "$MTD_DIR"/logic_brec_layer.c"
		do_patch_copy
	else
		echo "  ########## Not need copy logic_brec_layer.c"
	fi
}

build_main_code()
{
	cd $TOPDIR/scripts
	pwd
	make clean
	
	if [ "$input_arg" = "projector" ]; then
	echo "for build projector"
	cat cfg/1213/config.linux.projector > $TOPDIR/sdk/linux/.config;
	fi
	
	make all
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "#                main code make error               #"
		echo "#####################################################"
		exit 1;
	fi
	if [ "$input_arg" =  "8268_ezcast" ];then
		cd $TOPDIR/sdk/library/hdcp/
		pwd
		make clean
		make
	fi
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "#                hdcp make error                  #"
		echo "#####################################################"
		exit 1;
	fi
	if [ "$input_arg" = "8252n_8M" -o "$input_arg" = "8252n_4M"  -o "$input_arg" = "8258_4M" -o "$input_arg" = "8258_4M_ADB_MirrorOnly" -o  "$input_arg" = "8258B_Wire" -o "$input_arg" = "8258N_Wire" -o "$input_arg" = "8268_4M" -o  "$input_arg" = "8268B_Wire" ]; then
		cd $TOPDIR/sdk/library/ezota
		pwd
		make clean
		make
	fi
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "#                ezota make error                  #"
		echo "#####################################################"
		exit 1;
	fi
	if [ "$input_arg" = "8252n_8M" -o "$input_arg" = "8252n_4M" -o "$input_arg" = "8258B_Wire" -o "$input_arg" = "8258N_Wire" -o "$input_arg" = "8258_4M" -o "$input_arg" = "8258_4M_ADB_MirrorOnly" -o "$input_arg" = "8268_4M" -o  "$input_arg" = "8268B_Wire" ]; then
		if [ $flash_type -ne 1 ];then
			cd $TOPDIR/case/apps/wireUi
			pwd
			make clean
			make
		fi
	fi
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "#                wireUi make error                  #"
		echo "#####################################################"
		exit 1;
	fi
	if [ "$input_arg" = "8252n_8M" -o "$input_arg" = "8252n_4M" -o "$input_arg" = "8258B_Wire" -o "$input_arg" = "8258N_Wire"  -o "$input_arg" = "8258_4M" -o "$input_arg" = "8258_4M_ADB_MirrorOnly" -o "$input_arg" = "8268_4M" -o  "$input_arg" = "8268B_Wire" ]; then
		cd $TOPDIR/case/apps/reset_test
		pwd
		make clean
		make
	fi
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "#                reset.app make error                  #"
		echo "#####################################################"
		exit 1;
	fi
}

build_bootloader()
{
	echo "#############################################################"
	echo "#                  build bootloader                         #"
	echo "#############################################################"
	cd $BOOTLOADER_DIR
	make clean
	if [ "$input_arg" = "projector" -o "$input_arg" = "probox" -o "$input_arg" = "prolanplus" ];then
		./am_sdk_config.sh AM_13
	elif [ "$input_arg" = "am8258" -o "$input_arg" = "8258B" -o "$input_arg" = "8258B_Wire" -o "$input_arg" = "8258N_Wire" -o "$input_arg" = "8258_4M" -o "$input_arg" = "8258_4M_ADB_MirrorOnly" ];then
		./am_sdk_config.sh AM_8258
	elif [ "$input_arg" = "8268_ezcast" -o "$input_arg" = "8268_4M" -o  "$input_arg" = "8268B_Wire" -o "$input_arg" = "8268B" ];then
		./am_sdk_config.sh AM_8268
	else
		./am_sdk_config.sh AM_8251
	fi
	make
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 			   bootloader make error 			  #"
		echo "#####################################################"
		exit 1;
	fi
	cd -
}

build_eeprom()
{
	echo "#############################################################"
	echo "#                  build eeprom                             #"
	echo "#############################################################"
	cd $EEPROM_DIR
	make clean
	if [ "$input_arg" = "probox" -o "$input_arg" = "prolanplus" -o "$input_arg" = "projector" ];then
		./am_sdk_config.sh AM_13
	else
		./am_sdk_config.sh AM_8251
	fi
	make
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 			   eeprom make error 			  #"
		echo "#####################################################"
		exit 1;
	fi
	cd -
}

build_cgi()
{
	echo "#############################################################"
	echo "#                    build cgi                              #"
	echo "#############################################################"
	cd $CGI_DIR
	make clean
	make release
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 			   cgi make error 	  		          #"
		echo "#####################################################"
		exit 1;
	fi
	cd -	
}
ezcastpro_build_snmp()
{
	echo "#############################################################"
	echo "#                    build snmp                           #"
	echo "#############################################################"
	cd $SNMP_DIR
	sh do_build.sh
	sh do_release.sh
        cd -
        test -e $SNMP_DIR/../install/net-snmp/bin && \
        echo "SNMP for EZCastPro done!!!" || exit 1
}

function do_wifi_build()
{
	if [ $# -lt 3 ]; then
		echo "## [do_wifi_build] Have not enough parameter!!!"
		return 1
	fi
	path=$1
	folder=$2
	gz_file=$3
	if [ -d $path/$folder -o -f $path/$folder ]; then
		rm -rf $path/$folder
		echo "  ########## remove "$path/$folder
	fi
	cd $path
	tar xf $gz_file
	cd $path/$folder
	make clean
	make 
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 	   	  WiFi driver make error 	  		      #"
		echo "#####################################################"
		exit 1;
	fi
	make install
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 			  WiFi driver make error 			  #"
		echo "#####################################################"
		exit 1;
	fi
	cd $TOPDIR/scripts
}

function build_wifi_driver()
{
	
	echo "#############################################################"
	echo "#              	  build wifi driver                       #"
	echo "#############################################################"
	if [ $# -lt 4 ]; then
		echo "## Have not enough parameter!!!"
		return 1
	fi
	path=$1
	folder=$2
	gz_file=$3
	ko_file=$4
	echo "##  path: "$path
	echo "##  folder: "$folder
	echo "##  gz_file: "$gz_file
	echo "##  ko_file: "$ko_file
	if [ ! -d $path/$folder ]; then
		echo "  ########## "$path/$folder" not exist!!!"
		do_wifi_build $path $folder $gz_file
	else
		DRIVER_DIR=$TOPDIR/sdk/bin/1213/rootfs
		if [ -f $path/$folder/$ko_file -a -f $DRIVER_DIR/$ko_file ]; then
			cmp $DRIVER_DIR/$ko_file $path/$folder/$ko_file
			if [ $? -ne 0 ]; then
				echo "  ########## Driver file different, and rebuild WiFi driver!!!"
				do_wifi_build $path $folder $gz_file
			else
				echo "  ########## WiFi driver have been built"
			fi
		else
			echo "  ########## Driver file not exist, and rebuild WiFi driver!!!"
			do_wifi_build $path $folder $gz_file
		fi
	fi
}

change_version()
{
	echo "#############################################################"
	echo "#                Write version                              #"
	echo "#############################################################"
	cd $VERSION_CFG_DIR
#	VERSION_SCRIPTS=`svnversion -c $TOPDIR/scripts |sed 's/^.*://' |sed 's/[A-Z]*$//'`
#	VERSION_CASE=`svnversion -c $TOPDIR/case |sed 's/^.*://' |sed 's/[A-Z]*$//'`
#	VERSION_SDK=`svnversion -c $TOPDIR/sdk |sed 's/^.*://' |sed 's/[A-Z]*$//'`
#	VERSION_FLASH=`svnversion -c $TOPDIR/Flash |sed 's/^.*://' |sed 's/[A-Z]*$//'`
#	echo "  ########## VERSION_SCRIPTS: "$VERSION_SCRIPTS", VERSION_CASE: "$VERSION_CASE", VERSION_SDK: "$VERSION_SDK", VERSION_FLASH: "$VERSION_FLASH
	
#	VERSION_SVN=$VERSION_CASE
#	if [ $VERSION_SCRIPTS -ge $VERSION_SVN ]; then
#		VERSION_SVN=$VERSION_SCRIPTS
#	fi
#	if [  $VERSION_SDK -ge $VERSION_SVN ]; then
#		VERSION_SVN=$VERSION_SDK
#	fi
#	if [  $VERSION_FLASH -ge $VERSION_SVN ]; then
#		VERSION_SVN=$VERSION_FLASH
#	fi

    VERSION_SVN=$(get_version_from_git)
	echo "  ########## VERSION_SVN: "$VERSION_SVN
	V_PATH=$TOPDIR/sdk/user1/thttpd/html/version
	git log --pretty=oneline | sed -n '1p;2p;3p;4p;5p;' | awk '{print $1}' > $V_PATH
	chmod 644 $V_PATH
	
	VERSION_CONF=`cat $TOPDIR/case/project_kit/project_qc/config/version/Version_8251_EZCast.txt | grep FIRMWARE= | cut -d[ -f2 | cut -d] -f1`
	echo "  ########## VERSION_CONF: "$VERSION_CONF
	if [ $VERSION_SVN -ge $VERSION_CONF ]; then
		VERSION_NEW=$VERSION_SVN
	else
		VERSION_NEW=$VERSION_CONF
	fi
	echo "  ########## New version is: "$VERSION_NEW
	if [ -f ./version.conf ]; then
		cat ./version.conf | while read line
		do
			if echo "$line" | grep -q "FIRMWARE"
			then
				echo "FIRMWARE = $VERSION_NEW" >> version_new.conf
			else
				echo $line >> version_new.conf
			fi
		done
		mv ./version_new.conf ./version.conf
		cat ./version.conf
	fi
	
	path=$TOPDIR/scripts
	file=$path/version_cfg.h
	tmp_head=$TOPDIR/heal.h
	tmp_file=$TOPDIR/tmp
	echo "#  head patch"
	echo "#  file: "$file
	if [ -f $file ]; then
		cat $file | while read line
		do
			ret=0
			if echo "$line" | grep -q "VERSION_FIRMWARE"
			then
				ret=1
			fi
			if [ "$input_arg" = "am8252" ]; then			
				if echo "$line" | grep -q "VERSION_MODEL"
				then
					ret=1
				fi
			fi
			if [ $ret -eq 1 ]; then
				ret=0
				val=$VERSION_NEW
				item=VERSION_FIRMWARE
				if [ "$input_arg" = "am8252" ]; then			
					if echo "$line" | grep -q "VERSION_MODEL"
					then
						val=ezcast_am8252
						item=VERSION_MODEL
					fi
				fi
				echo "## item: "$item
				echo $line > $tmp_file
				old=`cat $tmp_file | awk '{print $3}'`
				sed -i "s/$old/$val/g" $tmp_file
				cat $tmp_file | grep $item >> $tmp_head
				rm $tmp_file
			else
				echo $line >> $tmp_head
			fi
		done
		mv $tmp_head $file
		cat $file
	fi

	path=$TOPDIR/case/project_kit/project_qc/config
	file=$path/version/Version_8251_EZCast.txt
	linuxtool_config $file FIRMWARE= $VERSION_NEW
		
}

patch_ezwilan_nand()
{
 
	file=$TOPDIR/sdk/linux/drivers/mtd/am7x_nftl/Flash/common/createtbl/nand_id_tbl.c
	line=`cat $file | grep -n W29N01GV | cut -d: -f1` 
	echo "patch nand_id_table for EZWiLAN,line="$line
	if [ $line > 0 ]; 
	then
		sed -i "$line s/\/\///" $file
	fi
}


memory_config()
{
	if [ $# -lt 5 ];then
		echo "#########[ddr config] Have not enough parameter!!! ##########"
		return 1
	fi
	file=$1
	item=$2
	new_char=$3
	row=$4
	col=$5
	if [ ! -f $file ];then
		echo "######## "$file" is not a file  ########"
		return 1
	fi
	echo "#  ddr patch	"
	echo "#  file: "$file
	echo "#  item: "$item
	echo "#  new_char: "$new_char
	echo "#  row: "$row
	echo "#  col: "$col
	if [ $col -eq 0 ]; then
		old_val=`cat $file | grep $item |awk '{print $3}'`
	elif [ $col -eq -1 ]; then
		line=`cat $file| grep -nm1 $item|cut -d: -f1`	
		echo "#  line: "$line		
		old_val=`cat $file| grep -nm1 $item`
		echo "#  old_val: "$old_val	
		value_length=`echo $old_val | wc -c`
		echo "#  value_length: "$value_length
	else
		old_val=`cat $file | grep $item|sed -n ''"$row"'p'|awk '{print $'"$col"'}'| cut -d '*' -f1`  
	fi
	echo "#  old_val: "$old_val
	old=$old_val
	new=$new_char
	echo "#  old: "$old
	echo "#  new: "$new
	if [ $col -eq -1 ]; then
		if [ $value_length -le 10 ]; then
			sed -i  "$line s/^/$new/" $file
		fi
	else
		sed -i 's/'"$old"'/'"$new"'/g' $file
	fi
}
ezmusic_config()
{
	echo "######  ezmusic_config######"
	#path=$TOPDIR/case/project_kit/project_qc/config
	#file=$path/ezwifi_config/ezconfig_qc_ezcast.txt
	#linuxtool_config $file EZCAST_TYPE= music

	file=$TOPDIR/case/images/ezcast/bootarg.txt	
	memory_config $file reserved_mem= reserved_mem=18M 0 0

	#file=$TOPDIR/case/apps/fui/include/fui_common.h
	#ddr_config $file HEAP_SHARE_SIZE \(8 4 3 

	#file=$TOPDIR/case/apps/fui/include/video_engine.h
	#ddr_config $file _VIDEO_HEAP1_SIZE \(6 4 3 

	file=$TOPDIR/sdk/library/wifi_subdisplay/ez_decodeconfig.h
	memory_config $file MAX_INPUTBUFFER_SIZE \(1000000\) 3 3
	memory_config $file MAX_DECODEBUFFER_WIDTH \(320\) 3 3
	memory_config $file MAX_DECODEBUFFER_HEIGHT \(240\) 3 3
	memory_config $file 0, 6*1024*1024\+ 1 -1
}
ezmusic_remove_useless_file()
{
	echo "##############remove bin file for ezmusic###########"
	target=$TOPDIR/case/images/ezcast
	rm -rf $target/*.bin

	#echo "##############remove 8821au.ko for ezmusic###########"
	#target=$TOPDIR/sdk/bin/1213/rootfs
	#rm -rf $target/8821au.ko
	
	echo "##############remove useless swf###########"
	target=$TOPDIR/case/images/ezcast/data	
	rm -rf $target/miracast_lite.swf
	rm -rf $target/setting.swf
	rm -rf $target/connectPC.swf
	rm -rf $target/Main_storage_bar.swf
	rm -rf $target/Main_top_bar.swf  
	rm -rf $target/miracast.swf
	rm -rf $target/main_customize.swf
	rm -rf $target/ezcast_background.swf
	
	echo "##############remove useless img###########"
	target=$TOPDIR/sdk/user1/thttpd/html
	rm -rf $target/img_rs232  
	target=$TOPDIR/sdk/rootfs/usr/share/ezdata
	rm -rf $target/*.png
	rm -rf $target/*.jpg
}
ezmusic_patch()
{
	echo "#####################################################"
	echo "# 			   do ezmusic patch	 				  #"
	echo "#####################################################"
	ezmusic_config
	ezmusic_remove_useless_file
	open_card_driver_for_ezmusic
	patch_mkfs
	patch_AM_FS_EZCAST_cfg
	#patch_am_syscfg_sh
	#echo "######  patch ez_remoteconfig.h ######"
	#file=$TOPDIR/sdk/library/wifi_subdisplay/ez_remoteconfig.h
	#patch=$TOPDIR/Flash/EZmusic/Patch/ez_remoteconfig.h
	#echo $patch" >>>> "$file
	#cp -dprf $patch $file

	#echo "######  patch ez_decodeconfig.h ######"
	#file=$TOPDIR/sdk/library/wifi_subdisplay/ez_decodeconfig.h
	#patch=$TOPDIR/Flash/EZmusic/Patch/ez_decodeconfig.h
	#echo $patch" >>>> "$file
	#cp -dprf $patch $file
	
	echo "######  patch html ######"
	file=$TOPDIR/sdk/user1/thttpd/html/websetting_Vertical.html
	patch=$TOPDIR/Flash/EZmusic/Patch/websetting_Vertical.html
	echo $patch" >>>> "$file
	cp -dprf $patch $file

	file=$TOPDIR/sdk/user1/thttpd/html/websetting_Horizontal.html
	patch=$TOPDIR/Flash/EZmusic/Patch/websetting_Horizontal.html
	echo $patch" >>>> "$file
	cp -dprf $patch $file

	echo "######  copy warning tone for ezmusic ######"
	file=$TOPDIR/sdk/user1/
	patch=$TOPDIR/Flash/EZmusic/Patch/warningtone
	echo $patch" >>>> "$file
	cp -rf $patch $file

	file=$TOPDIR/sdk/linux/drivers/serial/am7x_uart.c
	patch=$TOPDIR/Flash/EZmusic/Patch/am7x_uart.c
	echo $patch" >>>> "$file
	cp -dprf $patch $file
	
	echo "######  EZMusic patch end ######"

}

input_arg=$1
arg_num=$#
WORK_DIR=$(cd $(dirname $0) && pwd)
TOPDIR=$(cd $WORK_DIR/.. && pwd)
MTD_DIR=$TOPDIR/sdk/linux/drivers/mtd/am7x_nftl/Flash/common/logic
PATCH_DIR=$TOPDIR/Flash/EZCast/Patch
BOOTLOADER_DIR=$TOPDIR/sdk/bootloader/nand/scripts
EEPROM_DIR=$TOPDIR/sdk/bootloader/eeprom/scripts
CGI_DIR=$TOPDIR/case/apps/cgi-bin
NET_DIR=$TOPDIR/sdk/library/net
RTL8188EU_DIR=$NET_DIR/rtl8188EUS_linux_v4.1.7_8310.20140521_beta
VERSION_CFG_DIR=$TOPDIR/sdk/rootfs/etc
SNMP_DIR=$TOPDIR/sdk/library/net-snmp/build

scripts_path=$TOPDIR/scripts
MC_FILE=$scripts_path/mconfig.h

ezcast_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZCAST_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZCAST_ENABLE | awk '{print $3}' || echo 0`
ezcastpro_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZCASTPRO_MODE && cat mconfig.h | grep MODULE_CONFIG_EZCASTPRO_MODE | awk '{print $3}' || echo 0`
ezcastpro128_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZCASTPRO128_MODE && cat mconfig.h | grep MODULE_CONFIG_EZCASTPRO128_MODE | awk '{print $3}' || echo 0`
ezmusic_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZMUSIC_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZMUSIC_ENABLE | awk '{print $3}' || echo 0`
ezwilan_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZWILAN_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZWILAN_ENABLE | awk '{print $3}' || echo 0`
ezcast5g_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZCAST5G_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZCAST5G_ENABLE | awk '{print $3}' || echo 0`
mirascreent5g_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_MIRASCREEN5G_ENABLE && cat mconfig.h | grep MODULE_CONFIG_MIRASCREEN5G_ENABLE | awk '{print $3}' || echo 0`
ezwire_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_EZWIRE_ENABLE && cat mconfig.h | grep MODULE_CONFIG_EZWIRE_ENABLE | awk '{print $3}' || echo 0`
ezwire_type=`cat $MC_FILE | grep -q MODULE_CONFIG_EZWIRE_TYPE && cat mconfig.h | grep MODULE_CONFIG_EZWIRE_TYPE | awk '{print $3}' || echo 0`
flash_type=`cat $MC_FILE | grep -q MODULE_CONFIG_FLASH_TYPE && cat mconfig.h | grep MODULE_CONFIG_FLASH_TYPE | awk '{print $3}' || echo 0`
config_3to1=`cat $MC_FILE | grep -q MODULE_CONFIG_3TO1 && cat mconfig.h | grep MODULE_CONFIG_3TO1 | awk '{print $3}' || echo 0`

echo "MC_FILE: "$MC_FILE
echo "ezcast_flag: "$ezcast_flag
echo "ezcastpro_flag: "$ezcastpro_flag
echo "ezmusic_flag: "$ezmusic_flag
echo "ezwilan_flag: "$ezwilan_flag
echo "ezcast5g_flag: "$ezcast5g_flag
echo "mirascreent5g_flag: "$mirascreent5g_flag
echo "ezwire_flag: "$ezwire_flag
echo "ezwire_type: "$ezwire_type
echo "flash_type: "$flash_type
echo "config_3to1: "$config_3to1

do_main()
{
	echo "######################################################"
	echo "#                    EZCast patch                    #"
	echo "######################################################"
	echo "TOPDIR: "$TOPDIR
	echo "input_arg number: "$arg_num
	echo "input_arg: "$input_arg
	
	if [ "$input_arg" = "am8252_copy" ]; then
		echo "  ########## Only copy SWF file!!!"
		am8252_copy_swf
		return 0
	fi
	if [ ! "$input_arg" = "manual" ]; then
		if [ "$input_arg" != "projector" -a "$input_arg" != "probox" -a "$input_arg" !=  "prolan" -a "$input_arg" !=  "prodongle" -a "$input_arg" != "prolanplus" ]; then
			echo "  ########## Auto patch!!!"
			patch_code
		fi
	fi
	if [ "$input_arg" = "am8252" ]; then
		echo "########## am8252!!!"
		am8252_patch
	fi
	if [ "$input_arg" = "ezmusic" ]; then
		echo "########## ezmusic!!!"
		ezmusic_patch
	fi
	#if [ "$input_arg" = "ezwilan" ]; then
		#patch_ezwilan_nand
	#fi
	if [  "$input_arg" = "8258_4M_ADB_MirrorOnly" ]; then
		sh build_4M_Patch_ADB_MirrorOnly.sh
	fi
	if [  "$input_arg" = "8258_4M" -o "$input_arg" = "8252n_4M" -o "$input_arg" = "8268_4M" ]; then
		sh build_4M_patch.sh
	fi
	if [  "$input_arg" = "8252n_8M" -o "$input_arg" = "8258B_Wire" -o "$input_arg" = "8258N_Wire" -o  "$input_arg" = "8268B_Wire" ]; then
		if [ $flash_type -ne 1 ];then
			sh build_8M_patch.sh
		fi
	fi
	if [  "$input_arg" = "8252n" -o "$input_arg" = "8258B" -o "$input_arg" = "8268B" ]; then
		echo "  ##########8252n patch!!!"
		am8252n_patch
	fi
	cd $TOPDIR/scripts
	if [ "$input_arg" = "ezwire" ];then
		if [ $flash_type -eq 0 ];then
			./am_sdk_config.sh ezwire
		elif [ $flash_type -eq 1 ];then
			./am_sdk_config.sh AM_8252n
		elif [ $flash_type -eq 2 ];then
			./am_sdk_config.sh AM_8252n_8M
		fi
	elif [ "$input_arg" = "8252n" ];then
		./am_sdk_config.sh AM_8252n
	elif [ "$input_arg" = "8258B" ];then
		./am_sdk_config.sh AM_8258B
	elif [ "$input_arg" = "am8258" ];then
		./am_sdk_config.sh AM_8258
	elif [ "$input_arg" = "8258B_Wire" ];then
		./am_sdk_config.sh AM_8258B_Wire
	elif [ "$input_arg" = "8258N_Wire" ];then
		./am_sdk_config.sh AM_8258N_Wire
	elif [ "$input_arg" = "8252n_8M" ];then
		./am_sdk_config.sh AM_8252n_8M
	elif [ "$input_arg" = "8252n_4M" ];then
		./am_sdk_config.sh AM_8252n_4M
	elif [ "$input_arg" = "8258_4M" ];then
		./am_sdk_config.sh AM_8258_4M
	elif [ "$input_arg" = "8258_4M_ADB_MirrorOnly" ];then
		./am_sdk_config.sh AM_8258_4M_ADB_MirrorOnly
	elif [ "$input_arg" = "8268_ezcast" ];then
		./am_sdk_config.sh AM_8268
	elif [ "$input_arg" = "8268_4M" ];then
		./am_sdk_config.sh AM_8268_4M
	elif [ "$input_arg" = "8268B_Wire" ];then
		./am_sdk_config.sh AM_8268B_Wire
	elif [ "$input_arg" = "8268B" ];then
		./am_sdk_config.sh AM_8268B
	elif [ "$input_arg" = "projector" ];then
		./am_sdk_config.sh AM_13
	elif [ "$input_arg" = "probox" -o "$input_arg" = "prolanplus" ];then
		./am_sdk_config.sh probox
	else
		if [ $flash_type -eq 0 ];then
			./am_sdk_config.sh AM_8251
		elif [ $flash_type -eq 1 ];then
			./am_sdk_config.sh AM_8252n_ezcast
		elif [ $flash_type -eq 2 ];then
			./am_sdk_config.sh AM_8252n_8M
		fi
	fi
	if [ $? -ne 0 ];then
		printf "\033[0;31m**************************************\033[0m\n"
		printf "\033[0;31m*      am_sdk_config.sh fail!!       *\033[0m\n"
		printf "\033[0;31m**************************************\033[0m\n"
		exit 1
	fi
	
	if [ ! "$input_arg" = "manual" ]; then
		if [ "$input_arg" != "projector" -a "$input_arg" != "probox" -a "$input_arg" !=  "prolan" -a "$input_arg" !=  "prodongle" -a "$input_arg" != "prolanplus" ]; then
			change_version
		fi
	fi
	build_main_code

	if [ "$input_arg" = "8252n" -o "$input_arg" = "8252n_8M" -o "$input_arg" = "8252n_4M" -o "$input_arg" = "am8258" -o "$input_arg" = "8258B" -o "$input_arg" = "8258B_Wire" -o "$input_arg" = "8258N_Wire" -o "$input_arg" = "8258_4M" -o "$input_arg" = "8258_4M_ADB_MirrorOnly" -o "$input_arg" = "8268_ezcast" -o "$input_arg" = "8268_4M" -o "$input_arg" = "8268B_Wire" -o $flash_type -eq 1 -o $flash_type -eq 2 ];then
		BOOTLOADER_DIR=$TOPDIR/sdk/bootloader/snor/scripts
	fi
	build_bootloader
	if [ "$input_arg" = "ezwilan" -o "$input_arg" = "probox" -o "$input_arg" = "prolan" -o "$input_arg" = "prolanplus" -o "$input_arg" = "projector" ]; then
		build_eeprom
	fi
	build_cgi
	if [ ! "$input_arg" = "manual" ]; then
		if [ "$input_arg" = "ezcast5g" -o "$input_arg" = "ezmusic" -o "$input_arg" = "probox" ]; then
			cd $TOPDIR/scripts
			sh build_wifi_driver.sh 8811au  
			sh build_wifi_driver.sh 8821cu
		fi
		if [ "$input_arg" = "projector"  ]; then
			cd $TOPDIR/scripts
			sh build_wifi_driver.sh 8192cu
		fi
		if [ "$input_arg" = "probox"  ]; then
			cd $TOPDIR/scripts
			sh build_wifi_driver.sh 8189es
		fi
		if [ "$input_arg" != "ezwire" -a "$input_arg" != "8252n" -a "$input_arg" != "8252n_8M" -a "$input_arg" != "8252n_4M" -a "$input_arg" != "8258B" -a "$input_arg" != "am8258" -a "$input_arg" != "8258B_Wire" -a "$input_arg" != "8258N_Wire" -a "$input_arg" != "8258_4M" -a "$input_arg" != "8258_4M_ADB_MirrorOnly" -a "$input_arg" != "8268_ezcast" -a "$input_arg" != "8268_4M" -a "$input_arg" != "8268B_Wire" -a "$input_arg" != "8268B" ];then
			echo "######## running realtek driver build!!!"
			cd $TOPDIR/scripts
			sh build_wifi_driver.sh 8188eu
			sh build_wifi_driver.sh 8188fu
			sh build_wifi_driver.sh 8192du
			sh build_wifi_driver.sh 8192eu
		fi
		if [ "$input_arg" = "am8258" -o "$input_arg" = "8268_ezcast" ];then
			echo "######## running 8188fu  driver build!!!"
			cd $TOPDIR/scripts
			sh build_wifi_driver.sh 8188fu
			if [ $mirascreent5g_flag -eq 1 ];then
				sh build_wifi_driver.sh 8821cu_test
			fi
		fi
	fi

	cd $TOPDIR/scripts
	if [ "$input_arg" = "probox"  -o "$input_arg" = "projector" ];then
		echo "----build_imobiledevice: "
		./build_imobiledevice.sh release
		if [ $? -ne 0 ]; then
           	echo "######################################################"
           	echo "#            make imobiledevice fail                 #"
           	echo "######################################################"
           	exit 1
       	 fi

		cd $TOPDIR/sdk/library/alsa-lib/build/
		./do_build.sh
	fi
	if [ "$input_arg" = "8258_4M_ADB_MirrorOnly" -o "$input_arg" = "8258B_Wire" -o "$input_arg" = "8258N_Wire" -o "$input_arg" = "8252n_8M" -o "$input_arg" = "8268B_Wire" ];then
		cd $TOPDIR/sdk/library/alsa-lib/build/
		./do_build.sh
	fi
	if [ "$input_arg" = "8258_4M" -o "$input_arg" = "8252n_4M" -o "$input_arg" = "8258_4M_ADB_MirrorOnly" -o "$input_arg" = "8258B_Wire" -o "$input_arg" = "8258N_Wire" -o "$input_arg" = "8252n_8M" -o "$input_arg" = "8268_4M" -o "$input_arg" = "8268B_Wire" ];then
		cd $TOPDIR/scripts
		./build_imobiledevice.sh release
		./mkfs.sh ezwire
	elif [ "$input_arg" = "ezwire" -o "$input_arg" = "8252n" -o "$input_arg" = "8258B" -o "$input_arg" = "8268B" ];then
		if [ "$input_arg" = "8252n" -o "$input_arg" = "8258B" -o "$input_arg" = "8268B" -o $flash_type -eq 1 -o $flash_type -eq 2 ];then
			cd $TOPDIR/sdk/library/xml/build/
			./do_build.sh 8252n_8M
		fi
		cd $TOPDIR/scripts
		./build_imobiledevice.sh release
		if [ $? -ne 0 ]; then
           	echo "######################################################"
           	echo "#            make imobiledevice fail                 #"
           	echo "######################################################"
           	exit 1
       	fi

		cd $TOPDIR/sdk/library/alsa-lib/build/
		./do_build.sh

		cd $TOPDIR/scripts
		./mkfs.sh ezwire
	elif [ "$input_arg" = "projector" -o "$input_arg" = "probox" -o "$input_arg" = "prolan" -o "$input_arg" = "prodongle" -o "$input_arg" = "prolanplus" ]; then
		cd $TOPDIR/scripts
		ezcastpro_build_snmp
		if [ $ezcastpro128_flag -eq 1 ];then
			echo "build ezcastpro128 xml"
			cd $TOPDIR/sdk/library/xml/build/
			./do_build.sh 8252n_8M
			cd $TOPDIR/scripts
		fi
		./mkfs.sh ezcastpro  
	else
		if [ $flash_type -eq 1 -o $flash_type -eq 2 ];then
			cd $TOPDIR/sdk/library/xml/build/
			./do_build.sh 8252n_8M
		fi
		cd $TOPDIR/scripts
		if [ $mirascreent5g_flag -eq 1 ];then
			sh build_strip_file.sh
		fi
		./mkfs.sh realtek
	fi
   
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  mkfs.sh fail                      #"
		echo "######################################################"
		exit 1
	fi
	echo "######################################################"
	echo "#                       Done!!!                      #"
	echo "######################################################"
}

check_arg()
{
	if [ $arg_num -eq 0 ]; then
		echo "To build EZCast firmware!!!!"
		return 0
	fi
	if [ "$input_arg" = "am8252_copy" ]; then
		echo "To do 8252 copy firmware!!!!"
		return 0
	fi
	if [ "$input_arg" = "am8252" ]; then
		echo "To build EZCast 8252 firmware!!!!"
		return 0
	fi
	if [ "$input_arg" = "manual" ]; then
		echo "To build EZCast and do not make clean firmware!!!!"
		return 0
	fi
	if [ "$input_arg" = "ezmusic" ]; then
		echo "To build EZMusic firmware!!!!"
		return 0
	fi
	if [ "$input_arg" = "ezwilan" ]; then
		echo "To build EZLAN firmware!!!!"
		return 0
	fi
	if [ "$input_arg" = "ezcast5g" ]; then
		echo "To build EZCast5g firmware!!!!"
		return 0
	fi
	if [ "$input_arg" = "ezwire" ]; then
		echo "To build EZWire firmware!!!!"
		return 0
	fi
	if [ "$input_arg" = "8252n" ]; then
		echo "To build snor firmware for chip 8252N!!!!"
		return 0
	fi 
	if [ "$input_arg" = "8258B" ]; then
		echo "To build snor firmware for chip 8258B 16M snor!!!!"
		return 0
	fi 
	if [ "$input_arg" = "am8258" ]; then
		echo "To build  firmware for chip 8258!!!!"
		return 0
	fi
	if [ "$input_arg" = "8258B_Wire" ]; then
		echo "To build 8M snor firmware for chip 8258B Wire!!!!"
		return 0
	fi
	if [ "$input_arg" = "8258N_Wire" ]; then
		echo "To build  snor firmware for chip 8258N Wire!!!!"
		return 0
	fi
	if [ "$input_arg" = "8258_4M_ADB_MirrorOnly" ]; then
		echo "To build 4M snor adb mirror only firmware for chip 8258!!!!"
		return 0
	fi
	if [ "$input_arg" = "8258_4M" ]; then
		echo "To build 4M snor firmware for chip 8258!!!!"
		return 0
	fi
	if [ "$input_arg" = "8252n_8M" ]; then
		echo "To build 8M snor firmware for chip 8252N!!!!"
		return 0
	fi
	if [ "$input_arg" = "8252n_4M" ]; then
		echo "To build 4M snor firmware for chip 8252N!!!!"
		return 0
	fi
	if [ "$input_arg" = "8268_4M" ]; then
		echo "To build 4M snor firmware for chip 8268!!!!"
		return 0
	fi
	if [ "$input_arg" = "8268B" ]; then
		echo "To build snor firmware for chip 8268B 16M snor!!!!"
		return 0
	fi 
	if [ "$input_arg" = "8268B_Wire" ]; then
		echo "To build 8M snor firmware for chip 8268B Wire!!!!"
		return 0
	fi
	if [ "$input_arg" = "8268_ezcast" ]; then
		echo "To build  firmware for chip 8268!!!!"
		return 0
	fi
	if [ "$input_arg" = "projector" ]; then
		echo "To build 8250 EZCastProjector  firmware!!!!"
		return 0
	fi
	
	if [ "$input_arg" = "probox" ]; then
		echo "To build 8250 EZCastProbox  firmware!!!!"
		return 0
	fi
	if [ "$input_arg" = "prolan" ]; then
		echo "To build 8251 EZCastProLan  firmware!!!!"
		return 0
	fi
	if [ "$input_arg" = "prodongle" ]; then
		echo "To build 8251 EZCastProdongle  firmware!!!!"
		return 0
	fi
	if [ "$input_arg" = "prolanplus" ]; then
		echo "To build 8251 EZCastProLanPlus  firmware!!!!"
		return 0
	fi
	echo "*********************************************************"
	echo " sh ezcast_make.sh [arg]    --  arg is non-required"
	echo " arg:"
	echo "     if arg is empty, then make a ezcast firmware."
	echo "     ezmusic: make a ezmusic firmware."
	echo "     ezwilan: make a EZLAN firmware."
	echo "     ezcast5g: make a ezcast5g firmware."
	echo "     ezwire: make a EZWire firmware."
	echo "     8252n: make a snor firmware for chip 8252N."
	echo "     8258B: make a snor firmware for chip 8258B 16M snor ."
	echo "     am8258: make a  firmware for chip 8258." 
	echo "     8258B_Wire: make a  firmware for chip 8258B Wire."
	echo "     8258N_Wire: make a  firmware for chip 8258N Wire."
	echo "     8258_4M: make a 4M snor firmware for chip 8258."
	echo "     8258_4M_ADB_MirrorOnly: make a 4M_ADB_MirrorOnly snor firmware for chip 8258."
	echo "     8252n_8M: make a 8M snor firmware for chip 8252N."
	echo "     8252n_4M: make a 4M snor firmware for chip 8252N."
	echo "     8268_4M: make a 4M snor firmware for chip 8268."
	echo "     8268B_Wire: make a  firmware for chip 8268B 8M Wire."
	echo "     8268B: make a snor firmware for chip 8268B 16M snor ."
	echo "     8268_ezcast: make a firmware for chip 8268 ezcast."
	echo "     projector: make a EZCastProjector firmware."
	echo "     probox: make a EZCastProbox firmware."
	echo "     prolan: make a EZCastProLan firmware."
	echo "     prodongle: make a EZCastProdongle firmware."
	echo "     prolanplus: make a EZCastProLanPlus firmware."
	echo "*********************************************************"
	exit 1
}

check_arg
do_main

