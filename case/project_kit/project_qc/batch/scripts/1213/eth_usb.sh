#!/bin/sh

MIRAWIRE_APK_FILE=/am7x/case/data/MiraWire-release.apk
ACTIVITY_V1=2
ACTIVITY_V2=3
ACTIVITY_V3=0

ANDROID_V1=0
ANDROID_V2=0

apkV1=0
apkV2=0
apkV3=0

apkInstalled=0

AMSCREEN_PATH=/am7x/case/data
AMSCREEN_NAME=amscreen.4

ADB_APK_FILE=/am7x/case/data/AdbService.apk

UAC_SUPPORT_FLAG=/tmp/USB_AUDIO_SUPPORT
UAC_START_FLAG=/tmp/UAC_START

amscreenResult=0

aoaSupport="false"
id="unknown"
vendor="unknown"
version="unknown"
hostname="unknown"

audioResult=0

lib_dir="/lib/modules/2.6.27.29"
dev_name=$1
opt=$2
echo "********** $dev_name $opt ************"
pkg=com.actionsmicro.ezcast
service=com.actionsmicro.ezdisplay.service
service2=com.actionsmicro.ezdisplay.service
fun1=initadbmirror
fun2=LaunchScreenCastActivity
cmd=start
adbPkg=com.actionsmicro.adbservice
port_num=41936
 getPortnum()
{
#	echo "=============================================="
	echo "=              getPortnum                   ="
#	echo "=============================================="
	if [ -f /tmp/port_num ];then
		port_num=`cat /tmp/port_num | awk '{print $1}' `
		echo ">>>>>>>>>>>>port_num=$port_num"
		if [ $port_num -eq 41936 ];then
			port_num=41937
			echo $port_num > /tmp/port_num
		else
			port_num=41936
			echo $port_num > /tmp/port_num
		fi
	else
		port_num=41936
		echo $port_num > /tmp/port_num

	fi
	cat /tmp/port_num
}

setPackage()
{
#	echo "=============================================="
#	echo "=              set package                   ="
#	echo "=============================================="
	if [ -f /tmp/MiraWirePlus ];then
		pkg=com.actionsmicro.mirawire
		service=com.actionsmicro.mirawire.service
		service2=""
		fun1=initadbmirror
		fun2=LaunchScreenCastActivity
		cmd=startservice
	fi
}

setDongleInfo()
{
	if [ -f /tmp/ota_config.json ];then
		id=`cat /tmp/ota_config.json | grep mac_address | cut -d: -f2 | cut -d\" -f2`
		vendor=`cat /tmp/ota_config.json | grep vendor | cut -d: -f2 | cut -d\" -f2`
		version=`cat /tmp/ota_config.json | grep firmware_version | cut -d: -f2 | cut -d\" -f2`
		hostname=`hostname`
	fi
}

checkPackage()
{
#	echo "=============================================="
#	echo "=             check package                  ="
#	echo "=============================================="
	adb_ret=1
	while [ $adb_ret -ne 0 ]
	do
		if [ -f /tmp/eth_usb_stop ];then
			exit 1
		fi
		usleep 200000
		appInfo=$(adb shell pm list package -i $pkg)
		adb_ret=$?
	done
	adb shell am force-stop com.actionsmicro.mirawire
	adb shell am force-stop com.actionsmicro.ezcast
	echo $appInfo | grep "$pkg "
	if [ $? -ne 0 ];then
		echo "Please install EZCast APK!!!"
		apkV1=0;
		apkV2=0;
		apkV3=0;
		apkInstalled=0;
	else
		apkInstalled=1;
		apkVer=$(adb shell dumpsys package $pkg | grep versionName)
		apkVer=$(echo ${apkVer} | cut -d= -f2)
		echo "MiraWire Version: \"${apkVer}\""
		echo ${apkVer} | grep "debugVersion"
		if [ $? -eq 0 ];then
			echo "debug APK"
			apkV1=0;
			apkV2=1;
			apkV3=0;
		else
			apkV1=$(echo $apkVer | cut -d. -f1)
			apkV2=$(echo $apkVer | cut -d. -f2)
			apkV3=$(echo $apkVer | cut -d. -f3)
			if [ ! -n "$(echo $apkV1 | sed -n "/^[0-9]\+$/p")" ];then
				apkV1=0;
				apkV2=0;
				apkV3=0;
			fi
		fi
		echo "apkV1: ${apkV1}, apkV2: ${apkV2}, apkV3: ${apkV3}"
		if [ ${apkV1} -gt $ACTIVITY_V1 ];then
			echo "apkV1 OK"
			fun2=LaunchScreenCastActivity
			cmd=start
		elif [ ${apkV1} -eq $ACTIVITY_V1 -a ${apkV2} -gt $ACTIVITY_V2 ];then
			echo "apkV1 and apkV2 OK"
			fun2=LaunchScreenCastActivity
			cmd=start
		elif [ ${apkV1} -eq $ACTIVITY_V1 -a ${apkV2} -eq $ACTIVITY_V2 -a ${apkV3} -ge $ACTIVITY_V3 ];then
			echo "apkV1 and apkV2 and apkV3 OK"
			fun2=LaunchScreenCastActivity
			cmd=start
		else
			fun2=UsbMirrorService
			cmd=startservice
		fi
		
	fi
}

