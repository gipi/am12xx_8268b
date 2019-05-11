#!/bin/bash
#********************************************************************************
#   Script for Bash Shell of Redhat Linux
#   -------------------------------------
#	A basic utility library for BASH scripting.
#
#
#@Version:  1.0
#@File:  $utils.sh
#@Author:  Pan Ruochen (ijkxyz@msn.com)
#@History: 
#     1. The first version was created on 2008/8/8
#*******************************************************************************
source ./get_git_version.sh

#
#  Filter out comments and blank lines.
#
function strip_comments()
{
	local file;
	if [ -z "$1" ]; then
		file=/dev/fd/0;
	else
		file="$1";
	fi
	gawk -F '#' '$1!=""{print $1}' "$file"
}

function strip_comments_tolower()
{
	local file;
	if [ -z "$1" ]; then
		file=/dev/fd/0;
	else
		file="$1";
	fi
	gawk -F '#' '$1!=""{print tolower($1)}' "$file"
}

function txt2bin()
{
	local file
	if [ -z "$1" ]; then
		file=/dev/fd/0
	else
		file="$1"
	fi
	gawk --re-interval -f \$print-binary-int32.awk "$file"
}


function get_unique_path()
{
	local curdir rootdir OLDIFS flag_abs array level size path i cl
	
	curdir=$1
	rootdir=$2
	shift 2
	while [ -n "$1" ]
	do
		if [ "${1:0:1}" = '/' ]; then
##			echo "*ABS*  $1" >&2
			path=$1
			flag_abs=/
		else
##			echo "*REL*  $1" >&2
			path=$curdir/$1
			flag_abs=
		fi
#		shift 1; continue
		OLDIFS="$IFS"
		IFS='/'
		array=($path)
		IFS="$OLDIFS"
##		echo "ARRAY  ${array[@]}" >&2
		level=()
		size=${#array[@]}
		cl=0
		for((i=0; i<size; i++)) {
			case ${array[$i]} in
			'' | .) ;;
			..)	((cl--)) ;;
			*)	level[$cl]=${array[$i]}; ((cl++));
			esac
		}
		path=/
		size=$cl
		for((i=0; i<$size; i++)) {
			path=$path${level[$i]}
			if [ $i -lt $((size-1)) ]; then
				path=$path/
			fi
		}
##		echo "$path"
##		check_path_conv $1 $path
		if [ "$path" = "$rootdir" ]; then
			echo .
		else
			i=${path##$rootdir/}
			if [ "$i" = "$path" ]; then
				echo "Bad path: $1" >&2
#				return 1
			else
				echo $i
			fi
		fi
		shift 1
	done
}

function get_std_path()
{
	local curdir rootdir OLDIFS flag_abs array level size path i cl
	
	while [ -n "$1" ]
	do
		if [ "${1:0:1}" = '/' ]; then
##			echo "*ABS*  $1" >&2
			path=$1
			flag_abs=/
		else
##			echo "*REL*  $1" >&2
			path=$curdir/$1
			flag_abs=
		fi
#		shift 1; continue
		OLDIFS="$IFS"
		IFS='/'
		array=($path)
		IFS="$OLDIFS"
##		echo "ARRAY  ${array[@]}" >&2
		level=()
		size=${#array[@]}
		cl=0
		for((i=0; i<size; i++)) {
			case ${array[$i]} in
			'' | .) ;;
			..)	((cl--)) ;;
			*)	level[$cl]=${array[$i]}; ((cl++));
			esac
		}
		path=/
		size=$cl
		for((i=0; i<$size; i++)) {
			path=$path${level[$i]}
			if [ $i -lt $((size-1)) ]; then
				path=$path/
			fi
		}
		echo "$path"
		shift 1
	done
}

function do_remove_unused_module
{
	local __rootfs_dir
	
	__rootfs_dir=$2

    ##
    # do the related work according to 
    # the config options
    ##
    case "$1" in
    MODULE_CONFIG_NETWORK)
    		rm -rf ${__rootfs_dir}/lib/modules/2.6.27.29/8192.ko
    		rm -rf ${__rootfs_dir}/am7x/lib/wpa_cli.so
    		rm -rf ${__rootfs_dir}/sbin/wpa_cli
    		rm -rf ${__rootfs_dir}/sbin/wpa_passphrase
    		rm -rf ${__rootfs_dir}/sbin/wpa_supplicant
    #    echo remove MODULE_CONFIG_NETWORK
    ;;
    
    MODULE_CONFIG_FACEBOOK)
    		rm -rf ${__rootfs_dir}/am7x/lib/libjson*
    		rm -rf ${__rootfs_dir}/am7x/lib/libfacebook.so
    #    echo remove MODULE_CONFIG_FACEBOOK
    ;;
    
    MODULE_CONFIG_PICASA)
    		rm -rf ${__rootfs_dir}/am7x/lib/libwebalbum.so
    #    echo remove MODULE_CONFIG_PICASA
    ;;
    
    MODULE_CONFIG_FLICKR)
    		rm -rf ${__rootfs_dir}/am7x/lib/libraptor2*
    		rm -rf ${__rootfs_dir}/am7x/lib/libflickcurl*
    		rm -rf ${__rootfs_dir}/am7x/lib/libflickr.so
    #    echo remove MODULE_CONFIG_FLICKR
    ;;
    
    MODULE_CONFIG_WEBMAIL)
    		rm -rf ${__rootfs_dir}/am7x/lib/libetpan*
    #    echo remove MODULE_CONFIG_WEBMAIL
    ;;
    
    MODULE_CONFIG_DLNA)
        rm -rf ${__rootfs_dir}/lib/libmediabolic*
        rm -rf ${__rootfs_dir}/lib/mediaserver
        rm -rf ${__rootfs_dir}/lib/modules/libmediabolic*
        rm -rf ${__rootfs_dir}/lib/modules/libdigital*
        rm -rf ${__rootfs_dir}/am7x/bin/mediarenderer
    ;;
    MODULE_CONFIG_CARD_UPGRADE)
   	  	rm -rf ${__rootfs_dir}/lib/modules/2.6.27.29/am7x_upgrade.ko
    #    echo remove MODULE_CONFIG_CARD_UPGRADE
    ;;
	MODULE_CONFIG_BLUETOOTH)
		rm -rf ${__rootfs_dir}/lib/modules/2.6.27.29/bluetooth.ko
		rm -rf ${__rootfs_dir}/lib/modules/2.6.27.29/rfcomm.ko
		rm -rf ${__rootfs_dir}/lib/modules/2.6.27.29/l2cap.ko
	#	echo remove MODULE_CONFIG_BLUETOOTH
    ;;
    
    MODULE_CONFIG_QRCODE_GENERATOR)
		rm -rf ${__rootfs_dir}/am7x/lib/libqrcode.so
    ;;
    
    MODULE_CONFIG_SQLITE)
		rm -rf ${__rootfs_dir}/am7x/lib/libsqlite3*
    ;;
    
    *)
    #    echo "invalidate config options"
    ;;
    
    esac
}

function do_generate_compile_options
{
    echo "CFLAGS += -D$1" >>module_config.mak
    echo "MOD_CONF_FLAGS += -D$1" >>module_config.mak
    echo "AM_MODULE_CONFIG_FLAGS += $1" >>module_config.mak
}

