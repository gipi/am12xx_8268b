#! /bin/bash

#**********************************************
# This shell script build the network 
# related open source libraries.
#
# @author: simomn Lee
# @date: 2011.6.22
#**********************************************

CUR_DIR=$(pwd)
SDK_DIR=$CUR_DIR/../sdk
echo $SDK_DIR

#**********************************************
# build the bluez-libs.
#**********************************************
BLUEZLIBS_LIB_DIR=$SDK_DIR/library/bluetooth/bluez_libs
BLUEZLIBS_LIB_BUILD_DIR=$BLUEZLIBS_LIB_DIR/build

echo " Begin building the bluez-libs"

cd $BLUEZLIBS_LIB_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the libexpat.
#**********************************************
EXPAT_LIB_DIR=$SDK_DIR/library/bluetooth/libexpat
EXPAT_LIB_BUILD_DIR=$EXPAT_LIB_DIR/build

echo " Begin building the libexpat"

cd $EXPAT_LIB_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the libUSB.
#**********************************************
USB_LIB_DIR=$SDK_DIR/library/bluetooth/libusb
USB_LIB_BUILD_DIR=$USB_LIB_DIR/build

echo " Begin building the libUSB"

cd $USB_LIB_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the libDBUS.
#**********************************************
DBUS_LIB_DIR=$SDK_DIR/library/bluetooth/libdbus
DBUS_LIB_BUILD_DIR=$DBUS_LIB_DIR/build

echo " Begin building the libDBUS"

cd $DBUS_LIB_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the bluez_utils.
#**********************************************
BLUEZUTILS_LIB_DIR=$SDK_DIR/library/bluetooth/bluez_utils
BLUEZUTILS_LIB_BUILD_DIR=$BLUEZUTILS_LIB_DIR/build

echo " Begin building the libUSB"

cd $BLUEZUTILS_LIB_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the openobex.
#**********************************************
OPENOBEX_LIB_DIR=$SDK_DIR/library/bluetooth/openobex
OPENOBEX_LIB_BUILD_DIR=$OPENOBEX_LIB_DIR/build

echo " Begin building the openobex"

cd $OPENOBEX_LIB_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the obexftp.
#**********************************************
OBEXFTP_LIB_DIR=$SDK_DIR/library/bluetooth/obexftp
OBEXFTP_LIB_BUILD_DIR=$OBEXFTP_LIB_DIR/build

echo " Begin building the obexftp"

cd $OBEXFTP_LIB_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR


