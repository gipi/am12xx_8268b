#! /bin/sh

CFG_FILE=mydebugfs.cfg

cat > $CFG_FILE << __EOF__
debug.img    ../sdk/debug      1M    ext2
__EOF__
filelist='
../case/images/fui.app
'

DIR_DEBUG=../sdk/debug
rm -f $DIR_DEBUG/*
if test -n "$filelist"; then
#	/bin/mv -f $filelist $DIR_DEBUG/
	/bin/cp -f $filelist $DIR_DEBUG/
fi
./mkfs.sh -f $CFG_FILE
rm -f $CFG_FILE