function do_module_process
{
    local __arg __config_file __remove_dir
    
    __config_file=$1
    __arg=$2
    __remove_dir=$3
    
    if [ "$__arg" -eq "1" ];then
        if [ -f "module_config.mak" ];then
            #echo "module_config.mak already exists,remove it"
            rm -rf module_config.mak
        fi
        echo "ifneq (\${__module_config_inited},1)" >>module_config.mak 
    fi
    
    if [ -f "$__config_file" ];then
        cat $__config_file | awk -F'/' '$1{print $1}' | awk '$3=='$__arg'{print $2}' | while read line
        do
            if [ "$__arg" -eq "0" ];then
                do_remove_unused_module $line $__remove_dir
            else
                do_generate_compile_options $line
            fi
        done
        
        ###############################################
        # Export module config to system.
        ###############################################
        rm -rf ./mconfig.tmp
        echo "#ifndef __ACT_MODULE_CONFIG_H__" >>./mconfig.tmp
        echo "#define __ACT_MODULE_CONFIG_H__" >>./mconfig.tmp
        cat $__config_file | grep "//" >>./mconfig.tmp

        cp -rf ./mconfig.tmp ../sdk/linux/include/linux/am7x_mconfig.h
        cp -rf ./mconfig.tmp ../case/include/am7x_mconfig.h
        cp -rf ./mconfig.tmp ../case/apps/miracast/mconfig.h
	if [ -d "../sdk/library" ];then
		cp -rf ./mconfig.tmp ../sdk/library/midware/inc/am7x_mconfig.h
	fi
		#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		# for EZCastPro options
		#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        if [ -f $TOP_DIR/case/include/ezcastpro.h ];then
#			echo "ezcastpro.h already exists,remove it"
			rm -f $TOP_DIR/case/include/ezcastpro.h
        fi
        cat $__config_file | grep "//" | awk '$3>1{print}' > $TOP_DIR/case/include/ezcastpro.h
        cat $__config_file | grep "MODULE_CONFIG_HDMI2MHL_ENABLE" >> $TOP_DIR/case/include/ezcastpro.h		
        EZCAST_CONFIG=$TOP_DIR/case/include/ezcast_config.h
        EZCAST_CONFIG_SDK=$TOP_DIR/sdk/include/ezcast_config.h
        echo "#ifndef __EZCAST_H_SDK__		// This file created by scripts/$utils.sh" > $EZCAST_CONFIG
        echo "#define __EZCAST_H_SDK__" >> $EZCAST_CONFIG
        echo "" >> $EZCAST_CONFIG
		echo "#ifdef MODULE_CONFIG_EZWIRE_TYPE" >> $EZCAST_CONFIG
		echo "#undef MODULE_CONFIG_EZWIRE_TYPE" >> $EZCAST_CONFIG
		echo "#endif" >> $EZCAST_CONFIG
		echo "" >> $EZCAST_CONFIG
		echo "#ifdef MODULE_CONFIG_FLASH_TYPE" >> $EZCAST_CONFIG
		echo "#undef MODULE_CONFIG_FLASH_TYPE" >> $EZCAST_CONFIG
		echo "#endif" >> $EZCAST_CONFIG
		echo "#ifdef MODULE_CONFIG_CHIP_TYPE" >> $EZCAST_CONFIG
		echo "#undef MODULE_CONFIG_CHIP_TYPE" >> $EZCAST_CONFIG
		echo "#endif" >> $EZCAST_CONFIG
		echo "#ifdef MODULE_CONFIG_QR_URL" >> $EZCAST_CONFIG
		echo "#undef MODULE_CONFIG_QR_URL" >> $EZCAST_CONFIG
		echo "#endif" >> $EZCAST_CONFIG
		echo "#ifdef MODULE_CONFIG_QR_APP_VENDOR" >> $EZCAST_CONFIG
		echo "#undef MODULE_CONFIG_QR_APP_VENDOR" >> $EZCAST_CONFIG
		echo "#endif" >> $EZCAST_CONFIG
		echo "" >> $EZCAST_CONFIG
        cat $__config_file | grep "MODULE_CONFIG_EZWILAN_ENABLE" | awk '$3==1{print}' | grep "MODULE_CONFIG_EZWILAN_ENABLE" && echo "#define EZWILAN_ENABLE 1" >> $EZCAST_CONFIG || echo "#define EZWILAN_ENABLE 0" >> $EZCAST_CONFIG
		cat $__config_file | grep "MODULE_CONFIG_EZWIRE_TYPE" >> $EZCAST_CONFIG || echo "It's not EZWire."
		cat $__config_file | grep "MODULE_CONFIG_FLASH_TYPE" >> $EZCAST_CONFIG || echo "Not set flash type, maybe nand flash."
 		cat $__config_file | grep "MODULE_CONFIG_CHIP_TYPE" >> $EZCAST_CONFIG || echo "Not set chip type."
		cat $__config_file | grep "MODULE_CONFIG_QR_URL" >> $EZCAST_CONFIG || echo "Not set QR URL."
		cat $__config_file | grep "MODULE_CONFIG_QR_APP_VENDOR" >> $EZCAST_CONFIG || echo "Not set QR APP VENDOR."
       echo "" >> $EZCAST_CONFIG
        echo "#endif" >> $EZCAST_CONFIG
        cp -dprf $EZCAST_CONFIG $EZCAST_CONFIG_SDK
    fi
    
    if [ "$__arg" -eq "1" ];then
        if [ -f "module_config.mak" ];then
            echo "endif" >>module_config.mak
        fi
    fi
    
   
    
}

