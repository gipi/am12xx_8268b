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

src_file=libplist-2.0.tar.gz
src_dir=$root_dir/libplist-2.0
sdk_dir=$(cd $root_dir/../../ && pwd)
lib_dir=$sdk_dir/lib
install_dir=$root_dir/install
libxml2_dir=$(cd $root_dir/../xml/libxml2_install && pwd)

echo "install_dir: $install_dir"

if [ -e $install_dir ]
then
    echo "dir ${install_dir} exist"
	rm -rf $install_dir
else
    echo "dir ${install_dir} not exist" 
fi
mkdir $install_dir
if [ -e $src_dir ]
then
    echo "dir ${src_dir} exist"
else
    echo "Uncompress $src_file"
    cd $root_dir
    tar -xf $src_file
fi

#export PKG_CONFIG_PATH=$libxml2_dir/lib/pkgconfig
#echo $PKG_CONFIG_PATH

cd ${src_dir}
make clean
./configure clean
./configure CC="mips-linux-gnu-gcc -EL" CFLAGS="-mips32 -EL -muclibc -msoft-float" LDFLAGS="-EL -lm -muclibc -msoft-float" CXX="mips-linux-gnu-c++ -EL" CXXFLAGS=$CFLAGS CPP="mips-linux-gnu-cpp -EL" CPPFLAGS=$CFLAGS \
--prefix=$install_dir --host=mips-linux-gnu --enable-shared=no --without-cython 

make
make install


