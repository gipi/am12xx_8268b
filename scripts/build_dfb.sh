#! /bin/bash

#**********************************************
# This shell script build the direct fb 
# related open source libraries.
#
# @author: simomn Lee
# @date: 2011.07.09
#**********************************************

CUR_DIR=$(pwd)
SDK_DIR=$CUR_DIR/../sdk
echo $SDK_DIR

#**********************************************
# build the zlib.
#**********************************************
LIB_ZLIB_DIR=$SDK_DIR/library/zlib
LIB_ZLIB_BUILD_DIR=$LIB_ZLIB_DIR/build

echo " Begin building the zlib"

cd $LIB_ZLIB_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the freetype.
#**********************************************
LIB_FREETYPE_DIR=$SDK_DIR/library/freetype
LIB_FREETYPE_BUILD_DIR=$LIB_FREETYPE_DIR/build

echo " Begin building the freetype"

cd $LIB_FREETYPE_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the libjpeg.
#**********************************************
LIB_JPEG_DIR=$SDK_DIR/library/libjpeg
LIB_JPEG_BUILD_DIR=$LIB_JPEG_DIR/build

echo " Begin building the libjpeg"

cd $LIB_JPEG_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the libpng.
#**********************************************
LIB_PNG_DIR=$SDK_DIR/library/libpng
LIB_PNG_BUILD_DIR=$LIB_PNG_DIR/build

echo " Begin building the libpng"

cd $LIB_PNG_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the libgif.
#**********************************************
LIB_GIF_DIR=$SDK_DIR/library/libgif
LIB_GIF_BUILD_DIR=$LIB_GIF_DIR/build

echo " Begin building the libgif"

cd $LIB_GIF_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the directfb.
#**********************************************
LIB_DFB_DIR=$SDK_DIR/library/directfb
LIB_DFB_BUILD_DIR=$LIB_DFB_DIR/build

echo " Begin building the directfb"

cd $LIB_DFB_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR
