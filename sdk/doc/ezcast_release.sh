#*************************************************************
#*************************************************************
#	To release EZCast Customer source.
#	For help, please see function help_show
#	or run option "help"/"--help"/"-h"/"-H"
#*************************************************************
#*************************************************************
#!/bin/sh

arg_help="help"
arg_ezcast="ezcast"
arg_ezcastpro8251="ezcastpro_8251"
arg_ezcastpro8250="ezcastpro_8250"
arg_ezmusic="ezmusic"
arg_ezwire="ezwire"
arg_ezwiresnor="ezwiresnor"
arg_ezwiresnor8m="ezwiresnor8m"
arg_ezwiresnor4m_8252n="ezwiresnor4m_8252n"
arg_ezwiresnor4m_8258="ezwiresnor4m_8258"
arg_miraplug_8258b="miraplug_8258b"
arg_ezwire_8258n="ezwire_8258n"
arg_ezcastsnor="ezcastsnor"
arg_8258bmirawifi="8258bmirawifi"
arg_ezwiresnor4m_8268="ezwiresnor4m_8268"
arg_ezwire_8268B="ezwire_8268b"
arg_miraplug_8268b="miraplug_8268b"
arg_8268bmirawifi="8268bmirawifi"


function help_show()
{
echo " "
echo "  This script is a code to release source for EZCast, EZCastPro, etc."
echo "  sh ezcast_release.sh [option]"
echo "  [option]:"
echo "	$arg_help		-- Print Help (this message) and exit"
echo "	$arg_ezcast		-- Release EZCast, EZCast5G and EZCastLan"
echo "	$arg_ezcastpro8251	-- Release EZCastPro based am8251"
echo "	$arg_ezcastpro8250	-- Release EZCastPro based am8250"
echo "	$arg_ezmusic		-- Release EZMusic"
echo "	$arg_ezwire		-- Release EZWire"
echo "	$arg_ezwiresnor		-- Release EZWire(snor)"
echo "	$arg_ezwiresnor8m		-- Release EZWire(8M snor)"
echo "	$arg_ezwiresnor4m_8252n		-- Release EZWire(8252n 4M snor)"
echo "	$arg_ezwiresnor4m_8258		-- Release EZWire(8258L or 8258D 4M snor)"
echo "	$arg_miraplug_8258b		-- Release MiraPlug(8258B 16M snor)"
echo "	$arg_ezwire_8258n		-- Release MiraWire(8258N snor)"
echo "	$arg_ezcastsnor		-- Release EZCast(16M snor)"
echo "	$arg_8258bmirawifi		-- Release 8258b_mirawifi(16M snor)"
echo "	$arg_ezwiresnor4m_8268		-- Release EZWire(8268D 4M snor)"
echo "	$arg_ezwire_8268B		-- Release EZWire(8268B 8M snor)"
echo "	$arg_miraplug_8268b		-- Release MiraPlug(8268B 16M snor)"
echo "	$arg_8268bmirawifi		-- Release 8268b_mirawifi(16M snor)"

echo " "
}

this_dir=$(pwd)
top_dir=$this_dir/../..
flash_dir=$top_dir/Flash
arg_num=$#
input_arg1=$1

function rel_ezcast()
{
	echo "***************************************************"
	echo "             release EZCast source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8251
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast | xargs rm -rf
	cd ./EZCast
	ls | grep -vx FLA | grep -vx Patch | xargs rm -rf
	cd ./FLA
	rm -f dlna.* ez_air.as EZ_Air.fla mainmenu.* videoOSD.* listd.* ezhelp.* photo_new.* set_video_new.* 
	rm -f videoSubtitle.* set_audio_new.* office_new.* set_photo_new.* officeOSD.* ts_splitter.as 
	rm -f set_sys_new.* video_new.* test.* audio_new.* ezmedia.* Main_storage_bar.*
	rm -rf osdicon
	cd ../Patch
	ls | grep -vx dnsmasq | grep -vx EZCast5G | xargs rm -rf
	echo "remove Path"
	cd $top_dir
	rm -rf $top_dir/Path/
}
function rel_8258b_miraiwifi_snor()
{
	echo "***************************************************"
	echo "             release 8258b_miraiwifi(16M snor) source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8258
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast_smaller | grep -vx EZCast | xargs rm -rf
	cd ./EZCast
	ls | grep -vx Patch | xargs rm -rf
	cd ./Patch
	ls | grep -vx dnsmasq | grep -vx EZCast5G | grep -vx snor | xargs rm -rf
	echo "remove Path"
	cd $top_dir
	rm -rf $top_dir/Path/
}
function rel_ezcast_snor()
{
	echo "***************************************************"
	echo "             release EZCast(16M snor) source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8252n_ezcast
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast_smaller | grep -vx EZCast | xargs rm -rf
	cd ./EZCast
	ls | grep -vx Patch | xargs rm -rf
	cd ./Patch
	ls | grep -vx dnsmasq | grep -vx EZCast5G | grep -vx snor | xargs rm -rf
	echo "remove Path"
	cd $top_dir
	rm -rf $top_dir/Path/
}