function export_version_git()
{
    local __sdk_version __case_version __scripts_version scripts_dir
    local __transmit_svn_version=0
    scripts_dir=.
    
    if [ -f "version.conf" ];then
        rm -rf version.conf
    fi

    #echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> export_version"

    #>> software/patch/hardware information.
    if [ -f "$1" ];then
        cat $1 | awk '$4{print $2,"=",$3}' | while read line
        do
            echo $line >>version.conf
        done
    fi
	
	major_file=./Major_Version
    if [ ! -d "../.git" ]; then
		echo "It's release source, continue!!!"
		__transmit_svn_version=`cat $major_file | grep Major_Version= | cut -d= -f2` && echo "__transmit_svn_version="$__transmit_svn_version || __transmit_svn_version="00000"
	else
		__transmit_svn_version=$(get_version_from_git)
		echo "Major_Version="$__transmit_svn_version > $major_file
    fi
	__sdk_version=$__transmit_svn_version
	__case_version=$__transmit_svn_version
	__scripts_version=$__transmit_svn_version
  
    echo "VERSION_SVN = Sdk${__sdk_version}.Case${__case_version}.Scripts${__scripts_version}" >>version.conf
	echo  "BUILD_VERSION = $(date +%Y%m%d-%H%M%S)" >>version.conf
    
    cat ./mconfig.h | awk '$4{print $2,"=",$3}' | while read line
    do
        if echo "$line" | grep -q "MODULE_CONFIG_DDR_TYPE" 
        then
            echo $line >>version.conf
        fi
        if echo "$line" | grep -q "MODULE_CONFIG_DDR_CAPACITY"
        then
            echo $line >>version.conf
        fi
    done
    sed -e 's/MODULE_CONFIG_//g' -e 's/VERSION_FIRMWARE/FIRMWARE/g' -e 's/VERSION_rootfs/rootfs/g' -e 's/VERSION_user1/user1/g' -e 's/VERSION_cdrom/cdrom/g' -e 's/VERSION_bak/bak/g' -e 's/VERSION_vram/vram/g' -e 's/VERSION_fwbak/fwbak/g' -e 's/VERSION_udisk/udisk/g'<version.conf > version_new.conf
    rm -rf version.conf
    mv version_new.conf version.conf
    cp -f version.conf ../sdk/rootfs/etc/
    
}

