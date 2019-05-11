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
# build the libcurl.
#**********************************************
LIBCURL_DIR=$SDK_DIR/library/net/libcurl
LIBCURL_BUILD_DIR=$LIBCURL_DIR/build

echo " Begin building the libcurl"

cd $LIBCURL_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the libxml.
#**********************************************
LIBXML_DIR=$SDK_DIR/library/xml
LIBXML_BUILD_DIR=$LIBXML_DIR/build

echo " Begin building the libxml"

cd $LIBXML_BUILD_DIR
source ./do_config.sh
cd $LIBXML_BUILD_DIR
source ./do_make.sh

cd $CUR_DIR

#**********************************************
# build libetpan.
#**********************************************
LIBETPAN_DIR=$SDK_DIR/library/net/libetpan
LIBETPAN_BUILD_DIR=$LIBETPAN_DIR/build

echo " Begin building the libetpan"

cd $LIBETPAN_BUILD_DIR
source ./do_config.sh
cd $LIBETPAN_BUILD_DIR
source ./do_make.sh

cd $CUR_DIR

#**********************************************
# build the raptor.
#**********************************************
LIBRAPTOR_DIR=$SDK_DIR/library/net/raptor
LIBRAPTOR_BUILD_DIR=$LIBRAPTOR_DIR/build

echo " Begin building the raptor"

cd $LIBRAPTOR_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR



#**********************************************
# build the libjansonc.
#**********************************************
LIBJASON_DIR=$SDK_DIR/library/json-c
LIBJASON_BUILD_DIR=$LIBJASON_DIR/build

echo " Begin building the json-c"

cd $LIBJASON_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR

#**********************************************
# build the flickrcurl.
#**********************************************
LIBFLICKRCURL_DIR=$SDK_DIR/library/net/flickrcurl
LIBFLICKRCURL_BUILD_DIR=$LIBFLICKRCURL_DIR/build

echo " Begin building the flickrcurl"

cd $LIBFLICKRCURL_BUILD_DIR
source ./do_build.sh

cd $CUR_DIR