function rel_ezwire()
{
	echo "***************************************************"
	echo "             release EZWire source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh ezwire
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast | xargs rm -rf
	cd ./EZCast
	ls | grep -vx FLA | grep -vx Patch | xargs rm -rf
	cd ./FLA
	rm -f dlna.* ez_air.as EZ_Air.fla mainmenu.* videoOSD.* listd.* ezhelp.* photo_new.* set_video_new.* 
	rm -f videoSubtitle.* set_audio_new.* office_new.* set_photo_new.* officeOSD.* ts_splitter.as 
	rm -f set_sys_new.* video_new.* test.* audio_new.* ezmedia.* Main_storage_bar.*
	rm -rf osdicon
	cd ../Patch
	ls | grep -vx dnsmasq | grep -vx EZCast5G | grep -vx adb_stream | grep -vx app | grep -vx capture | grep -vx snor | grep -vx webdownload | xargs rm -rf
	echo "remove Path"
	cd $top_dir
	rm -rf $top_dir/Path/
}
function rel_8258b_miraplug()
{
	echo "***************************************************"
	echo "             release Miraplug (8258b 16M snor) source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8258B
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast | xargs rm -rf
	cd ./EZCast
	ls | grep -vx FLA |  grep -vx SWF |grep -vx Patch | xargs rm -rf
	cd ./SWF
	rm *.swf
	cd ../FLA
	rm -f dlna.* ez_air.as EZ_Air.fla mainmenu.* videoOSD.* listd.* ezhelp.* photo_new.* set_video_new.* 
	rm -f videoSubtitle.* set_audio_new.* office_new.* set_photo_new.* officeOSD.* ts_splitter.as 
	rm -f set_sys_new.* video_new.* test.* audio_new.* ezmedia.* Main_storage_bar.*
	rm -rf osdicon
	cd ../Patch
	ls | grep -vx dnsmasq | grep -vx adb_stream | grep -vx app | grep -vx capture | grep -vx snor | xargs rm -rf
	echo "remove Path"
	cd $top_dir
	rm -rf $top_dir/Path/
}

function rel_8258n_mirawire()
{
	echo "***************************************************"
	echo "             release MiraWire(8258N  snor) source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8258N_Wire
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast | grep -vx EZCast_smaller | xargs rm -rf
	cd ./EZCast
	ls | grep -vx SWF | grep -vx Patch | xargs rm -rf
	cd ./SWF
	rm *.swf
	cd ../Patch
	ls | grep -vx dnsmasq | grep -vx adb_stream | grep -vx app | grep -vx capture | grep -vx snor | xargs rm -rf
	echo "remove Path"
	cd $flash_dir
	cd ./EZCast_smaller
	ls | grep -vx patch_ezwire_snor  | xargs rm -rf
	cd $top_dir
	rm -rf $top_dir/Path/
}
function rel_ezwiresnor()
{
	echo "***************************************************"
	echo "             release EZWire(snor) source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8252n
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast | xargs rm -rf
	cd ./EZCast
	ls | grep -vx FLA | grep -vx Patch | xargs rm -rf
	cd ./FLA
	rm -f dlna.* ez_air.as EZ_Air.fla mainmenu.* videoOSD.* listd.* ezhelp.* photo_new.* set_video_new.* 
	rm -f videoSubtitle.* set_audio_new.* office_new.* set_photo_new.* officeOSD.* ts_splitter.as 
	rm -f set_sys_new.* video_new.* test.* audio_new.* ezmedia.* Main_storage_bar.*
	rm -rf osdicon
	cd ../Patch
	ls | grep -vx dnsmasq | grep -vx adb_stream | grep -vx app | grep -vx capture | grep -vx snor | xargs rm -rf
	echo "remove Path"
	cd $top_dir
	rm -rf $top_dir/Path/
}

function rel_ezwiresnor8m()
{
	echo "***************************************************"
	echo "             release EZWire(8M snor) source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8252n_8M
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast |grep -vx EZCast_smaller | xargs rm -rf
	cd ./EZCast
	ls | grep -vx FLA | grep -vx Patch | xargs rm -rf
	cd ./FLA
	rm -f dlna.* ez_air.as EZ_Air.fla mainmenu.* videoOSD.* listd.* ezhelp.* photo_new.* set_video_new.* 
	rm -f videoSubtitle.* set_audio_new.* office_new.* set_photo_new.* officeOSD.* ts_splitter.as 
	rm -f set_sys_new.* video_new.* test.* audio_new.* ezmedia.* Main_storage_bar.*
	rm -rf osdicon
	cd ../Patch
	ls | grep -vx dnsmasq | grep -vx adb_stream | grep -vx app | grep -vx capture | grep -vx snor | xargs rm -rf
	echo "remove Path"
	cd ./EZCast_smaller
	ls | grep -vx patch_ezwire_snor  | xargs rm -rf
	cd ./SWF
	rm *.swf
	cd $top_dir
	rm -rf $top_dir/Path/
}

