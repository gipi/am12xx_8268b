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

src_file=libimobiledevice-1.2.1.tar.bz2
src_dir=$root_dir/libimobiledevice-1.2.1
sdk_dir=$(cd $root_dir/../../ && pwd)
lib_dir=$sdk_dir/lib
install_dir=$root_dir/install
libopenssl_dir=$(cd $root_dir/../openssl/openssl_install && pwd)
libplist_dir=$(cd $root_dir/../libplist/install && pwd)
libusbmuxd_dir=$(cd $root_dir/../libusbmuxd/install && pwd)

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
    cp -dprf $myPath/patch/userpref.c $src_dir/common/
	cp -dprf $myPath/patch/lockdown.c.ios11 $src_dir/src/lockdown.c
fi

cd ${src_dir}
make clean
./configure clean
./configure CC="mips-linux-gnu-gcc -EL" CFLAGS="-mips32 -EL -muclibc -msoft-float -I$sdk_dir/include" LDFLAGS="-EL -muclibc -msoft-float -L$sdk_dir/lib" CXX="mips-linux-gnu-c++ -EL" CXXFLAGS="-mips32 -EL -muclibc -msoft-float" \
CPP="mips-linux-gnu-cpp -EL" CPPFLAGS="-mips32 -EL -muclibc -msoft-float" --prefix=$install_dir --host=mips-linux-gnu --enable-shared=no --disable-FEATURE --without-cython \
PKG_CONFIG_PATH=$libopenssl_dir/openssl/lib/pkgconfig:$libplist_dir/lib/pkgconfig:$libusbmuxd_dir/lib/pkgconfig openssl_CFLAGS="-I$sdk_dir/include"

make
make install