getAndroidVersion()
{
#	echo "=============================================="
#	echo "=           get android version              ="
#	echo "=============================================="
	androidVersion=$(adb shell getprop ro.build.version.release)
	echo "Android version: $androidVersion"
	ANDROID_V1=$(echo $androidVersion | cut -d. -f1)
	ANDROID_V2=$(echo $androidVersion | cut -d. -f2)
}

tryScreenrecord()
{
#	echo "=============================================="
#	echo "=           try screenrecord                 ="
#	echo "=============================================="
	if [ $ANDROID_V1 -lt 4 ];then
		echo "$androidVersion is not support..."
		exit 1
	elif [ $ANDROID_V1 -eq 4 ];then
		if [ $ANDROID_V2 -lt 4 ];then
			echo "$androidVersion is not support..."
			exit 1
		fi
		AMSCREEN_NAME=amscreen.4
	elif [ $ANDROID_V1 -eq 5 ];then
		AMSCREEN_NAME=amscreen.5
	elif [ $ANDROID_V1 -eq 6 ];then
		AMSCREEN_NAME=amscreen.6
	elif [ $ANDROID_V1 -eq 7 ];then
		if [ $ANDROID_V2 -eq 0 ];then
			AMSCREEN_NAME=amscreen.7
		else
			AMSCREEN_NAME=amscreen.71
		fi
	elif [ $ANDROID_V1 -ge 8 ];then
		AMSCREEN_NAME=amscreen.8
	else
		amscreenComplete=1
		amscreenResult=1
		return 1
	fi
	
	pid=`adb shell ps | grep "amscreen" | awk '{print $2}'`
	if [ "0$pid" = "0" ];then
		echo "screenrecord not exist..."
	else
		adb shell kill $pid
	fi

	BIN_PATH1=/data/local/tmp
	BIN_PATH2=/sdcard
	BIN_PATH3=/data/data/$adbPkg/adbBin
	NULL_PATH=NULL
	amscreenComplete=0
	
	while [ $amscreenComplete -eq 0 ]
	do
		binPath=$BIN_PATH1
		BIN_PATH1=$NULL_PATH
		if [ ! "$binPath"x = "$NULL_PATH"x ];then
			adb push $AMSCREEN_PATH/$AMSCREEN_NAME $binPath/
		fi
		if [ "$binPath"x = "$NULL_PATH"x -o $? -ne 0 ];then
			echo "Push to $binPath fail!!!"
			binPath=$BIN_PATH2
			BIN_PATH2=$NULL_PATH
			if [ ! "$binPath"x = "$NULL_PATH"x ];then
				adb push $AMSCREEN_PATH/$AMSCREEN_NAME $binPath/
			fi
			if [ "$binPath"x = "$NULL_PATH"x -o $? -ne 0 ];then
				echo "Push to $binPath fail!!!"
				binPath=$BIN_PATH3
				BIN_PATH3=$NULL_PATH
				if [ ! "$binPath"x = "$NULL_PATH"x ];then
					appInfo=$(adb shell pm list package -i $adbPkg)
					echo $appInfo | grep $adbPkg
					if [ $? -ne 0 ];then
						adb uninstall $adbPkg
						adb install $ADB_APK_FILE
						appInfo=$(adb shell pm list package -i $adbPkg)
						echo $appInfo | grep $adbPkg
						if [ $? -ne 0 ];then
							echo "Install AdbService.apk fail!!!"
							return 1
						fi
					fi
					
					adb shell run-as $adbPkg mkdir $binPath
					adb push $AMSCREEN_PATH/$AMSCREEN_NAME $binPath/
				fi
				if [ "$binPath"x = "$NULL_PATH"x -o $? -ne 0 ];then
					echo "Push to $binPath fail!!!"
					amscreenComplete=1
					amscreenResult=1
					return 1
				fi
			fi
		fi
		
		echo "binPath: $binPath"
		echo "AMSCREEN_NAME: $AMSCREEN_NAME"
		rm /tmp/amscreenRes
		adb shell "$binPath/$AMSCREEN_NAME --bit-rate 3M --output-format h264 --size 1280x720 --port $port_num - || echo 'amscreen fail'" > /tmp/amscreenRes &
		sleep 1
		cat /tmp/amscreenRes
		cat /tmp/amscreenRes | grep "amscreen fail"
		if [ $? -ne 0 ];then
			amscreenComplete=1
			amscreenResult=0
			sleep 1
			echo "apkInstalled: $apkInstalled"
			if [ $apkInstalled -eq 0 ];then
				adbcmd="adb shell am start -a android.intent.action.VIEW -d \"https://www.ezcast.com/adb.php?app_id=browser\&firmware_version=${version}\&ota_vendor=${vendor}\&deviceid=${id}\&hostname=${hostname}\""
			else
				adbcmd="adb shell \"am start --es \"key\" \"98811783\" --es \"msg\" '\"'[{\"type\":\"ad\",\"msg\":{\"hostname\":\"${hostname}\", \"firmware_version\":\"${version}\", \"ota_vendor\":\"${vendor}\", \"deviceid\":\"${id}\"}}]'\"' com.actionsmicro.ezcast/com.actionsmicro.iezvu.activity.HelpActivity2\""
			fi
			echo adbcmd: ${adbcmd}
			eval ${adbcmd}
			return 0
		fi
	done
}

