#!/bin/sh

if [ "$0" = "bash" -o "$0" = "-bash" ]; then
        cd "$(dirname "$BASH_SOURCE")"
        CUR_FILE=$(pwd)/$(basename "$BASH_SOURCE")
        myPath=$(dirname "$CUR_FILE")
        cd - > /dev/null
	#echo "bash-myPath: "$myPath
	#echo "bash-CUR_FILE: "$CUR_FILE
else
        echo "$0" | grep -q "$PWD"
        if [ $? -eq 0 ]; then
                CUR_FILE=$0
        else
                CUR_FILE=$(pwd)/$0
        fi
        myPath=$(dirname "$CUR_FILE")
	#echo "myPath: "$myPath
	#echo "CUR_FILE: "$CUR_FILE
fi

root_dir=$(cd $myPath/.. && pwd)
#echo $root_dir

let release_only=0

if [ $# -eq 1 -a "$1" = "release" ];then
	let release_only=1
fi
echo "release_only: "$release_only

library_dir=$root_dir/sdk/library

openssl_dir=$library_dir/openssl/build
nettle_dir=$library_dir/nettle/build
gnutls_dir=$library_dir/gnutls/build
libxml2_dir=$library_dir/xml/build
libplist_dir=$library_dir/libplist/build
libusbmuxd_dir=$library_dir/libusbmuxd/build
libimobiledevice_dir=$library_dir/libimobiledevice/build
libusb_dir=$library_dir/libusb/build
usbmuxd_dir=$library_dir/usbmuxd/build

cd $libxml2_dir
pwd
if [ $release_only -eq 0 ];then
	echo "Build libxml2"
	sh do_build.sh
	if [ $? -ne 0 ];then
		echo "Build XML2 fail."
		exit 1
	fi
fi

cd $openssl_dir
pwd
if [ $release_only -eq 0 ];then
	echo "Build openssl"
	sh do_build.sh
	if [ $? -ne 0 ];then
		echo "Build openssl fail."
		exit 1
	fi
fi

cd $libplist_dir
pwd
if [ $release_only -eq 0 ];then
	echo "Build libplist"
	sh do_build.sh
	if [ $? -ne 0 ];then
		echo "Build libplist fail."
		exit 1
	fi
fi
echo "Release libplist"
#sh do_release.sh
#cp $libplist_dir/../install/lib/*.a $root_dir/sdk/lib/

cd $libusb_dir
pwd
if [ $release_only -eq 0 ];then
	echo "Build libusb"
	sh do_build.sh
	if [ $? -ne 0 ];then
		echo "Build libusb fail."
		exit 1
	fi
fi
echo "Release libusb"
sh do_release.sh

cd $libusbmuxd_dir
pwd
if [ $release_only -eq 0 ];then
	echo "Build libusbmuxd"
	sh do_build.sh
	if [ $? -ne 0 ];then
		echo "Build libusbmuxd fail."
		exit 1
	fi
fi
echo "Release libusbmuxd"
#sh do_release.sh
#cp $libusbmuxd_dir/../install/lib/*.a $root_dir/sdk/lib/

cd $libimobiledevice_dir
pwd
if [ $release_only -eq 0 ];then
	echo "Build libimobiledevice"
	sh do_build.sh
	if [ $? -ne 0 ];then
		echo "Build libimobiledevice fail."
		exit 1
	fi
fi
echo "Release libimobiledevice"
#sh do_release.sh
cp $libimobiledevice_dir/../install/bin/ideviceinfo $root_dir/sdk/rootfs/usr/sbin
#cp $libimobiledevice_dir/../install/lib/*.a $root_dir/sdk/lib/

cd $usbmuxd_dir
pwd
if [ $release_only -eq 0 ];then
	echo "Build usbmuxd"
	sh do_build.sh
	if [ $? -ne 0 ];then
		echo "Build usbmuxd fail."
		exit 1
	fi
fi
echo "Release usbmuxd"
sh do_release.sh

echo "--END--"
