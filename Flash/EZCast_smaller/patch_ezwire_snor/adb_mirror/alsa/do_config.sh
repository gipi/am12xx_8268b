#! /bin/sh

src_top_dir="../alsa-lib-1.1.1"
install_dir="libalsa"

if [ -e ../$install_dir ]
then
    echo "dir ${install_dir} exist"
else
    echo "dir ${install_dir} not exist" 
    cd ..
    mkdir $install_dir
    cd build
fi


################################
# change to src directory
################################
cd $src_top_dir

################################
# do configure
################################
./configure clean
./configure --enable-debug-assert --with-debug --prefix=$(pwd)/../$install_dir --host=mips-linux-gnu \
CC="mips-linux-gnu-gcc -EL" CFLAGS="-mips32 -EL -muclibc -msoft-float" LDFLAGS="-EL -muclibc -msoft-float" --with-alsa-devdir=/dev

cd ../build
