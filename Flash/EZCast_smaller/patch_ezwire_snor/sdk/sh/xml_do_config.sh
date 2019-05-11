#! /bin/sh

src_top_dir="../libxml2-2.7.8"
install_dir="libxml2_install"

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
if [ $# -eq 1 -a "$1" = "8252n_8M" ];then
echo "8252n_8M xml"
./configure --prefix=$(pwd)/../$install_dir --host=mips-linux-gnu \
--without-ftp --without-html --enable-static=no --enable-shared --with-debug=off \
--without-c14n --without-catalog --without-debug --without-docbook --without-ftp --without-html --without-http --without-iconv \
--without-iso8859x --without-legacy --without-pattern --without-push --without-reader \
--without-regexps --without-schemas --without-schematron \
--without-valid --without-writer --without-xinclude --without-xpath --without-xptr --without-modules \
CC="mips-linux-gnu-gcc -EL" CFLAGS="-mips32 -EL -msoft-float -muclibc" LDFLAGS="-EL -msoft-float -muclibc"
else
echo "ez* xml"
./configure --prefix=$(pwd)/../$install_dir --host=mips-linux-gnu \
--without-ftp --without-html --enable-static=no --with-debug=off \
CC="mips-linux-gnu-gcc -EL" CFLAGS="-mips32 -EL -msoft-float -muclibc" LDFLAGS="-EL -msoft-float -muclibc"
fi
cd ../build