tryApkMirror()
{
#	echo "=============================================="
#	echo "=             try apk mirror                 ="
#	echo "=============================================="
	if [ $ANDROID_V1 -ge 5 ];then
		adb shell am force-stop com.actionsmicro.mirawire
		adb shell am force-stop com.actionsmicro.ezcast
		#adbModel=$(adb shell getprop ro.product.model)
		#echo "Model: ${adbModel}"
		#echo ${adbModel} | grep "X900"
		adbcmd="adb shell am $cmd -a \"$service.$fun1\" --ei port $port_num --ei bitrate 3000000 --ei framerate 30 --es \"msg\" '\"'[{\"type\":\"ad\",\"msg\":{\"hostname\":\"${hostname}\", \"firmware_version\":\"${version}\", \"ota_vendor\":\"${vendor}\", \"deviceid\":\"${id}\"}}]'\"' $pkg/$service2.$fun2"
		#adbcmd="adb shell am $cmd -a \"$service.$fun1\" --ei port $port_num --es \"msg\" '\"'[{\"type\":\"ad\",\"msg\":{\"hostname\":\"${hostname}\", \"firmware_version\":\"${version}\", \"ota_vendor\":\"${vendor}\", \"deviceid\":\"${id}\"}}]'\"' $pkg/$service2.$fun2"
		echo adbcmd: ${adbcmd}
		eval ${adbcmd}
		#adb shell am startservice -a "$service.$fun1" --ei port $port_num $pkg/$service.$fun2
		if [ $? -ne 0 ];then
			echo "ADB issue when start APK!!!"
			amscreenResult=1
			return 1
		else
			amscreenResult=0
		fi
	else
		echo "Android $androidVersion not support APK Mirror..."
		amscreenResult=1
		return 1
	fi
}

