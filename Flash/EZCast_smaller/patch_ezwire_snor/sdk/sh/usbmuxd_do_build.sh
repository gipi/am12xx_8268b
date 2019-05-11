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

src_file=usbmuxd-1.1.0.tar.bz2
src_dir=$root_dir/usbmuxd-1.1.0
sdk_dir=$(cd $root_dir/../../ && pwd)
lib_dir=$sdk_dir/lib
install_dir=$root_dir/install
libplist_dir=$(cd $root_dir/../libplist/install && pwd)
libusb_dir=$(cd $root_dir/../libusb/install && pwd)
libimobiledevice_dir=$(cd $root_dir/../libimobiledevice/install && pwd)

if [ -e $install_dir ]
then
    echo "dir ${install_dir} exist"
else
    echo "dir ${install_dir} not exist" 
    mkdir $install_dir
fi

if [ -e $src_dir ]
then
    echo "dir ${src_dir} exist"
else
    echo "Uncompress $src_file"
    cd $root_dir
    tar -xf $src_file
    cp -dprf $myPath/patch/conf.c $src_dir/src/
    cp -dprf $myPath/patch/main.c $src_dir/src/
    cp -dprf $myPath/patch/usbmuxd-proto.h $src_dir/src/
    cp -dprf $myPath/patch/config.h.in $src_dir/
    cp -dprf $myPath/patch/utils.c $src_dir/src/
fi

cd ${src_dir}
make clean
./configure clean
./configure CC="mips-linux-gnu-gcc -EL" CFLAGS="-mips32 -muclibc -msoft-float -EL -I$libimobiledevice_dir/include" LDFLAGS="-EL -muclibc -msoft-float" \
CXX="mips-linux-gnu-c++ -EL" CXXFLAGS=$CFLAGS CPP="mips-linux-gnu-cpp -EL" CPPFLAGS=$CFLAGS --host=mips-linux-gnu \
--prefix=$install_dir --enable-shared=no --without-systemd --with-udevrulesdir=$install_dir \
libimobiledevice_CFLAGS="-I$libimobiledevice_dir/include" libimobiledevice_LIBS="-L$libimobiledevice_dir/lib -limobiledevice" \
PKG_CONFIG_PATH=$libplist_dir/lib/pkgconfig:$libusb_dir/lib/pkgconfig:$libimobiledevice_dir/lib/pkgconfig

make
make install


