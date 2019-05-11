#!/bin/sh

set -e -o pipefail

CROSS=mips-linux-gnu-
sed -i 's/\(CFLAG_LIBC[ 	]*:=\)\([ 	]*-muclibc\)/\1/g' config.mak
make -C ../sdk/busybox clean all
make clean && make

LIBC6=`${CROSS}gcc -mips32r2 -msoft-float -EL -print-file-name='libc.so.6'`
LDIR=$(dirname $LIBC6)
files=`ls $LDIR/libc.so* $LDIR/libc-*.so* $LDIR/ld*.so* $LDIR/libpthread*.so* \
	$LDIR/libm.so* $LDIR/libm-*.so* $LDIR/libutil*.so*`
RFSLIB=../sdk/rootfs/lib
rm -f $RFSLIB/*.so*
`which cp` -dpf $files $RFSLIB
