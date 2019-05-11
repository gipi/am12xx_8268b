#! /bin/bash

#**********************************************
# strip some files ,so it can reduce the fw size 
#
#
# @author: 
# @date: 2018.4.12
#**********************************************

CUR_DIR=$(pwd)
SDK_LIB=$CUR_DIR/../sdk/lib
FLASH_DIR=$CUR_DIR/../Flash
mips-linux-gnu-strip $SDK_LIB/libcrypto.so.1.0.0
mips-linux-gnu-strip $SDK_LIB/libxml2.so.2.7.8
mips-linux-gnu-strip $SDK_LIB/libdlna.so
mips-linux-gnu-strip $SDK_LIB/libwebsockets.so.8.1
mips-linux-gnu-strip $FLASH_DIR/EZCast/Patch/EZCast5G/hostapd

