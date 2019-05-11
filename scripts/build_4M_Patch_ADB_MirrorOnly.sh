#! /bin/bash

#**********************************************
# It is the 8258 4M snor patch 
#
#
# @author: luoyuping
# @date: 2017.10.16
#**********************************************

CUR_DIR=$(pwd)
SDK_DIR=$CUR_DIR/../sdk
echo $SDK_DIR
CASE_DIR=$CUR_DIR/../case
echo $CASE_DIR
FLASH_DIR=$CUR_DIR/../Flash
echo $FLASH_DIR
PATCH_DIR=$CUR_DIR/../Flash/EZCast_smaller/patch_ezwire_snor
echo $PATCH

#**********************************************
# copy script files
#**********************************************
cp -rf $PATCH_DIR/adb_mirror/scripts/cfg/* $CUR_DIR/cfg/


#**********************************************
# copy case files.
#**********************************************

cp  $PATCH_DIR/case/cfg/AM_case.cfg $CASE_DIR/cfg/AM_case.cfg 
#cp  $PATCH_DIR/case/config/Makefile  $CASE_DIR/apps/sys/config/Makefile
cp  $PATCH_DIR/case/usb_process.sh  $CASE_DIR/scripts/1213/
#cp -rf  $PATCH_DIR/adb_mirror/wireUI/* $CASE_DIR/apps/wireUi/

cd $CUR_DIR

#**********************************************
# copy sdk files.
#**********************************************
rm  $SDK_DIR/rootfs/lib/*so*
cp -pdrf $PATCH_DIR/sdk/uclibc/* $SDK_DIR/rootfs/lib/
cp -pdrf $PATCH_DIR/sdk/load_photo/* $SDK_DIR/library/load_photo/
cp  $PATCH_DIR/sdk/debug.app  $SDK_DIR/rootfs/
cp  $PATCH_DIR/adb_mirror/busybox/.config $SDK_DIR/busybox/.config
cp  $PATCH_DIR/adb_mirror/alsa/conf.c  $SDK_DIR/library/alsa-lib/alsa-lib-1.1.1/src/conf.c
cp  $PATCH_DIR/adb_mirror/alsa/parser.c  $SDK_DIR/library/alsa-lib/alsa-lib-1.1.1/src/ucm/parser.c
cp  $PATCH_DIR/adb_mirror/alsa/do_config.sh  $SDK_DIR/library/alsa-lib/build/do_config.sh

cp  $PATCH_DIR/sdk/midware/osapi_makefile $SDK_DIR/library/midware/osapi/makefile 
cp  $PATCH_DIR/sdk/midware/Makefile_vp $SDK_DIR/library/midware/mmm_video/test/Makefile_vp
cp  $PATCH_DIR/sdk/midware/bmp_makefile $SDK_DIR/library/midware/mmm_image/1211/decoder/libbmp/Makefile
cp  $PATCH_DIR/sdk/midware/jpeg_makefile $SDK_DIR/library/midware/mmm_image/1211/decoder/libjpeg/Makefile
cp  $PATCH_DIR/sdk/midware/png_makefile $SDK_DIR/library/midware/mmm_image/1211/decoder/libpng/Makefile
cp  $PATCH_DIR/sdk/midware/tiff_makefile $SDK_DIR/library/midware/mmm_image/1211/decoder/libtiff/Makefile
cp  $PATCH_DIR/sdk/midware/ungif_makefile $SDK_DIR/library/midware/mmm_image/1211/decoder/libungif/Makefile
cp  $PATCH_DIR/sdk/midware/zlib_makefile $SDK_DIR/library/midware/mmm_image/1211/decoder/zlib/Makefile
cp  $PATCH_DIR/sdk/midware/image_makefile $SDK_DIR/library/midware/mmm_image/1211/makefile

cp  -pdrf  $PATCH_DIR/adb_mirror/lib/*so*  $SDK_DIR/lib/
cp  -pdrf  $PATCH_DIR/adb_mirror/bin/do_cmd  $SDK_DIR/rootfs/usr/bin/do_cmd

cd $CUR_DIR

#**********************************************
# copy pic files.
#**********************************************
cp  -rf $PATCH_DIR/adb_mirror/adb_stream/*  $FLASH_DIR/EZCast/Patch/adb_stream/
cp  -rf $PATCH_DIR/pic/* $FLASH_DIR/EZCast/Patch/capture/
cp  $PATCH_DIR/sdk/rcS.fat  $FLASH_DIR/EZCast/Patch/snor/rcS.fat
cd $CUR_DIR