#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# Export the version information to configure file located 
# at sdk/rootfs/etc/version.conf.
# Called before mkfs.
#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
function export_version
{
    local __sdk_version __case_version __scripts_version scripts_dir
    scripts_dir=.
    
    if [ -f "version.conf" ];then
        rm -rf version.conf
    fi

    #echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> export_version"

    #>> software/patch/hardware information.
    if [ -f "$1" ];then
        cat $1 | awk '$4{print $2,"=",$3}' | while read line
        do
            echo $line >>version.conf
        done
    fi
    
    #>> svn version
    tmp_cnt=0
    if [ -f ../sdk/.svn/entries ];then
        while read line
        do
            let tmp_cnt+=1;
            if [ "$tmp_cnt" == "11" ];then
                __sdk_version=$line
                break;
            fi
        done <../sdk/.svn/entries
    fi
    
    tmp_cnt=0
    if [ -f ../case/.svn/entries ];then
        while read line
        do
            let tmp_cnt+=1;
            if [ "$tmp_cnt" == "11" ];then
                __case_version=$line
                break;
            fi
        done <../case/.svn/entries
    fi
    
    tmp_cnt=0
    if [ -f ./.svn/entries ];then
        while read line
        do
            let tmp_cnt+=1;
            if [ "$tmp_cnt" == "11" ];then
                __scripts_version=$line
                #echo $__scripts_version
                break;
            fi
        done <./.svn/entries
    fi
    
    if [ $__sdk_version -ge $__case_version ];then
        if [ $__sdk_version -ge $__scripts_version ];then
            tmp_cnt=$__sdk_version
        else
            tmp_cnt=$__scripts_version
        fi
    else
        if [ $__case_version -ge $__scripts_version ];then
            tmp_cnt=$__case_version
        else
            tmp_cnt=$__scripts_version
        fi
    fi
    
    #echo "tmp_cnt=$tmp_cnt"
    echo "VERSION_SVN = Sdk${__sdk_version}.Case${__case_version}.Scripts${__scripts_version}" >>version.conf
	echo  "BUILD_VERSION = $(date +%Y%m%d-%H%M%S)" >>version.conf
    #echo "FIRMWARE = ${tmp_cnt}">>version.conf
    cat ./mconfig.h | awk '$4{print $2,"=",$3}' | while read line
    do
        if echo "$line" | grep -q "MODULE_CONFIG_DDR_TYPE" 
        then
            echo $line >>version.conf
        fi
        if echo "$line" | grep -q "MODULE_CONFIG_DDR_CAPACITY"
        then
            echo $line >>version.conf
        fi
    done
    sed -e 's/MODULE_CONFIG_//g' -e 's/VERSION_FIRMWARE/FIRMWARE/g' -e 's/VERSION_rootfs/rootfs/g' -e 's/VERSION_user1/user1/g' -e 's/VERSION_cdrom/cdrom/g' -e 's/VERSION_bak/bak/g' -e 's/VERSION_vram/vram/g' -e 's/VERSION_fwbak/fwbak/g' -e 's/VERSION_udisk/udisk/g'<version.conf > version_new.conf
    rm -rf version.conf
    mv version_new.conf version.conf
    cp -f version.conf ../sdk/rootfs/etc/
    
}

function do_wolconfig_process
{
    local __arg __config_file __remove_dir
    
    __config_file=$1
    __arg=$2
    __remove_dir=$3
    
    if [ "$__arg" -eq "1" ];then
        if [ -f "module_config.mak" ];then
            #echo "module_config.mak already exists,remove it"
            rm -rf module_config.mak
        fi
        echo "ifneq (\${__module_config_inited},1)" >>module_config.mak 
    fi
    
    if [ -f "$__config_file" ];then
        cat $__config_file | awk -F'/' '$1{print $1}' | awk '$3=='$__arg'{print $2}' | while read line
        do
            if [ "$__arg" -eq "0" ];then
                do_remove_unused_module $line $__remove_dir
            else
                do_generate_compile_options $line
            fi
        done
        
        ###############################################
        # Export module config to system.
        ###############################################
        rm -rf ./wolconfig.tmp
        echo "#ifndef __WAKE_ONLAN_CONFIG_H__" >>./wolconfig.tmp
        echo "#define __WAKE_ONLAN_CONFIG_H__" >>./wolconfig.tmp
        cat $__config_file | grep "//" >>./wolconfig.tmp

        cp -rf ./wolconfig.tmp ../sdk/include/am7x_wolconfig.h
    else # If not fing wolconfig.h, generate a dummy header for am7x_mac.h
        rm -rf ./wolconfig.tmp
        echo "#ifndef __WAKE_ONLAN_CONFIG_H__" >>./wolconfig.tmp
        echo "#define __WAKE_ONLAN_CONFIG_H__" >>./wolconfig.tmp
        echo "//Not find wolconfig.h" >>./wolconfig.tmp
        echo "#endif" >>./wolconfig.tmp
        cp -rf ./wolconfig.tmp ../sdk/include/am7x_wolconfig.h
    fi

    if [ "$__arg" -eq "1" ];then
        if [ -f "module_config.mak" ];then
            echo "endif" >>module_config.mak
        fi
    fi

}

export -f strip_comments strip_comments_tolower \
	txt2bin get_unique_path get_std_path

SV_CMD_UNSET_FUNCTIONS='unset -f strip_comments strip_comments_tolower 
	txt2bin get_unique_path get_std_path'

