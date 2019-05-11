#!/bin/sh

openssl_src_dir=../openssl-0.9.8zh
openssl_install_dir="openssl_install"

if [ -e ../$openssl_install_dir ]
then
    echo "dir ${openssl_install_dir} exist"
else
    echo "dir ${openssl_install_dir} not exist" 
    cd ../
    mkdir $openssl_install_dir
    cd ./build
fi

export MY_CROSS=mips-linux-gnu-

cd ${openssl_src_dir}
CC="mips-linux-gnu-gcc -mips32 -Os -EL -muclibc -msoft-float" RANLIB="mips-linux-gnu-ranlib" \
./Configure --prefix=$(pwd)/../$openssl_install_dir/openssl \
 shared linux-generic32


#./Configure dist --prefix=$(pwd)/../$openssl_install_dir shared -fPIC

make clean
make
make install

