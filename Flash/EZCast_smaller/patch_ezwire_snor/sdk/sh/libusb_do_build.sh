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

src_file=$root_dir/libusb-1.0.9.tar.bz2
src_dir=$root_dir/libusb-1.0.9
sdk_dir=$(cd $root_dir/../../ && pwd)
lib_dir=$sdk_dir/lib
install_dir=$root_dir/install

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
fi

cd ${src_dir}
make clean
./configure clean
./configure CC="mips-linux-gnu-gcc -EL" CFLAGS="-mips32 -EL -muclibc -msoft-float" LDFLAGS="-EL -muclibc -msoft-float" --prefix=$install_dir --host=mips-linux-gnu

make
make install