function rel_ezwiresnor4m_8252n()
{
	echo "***************************************************"
	echo "             release EZWire(8252n 4M snor) source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8252n_4M
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast | grep -vx EZCast_smaller | xargs rm -rf
	cd ./EZCast
	ls | grep -vx SWF | grep -vx Patch | xargs rm -rf
	cd ./SWF
	rm *.swf
	cd ../Patch
	ls | grep -vx dnsmasq | grep -vx adb_stream | grep -vx app | grep -vx capture | grep -vx snor | xargs rm -rf
	echo "remove Path"
	cd $flash_dir
	cd ./EZCast_smaller
	ls | grep -vx patch_ezwire_snor  | xargs rm -rf
	cd ./SWF
	rm *.swf
	cd $top_dir
	rm -rf $top_dir/Path/
}
function rel_ezwiresnor4m_8258()
{
	echo "***************************************************"
	echo "             release EZWire(8258L or 8258D  4M snor) source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8258_4M
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast | grep -vx EZCast_smaller | xargs rm -rf
	cd ./EZCast
	ls | grep -vx SWF | grep -vx Patch | xargs rm -rf
	cd ./SWF
	rm *.swf
	cd ../Patch
	ls | grep -vx dnsmasq | grep -vx adb_stream | grep -vx app | grep -vx capture | grep -vx snor | xargs rm -rf
	echo "remove Path"
	cd $flash_dir
	cd ./EZCast_smaller
	ls | grep -vx patch_ezwire_snor  | xargs rm -rf
	cd $top_dir
	rm -rf $top_dir/Path/
}

function rel_ezwiresnor4m_8268()
{
	echo "***************************************************"
	echo "             release EZWire( 8268D  4M snor) source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8268_4M
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast | grep -vx EZCast_smaller | xargs rm -rf
	cd ./EZCast
	ls | grep -vx SWF | grep -vx Patch | xargs rm -rf
	cd ./SWF
	rm *.swf
	cd ../Patch
	ls | grep -vx dnsmasq | grep -vx adb_stream | grep -vx app | grep -vx capture | grep -vx snor | xargs rm -rf
	echo "remove Path"
	cd $flash_dir
	cd ./EZCast_smaller
	ls | grep -vx patch_ezwire_snor  | xargs rm -rf
	cd $top_dir
	rm -rf $top_dir/Path/
}

function rel_8268b_mirawire()
{
	echo "***************************************************"
	echo "             release MiraWire(8268b 8M  snor) source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8268B_Wire
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast | grep -vx EZCast_smaller | xargs rm -rf
	cd ./EZCast
	ls | grep -vx SWF | grep -vx Patch | xargs rm -rf
	cd ./SWF
	rm *.swf
	cd ../Patch
	ls | grep -vx dnsmasq | grep -vx adb_stream | grep -vx app | grep -vx capture | grep -vx snor | xargs rm -rf
	echo "remove Path"
	cd $flash_dir
	cd ./EZCast_smaller
	ls | grep -vx patch_ezwire_snor  | xargs rm -rf
	cd $top_dir
	rm -rf $top_dir/Path/
}

function rel_8268b_miraplug()
{
	echo "***************************************************"
	echo "             release Miraplug (8268b 16M snor) source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8268B
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast | xargs rm -rf
	cd ./EZCast
	ls | grep -vx FLA |  grep -vx SWF |grep -vx Patch | xargs rm -rf
	cd ./SWF
	rm *.swf
	cd ../FLA
	rm -f dlna.* ez_air.as EZ_Air.fla mainmenu.* videoOSD.* listd.* ezhelp.* photo_new.* set_video_new.* 
	rm -f videoSubtitle.* set_audio_new.* office_new.* set_photo_new.* officeOSD.* ts_splitter.as 
	rm -f set_sys_new.* video_new.* test.* audio_new.* ezmedia.* Main_storage_bar.*
	rm -rf osdicon
	cd ../Patch
	ls | grep -vx dnsmasq | grep -vx adb_stream | grep -vx app | grep -vx capture | grep -vx snor | xargs rm -rf
	echo "remove Path"
	cd $top_dir
	rm -rf $top_dir/Path/
}