trySamsungMirror()
{
	amscreenResult=1

	if [ ${apkV1} -gt 2 ];then
		echo "apkV1 OK"
	elif [ ${apkV1} -eq 2 -a ${apkV2} -ge 3 ];then
		echo "apkV1 and apkV2 OK"
	else
		return 1;
	fi

	local manufacturer=$(adb shell getprop ro.product.manufacturer | awk '{{printf"%s",$0}}' | tr [A-Z] [a-z]);
#	manufacturer=$(echo ${manufacturer:0:7} | tr [A-Z] [a-z]);
	echo "The manufacturer is \"${manufacturer}\""
	model=$(adb shell getprop ro.product.model | awk '{{printf"%s",$0}}' | tr [A-Z] [a-z]);
	echo "The model is \"${model}\""
	if [ "$manufacturer"A == "samsung"A -o "$model"A == "alp-l29"A ];then
		tryApkMirror
	elif [ "$manufacturer"A == "huawei"A ];then
		local marketing_name=$(adb shell getprop ro.config.marketing_name | awk '{{printf"%s",$0}}' | tr [A-Z] [a-z]);
		echo "ro.config.marketing_name: \"${marketing_name}\""
		if [ "${marketing_name}"A == "huawei mate 10"A -o "${marketing_name}"A == "huawei mate 9"A ];then
			tryApkMirror
		fi
	fi
}

case  $dev_name  in
	"iphone") echo "The device is iphone"
		usb1_config=/sys/devices/platform/aotg-usb-am7x-.2/usb1/1-1/bConfigurationValue
		usb2_config=/sys/devices/platform/aotg-usb-am7x_next./usb1/1-1/bConfigurationValue
		driver=/lib/modules/2.6.27.29/ipheth.ko
		ETH_PORT=ieth0
		if [ "$opt" = "in" ];then
			echo "***** iphone in ***********"
			if [ ! -e /mnt/vram/wifi ];then
				mkdir /mnt/vram/wifi
			fi

			#if [ -e $usb1_config ];then
			#	echo 4 > $usb1_config
			#elif [ -e $usb2_config ];then
			#	echo 4 > $usb2_config
			#fi

			killall -9 imirror
			if [ -f /tmp/audio_time_out ];then
				rm /tmp/audio_time_out
			fi
			usbmuxd --enable-exit
			
			idpair_ret=1
			while [ $idpair_ret -ne 0 ]
			do
				if [ -f /tmp/eth_usb_stop ];then
					exit 1
				fi
				sleep 1
				ideviceinfo > /tmp/iosInfo
				idpair_ret=$?
			done
#sleep 5
			touch /tmp/PAIR_OK
			
			unlink /tmp/iosInfo
			insmod $driver
			killall thttpd
			killall udhcpc
			
			ifconfig $ETH_PORT up
			udhcpc -i $ETH_PORT -t 600 -T 1 -n &
			thttpd -C /etc/thttpd.conf -u root
			#do_cmd start_wifi_display

			sync

		elif [ "$opt" = "out" ];then
			echo "***** iphone out ***********"
			#do_cmd stop_wifi_display
			touch /tmp/audio_time_out
			touch /tmp/eth_usb_stop
			if [ -f /tmp/PAIR_OK ];then
				rm /tmp/PAIR_OK
			fi
			killall thttpd
			killall udhcpc
			killall ideviceinfo
			#imirror --exit
			#killall -9 imirror
			usbmuxd --force-exit
			ifconfig $ETH_PORT down
			rmmod $driver
		fi
		;;
	"android") echo "The device is android"
		driver=/lib/modules/2.6.27.29/rndis_host.ko
		if [ "$opt" = "in" ];then
			echo "***** android in ***********"
			if [ ! -e /mnt/vram/wifi ];then
				mkdir /mnt/vram/wifi
			fi

			modprobe $driver

			intf=usb1
			ifconfig $intf up
			if [ $? -ne 0 ];then
				intf=usb0
				ifconfig $intf up
				if [ $? -ne 0 ];then
					echo "!!!!!!!!!!!! interface usb* is not exist!!!"
					exit 1
				fi
			fi

			echo ">>>>>>>>>>>>>> interface is $intf"
			killall thttpd
			killall udhcpc
			udhcpc -i $intf -t 60 -n &
			thttpd -C /etc/thttpd.conf -u root
			#do_cmd start_wifi_display
		elif [ "$opt" = "out" ];then
			echo "***** android out ***********"
			#do_cmd stop_wifi_display
			killall thttpd
			killall udhcpc
			ifconfig usb1 down
			modprobe -r $driver
		fi
		;;
	"adb") echo "To start/stop android adb"
		if [ "$opt" = "start" ];then
			echo "=============================================="
			echo "=              Start ADB                     ="
			echo "=============================================="

			touch /tmp/audio_time_out
			if [ -f $UAC_SUPPORT_FLAG ];then
				rm $UAC_SUPPORT_FLAG
			fi
			
			if [ -f /tmp/audio_time_out ];then
				rm /tmp/audio_time_out
			fi

