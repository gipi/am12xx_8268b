#!/bin/sh
#
# build RTL8192EU/8192DU/8192CU/8188EU driver.
# 
# 
#

user_manual()
{
	echo ""
	echo "./build_wifi_driver.sh <arg>"
	echo "arg:"
	echo "	all: build all wifi driver;"
	echo "	8188eu: build 8188eu driver[$RTL8188EU_TAR] to dir[$RTL8188EU_DIR];"
	echo "	8188fu: build 8188fu driver[$RTL8188FU_TAR] to dir[$RTL8188FU_DIR];"
	echo "	8192cu: build 8192cu driver[$RTL8192CU_TAR] to dir[$RTL8192CU_DIR];"
	echo "	8192du: build 8192du driver[$RTL8192DU_TAR] to dir[$RTL8192DU_DIR];"
	echo "	8192eu: build 8192eu driver[$RTL8192EU_TAR] to dir[$RTL8192EU_DIR];"
	echo "	8189es: build 8189es driver in dir[$RTL8189ES_DIR];"
	echo "	8811au: build 8811au driver[$RTL8811AU_TAR] to dir[$RTL8811AU_DIR];"
	echo "	8821cu: build 8821cu driver[$RTL8821CU_TAR] to dir[$RTL8821CU_DIR];"
}

do_make()
{
	if [ $# -ne 1 ];then
                echo "[do_make] args error!!"
                exit 1
        fi
	BUILD_DIR=$1
	cd $BUILD_DIR
        make clean
        make -j4
        if [ $? -ne 0 ]; then
                echo "#####################################################"
                echo "#            $BUILD_DIR driver make error"
                echo "#####################################################"
                exit 1;
        fi
        make install
        if [ $? -ne 0 ]; then
                echo "#####################################################"
                echo "#            $BUILD_DIR driver install error"
                echo "#####################################################"
                exit 1;
        fi
        cd $TOPDIR/scripts
}

do_driver_build()
{
	if [ $# -ne 2 ];then
		echo "[do_driver_build] args error!!"
		exit 1
	fi
	TAR_FILE=$1
	BUILD_DIR=$2
	echo "#####################################################"
	echo "#      Tar file: $TAR_FILE"
	echo "#           dir: $BUILD_DIR"
	echo "#####################################################"

	cd $NET_DIR
	pwd
	if [ -d $BUILD_DIR -o -f $BUILD_DIR ];then
		rm -rf $BUILD_DIR
		echo "  ########## remove "$BUILD_DIR
	fi
	tar xf $TAR_FILE
	do_make $BUILD_DIR
}

input_arg=$1
arg_num=$#
WORK_DIR=$(cd $(dirname $0) && pwd)
TOPDIR=$(cd $WORK_DIR/.. && pwd)
NET_DIR=$TOPDIR/sdk/library/net
#RTL8188EU_DIR=$NET_DIR/rtl8188EUS_linux_v4.1.7_8310.20140521_beta
RTL8188EU_DIR=$NET_DIR/rtl8188EUS_linux_v4.3.0.6_12167.20140828
RTL8188EU_TAR=$RTL8188EU_DIR.tar.gz

RTL8188FU_DIR=$NET_DIR/rtl8188FU_linux_v5.3.0.1_28559.20180629.WEXT
RTL8188FU_TAR=$RTL8188FU_DIR.tar.gz

#RTL8192EU_DIR=$NET_DIR/rtl8192EU_linux_v4.3.1.1_11320.20140505
RTL8192EU_DIR=$NET_DIR/rtl8192EU_linux_v4.3.8_12406.20140929
RTL8192EU_TAR=$RTL8192EU_DIR.tar.gz

RTL8192CU_DIR=$NET_DIR/rtl8188C_8192C_usb_linux_v4.0.5_11249.20140422
RTL8192CU_TAR=$RTL8192CU_DIR.tar.gz

RTL8192DU_DIR=$NET_DIR/rtl8192DU_linux_v4.0.4_10867.20140321_beta
RTL8192DU_TAR=$RTL8192DU_DIR.tar.gz

RTL8189ES_DIR=$NET_DIR/rtl8189ES_linux_v4.3.10.1_13373.20150129

RTL8811AU_DIR=$NET_DIR/rtl8811AU_linux_v4.3.0_10674.20140509
RTL8811AU_TAR=$RTL8811AU_DIR.tar.gz

RTL8821CU_DIR=$NET_DIR/rtl8821CU_WiFi_linux_v5.2.5.2_24506.20171018_COEX20170310-1212
RTL8821CU_TAR=$RTL8821CU_DIR.tar.gz

RTL8821CU_TEST_DIR=$NET_DIR/rtl8821CU_WiFi_linux_v5.4.1_29192.20180822_COEX20180516-2e2e_beta
RTL8821CU_TEST_TAR=$RTL8821CU_TEST_DIR.tar.gz

do_main()
{
	echo "######################################################"
	echo "#                   REALTEK DRIVER                   #"
	echo "######################################################"
	echo "TOPDIR: "$TOPDIR
	if [ $arg_num -ne 1 ]; then
		user_manual
	fi

	if [ "$input_arg" = "8188eu" ]; then
		do_driver_build $RTL8188EU_TAR $RTL8188EU_DIR
	elif [ "$input_arg" = "8188fu" ]; then
		do_driver_build $RTL8188FU_TAR $RTL8188FU_DIR
	elif [ "$input_arg" = "8192cu" ]; then
		do_driver_build $RTL8192CU_TAR $RTL8192CU_DIR
	elif [ "$input_arg" = "8192du" ]; then
		do_driver_build $RTL8192DU_TAR $RTL8192DU_DIR
	elif [ "$input_arg" = "8192eu" ]; then
		do_driver_build $RTL8192EU_TAR $RTL8192EU_DIR
	elif [ "$input_arg" = "8189es" ]; then
		do_make $RTL8189ES_DIR
	elif [ "$input_arg" = "8811au" ]; then
		do_driver_build $RTL8811AU_TAR $RTL8811AU_DIR
	elif [ "$input_arg" = "8821cu" ]; then
		do_driver_build $RTL8821CU_TAR $RTL8821CU_DIR
	elif [ "$input_arg" = "8821cu_test" ]; then
		do_driver_build $RTL8821CU_TEST_TAR $RTL8821CU_TEST_DIR
	elif [ "$input_arg" = "all" ]; then
		do_driver_build $RTL8188EU_TAR $RTL8188EU_DIR
		do_driver_build $RTL8192CU_TAR $RTL8192CU_DIR
		do_driver_build $RTL8192DU_TAR $RTL8192DU_DIR
		do_driver_build $RTL8192EU_TAR $RTL8192EU_DIR
		do_make $RTL8189ES_DIR
		do_driver_build $RTL8811AU_TAR $RTL8811AU_DIR
		do_driver_build $RTL8821CU_TAR $RTL8821CU_DIR
	else
		user_manual
	fi
	
	echo "######################################################"
	echo "#                    Successful                      #"
	echo "######################################################"
}

do_main