function rel_8268b_miraiwifi_snor()
{
	echo "***************************************************"
	echo "             release 8268b_miraiwifi(16M snor) source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8268
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
	
	echo "remove Flash source code"
	cd $flash_dir
	pwd
	ls | grep -vx EZCast_smaller | grep -vx EZCast | grep -vx MiraScreen_Free | xargs rm -rf
	cd ./EZCast
	ls | grep -vx Patch | xargs rm -rf
	cd ./Patch
	ls | grep -vx dnsmasq | grep -vx EZCast5G | grep -vx snor | xargs rm -rf
	echo "remove Path"
	cd $top_dir
	rm -rf $top_dir/Path/
}

function rel_ezmusic()
{
	echo "***************************************************"
	echo "             release EZMusic source"
	echo "***************************************************"
	sh auto_rel_no_usb.sh AM_8253
	if [ $? -ne 0 ]; then
		echo "######################################################"
		echo "#                  release fail                      #"
		echo "######################################################"
		exit 1
	fi
}

function rel_ezcastpro()
{
	rel_chip=$1
	echo "***************************************************"
	echo "           release EZCastPro 8251 source"
	echo "		chip: "$rel_chip
	echo "***************************************************"
	if [ "$rel_chip" = "AM_8251" -o "$rel_chip" = "AM_13" ]; then
		sh auto_rel_no_usb.sh $rel_chip
		if [ $? -ne 0 ]; then
			echo "######################################################"
			echo "#                  release fail                      #"
			echo "######################################################"
			exit 1
		fi

		echo "remove Flash source code"
		cd $flash_dir
		pwd
		ls | grep -vx EZCast | xargs rm -rf
		cd ./EZCast
		ls | grep -vx FLA | grep -vx Patch | xargs rm -rf
		cd ./FLA
		rm -f dlna.* ez_air.as EZ_Air.fla ez_cast.* ezcast_background.* ezcast_p2p_go.as ezhelp.* main.* Main_device_detect.as 
		rm -f Main_ezmedia_common.as Main_net_common.as Main_top_bar.* miracast.* miracast_lite.* multilanguage.as 
		rm -f OTA_download.* setting.* test.* wifi_list.as
		cd ../Patch
		ls | grep -vx dnsmasq | grep -vx EZCast5G | grep -vx adb_stream | grep -vx app | xargs rm -rf
		echo "remove Path"
		cd $top_dir
		rm -rf $top_dir/Path/
	else
		echo "***************************************************"
		echo "			chip error!!!"
		echo "***************************************************"
	fi
}
function main()
{
	if [ $arg_num -lt 1 ]; then
		help_show
	fi
	if [ "$input_arg1" = "$arg_help" -o "$input_arg1" = "-h"  -o "$input_arg1" = "-H"  -o "$input_arg1" = "--$arg_help" ]; then
		help_show
	elif [ "$input_arg1" = "$arg_ezcast" ]; then
		rel_ezcast
	elif [ "$input_arg1" = "$arg_ezcastpro8251" ]; then
		rel_ezcastpro AM_8251
	elif [ "$input_arg1" = "$arg_ezcastpro8250" ]; then
		rel_ezcastpro AM_13
	elif [ "$input_arg1" = "$arg_ezmusic" ]; then
		rel_ezmusic
	elif [ "$input_arg1" = "$arg_ezwire" ]; then
		rel_ezwire
	elif [ "$input_arg1" = "$arg_ezwiresnor" ]; then
		rel_ezwiresnor
	elif [ "$input_arg1" = "$arg_ezwiresnor8m" ]; then
		rel_ezwiresnor8m
	elif [ "$input_arg1" = "$arg_ezwiresnor4m_8252n" ]; then
		rel_ezwiresnor4m_8252n
	elif [ "$input_arg1" = "$arg_ezcastsnor" ]; then 
		rel_ezcast_snor
	elif [ "$input_arg1" = "$arg_ezwiresnor4m_8258" ]; then
		rel_ezwiresnor4m_8258
	elif [ "$input_arg1" = "$arg_8258bmirawifi" ]; then 
		rel_8258b_miraiwifi_snor
	elif [ "$input_arg1" = "$arg_miraplug_8258b" ]; then   
		rel_8258b_miraplug
	elif [ "$input_arg1" = "$arg_ezwire_8258n" ]; then 
		rel_8258n_mirawire
	elif [ "$input_arg1" = "$arg_ezwiresnor4m_8268" ]; then
		rel_ezwiresnor4m_8268
	elif [ "$input_arg1" = "$arg_8268bmirawifi" ]; then 
		rel_8268b_miraiwifi_snor
	elif [ "$input_arg1" = "$arg_miraplug_8268b" ]; then   
		rel_8268b_miraplug
	elif [ "$input_arg1" = "$arg_ezwire_8268B" ]; then   
		rel_8268b_mirawire
	else
		echo "  args error!!!"
		help_show
	fi
}

main
 
