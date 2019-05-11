#/bin/bash

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

cd ${myPath}/squashfs-tools

if [ -e ${myPath}/../../library -a -e zlib-install/lib/libz.a -a -e zlib-install/include/zlib.h -a -e zlib-install/include/zconf.h ];then
	echo "zlib has been built."
else
	echo "############################"
	echo "#     To build zlib        #"
	echo "############################"

	make clean

	if [ -e zlib-install ];then
		rm zlib-install/ -rf
	fi

	if [ -e zlib-1.2.8 ];then
		rm zlib-1.2.8/ -rf
	fi

	tar -xf zlib-1.2.8.tar.gz
	mkdir zlib-install
	cd zlib-1.2.8
	./configure --prefix=../zlib-install --static
	make
	make install
	cd -
	echo "############################"
	echo "#   build zlib complete    #"
	echo "############################"
fi

make
make install