#			setPackage
			ifconfig lo up
			adb devices
			killall adbForward.app
			do_cmd stopAdbSocket
			adb forward --remove-all
			getPortnum
			checkPackage
			setDongleInfo
			getAndroidVersion
			amscreenResult=1
			if [ $apkV1 -eq 0 -a $apkV2 -eq 0 -a $apkV3 -eq 0 ];then
				tryScreenrecord
				if [ $amscreenResult -ne 0 ];then
					echo "ADB mirroring fail."
					exit 1
				fi
			else
				trySamsungMirror
				if [ $amscreenResult -ne 0 ];then
					amscreenResult=1
					tryScreenrecord
					if [ $amscreenResult -ne 0 ];then
						amscreenResult=1
						tryApkMirror
						if [ $amscreenResult -ne 0 ];then
							echo "ADB mirroring fail."
							exit 1
						fi
					fi
				fi
			fi
			
			adb forward tcp:55555 tcp:$port_num
			if [ $? -ne 0 ];then
				echo "ADB issue when forward!!!"
				exit 1
			fi
			sleep 1
			do_cmd startAdbSocket
			if [ ! -f /tmp/audio_time_not_support ];then
				audio_time -c &
			fi

			if [ -f /tmp/audio_time_not_support ];then
				echo "Do not support audio in this device!!!"
			else
				sleep 3
				sleep 3
				sleep 3
				setDongleInfo
				dacTime=$(getDacTime.app)
				echo "=====Dac time: $dacTime"
				if [ $dacTime -eq 0 ];then
					echo "No audio..."
					aoaSupport="false"
				else
					touch $UAC_SUPPORT_FLAG
				fi
				adbcmd="adb shell \"am startservice -a \"uploadcloud\" --es \"key\" \"98811783\" --es \"msg\" '\"'{\"hostname\":${hostname}, \"supportAOA\":${aoaSupport}, \"firmware_version\":\"${version}\", \"ota_vendor\":\"${vendor}\", \"deviceid\":\"${id}\"}'\"' $pkg/$service.UploadCloudService\""
				#echo adbcmd: ${adbcmd}
				eval ${adbcmd}
				touch $UAC_START_FLAG
			fi
			
		elif [ "$opt" = "stop" ];then
			#killall adbForward.app
			do_cmd stopAdbSocket
			audioResult=`ps | grep "audio_time" | grep -v "grep" | wc -l`
			if [ $audioResult -ne 0 ];then
				touch /tmp/audio_time_out
				usleep 200000
			fi
			touch /tmp/eth_usb_stop
			usleep 200000
			if [ -f $UAC_START_FLAG ];then
				rm $UAC_START_FLAG
			fi
			killall adb
		fi
		;;
	
	"aoa") echo "Switch to AOA"
		if [ -f /tmp/eth_usb_stop ];then
			rm /tmp/eth_usb_stop
		fi
		touch /tmp/audio_time_out
		switchToAoa.app
		if [ $? -ne 0 ];then
			echo "try AOA failed"
			touch /tmp/aoa_not_support
			usleep 10000
		fi
		;;
	"checkuac") #echo "Check UAC"
#		setPackage
		aoaSupport="true"
		dacTime=$(getDacTime.app)
#		echo "=====Dac time: $dacTime"
		if [ $dacTime -ne 0 ];then
			aoaSupport="true"
			setDongleInfo
			adbcmd="adb shell \"am startservice -a \"uploadcloud\" --es \"key\" \"98811783\" --es \"msg\" '\"'{\"hostname\":${hostname}, \"supportAOA\":${aoaSupport}, \"firmware_version\":\"${version}\", \"ota_vendor\":\"${vendor}\", \"deviceid\":\"${id}\"}'\"' $pkg/$service.UploadCloudService\""
			#echo adbcmd: ${adbcmd}
			eval ${adbcmd}
			touch $UAC_SUPPORT_FLAG
		fi
		;;
esac

