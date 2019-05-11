#***********************************************************************
#***********************************************************************
#						description
#		1. remove nand/card/lcm/2d/hantro/dac/code recover/card update ....
#		2. remove relative from kconfig .config Makefile
#		3. remove sdk library/app bootloader
#		4. release relative driver binary/library binary /app binary
#***********************************************************************
#***********************************************************************
#! /bin/bash

set -e -o pipefail

ROOT=../../
SDK_HOME=$ROOT/sdk
LINUX_HOME=$SDK_HOME/linux
CASE_HOME=$ROOT/case
SCRIPTS_HOME=$ROOT/scripts
FLASH_HOME=$ROOT/Flash

if [ "$1" == "AM_11" ]; then
    AM_CHIP_ID=1211
    CONFIG_LINUX_FILE=config.linux
elif [ "$1" == "AM_13" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux
elif [ "$1" == "AM_8251" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8251
elif [ "$1" == "AM_8253" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8251
elif [ "$1" == "ezwire" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.ezwire
elif [ "$1" == "AM_8252n" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8252n
elif [ "$1" == "AM_8252n_8M" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8252n_8M
elif [ "$1" == "AM_8252n_4M" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8252n_4M
elif [ "$1" == "AM_8252n_ezcast" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8252n_ezcast
elif [ "$1" == "AM_8258B" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8258B  
elif [ "$1" == "AM_8258N_Wire" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8258N_Wire  
elif [ "$1" == "AM_8258" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8258
elif [ "$1" == "AM_8258_4M" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8258_4M
elif [ "$1" == "AM_8268_4M" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8268_4M
elif [ "$1" == "AM_8268B_Wire" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8268B_Wire 
elif [ "$1" == "AM_8268B" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8268B  
elif [ "$1" == "AM_8268" ]; then
    AM_CHIP_ID=1213
    CONFIG_LINUX_FILE=config.linux.8268

else
    echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
    echo ">> Warning:Must specify the release chip type!"
    echo ">> Usage: sh auto_rel_no_usb.sh AM_XX"
    echo ">>   XX represents AM_11,AM_13,AM_8251 etc."
    echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
    exit 1
fi


#****************************************
#      am sdk config 
#****************************************
echo "[	step 1 :am sdk config...	]"
cd $ROOT/scripts
sh am_sdk_config.sh $1
cd ../sdk/doc


#****************************************
#      build the source 
#****************************************
echo "[	step 2:build the sdk...	]"
cd $ROOT/scripts
make all
sh build_wifi_driver.sh 8192eu
cd ../sdk/doc

#****************************************
#      build the bootloader 
#****************************************
echo "[	step 3:build the bootloader...	]"
if [ "$1"A = "AM_8252n"A -o "$1"A = "AM_8252n_ezcast"A -o "$1"A = "AM_8252n_8M"A  -o "$1"A = "AM_8252n_4M"A -o  "$1"A = "AM_8258"A -o "$1"A = "AM_8258_4M"A -o  "$1"A = "AM_8258B"A -o "$1"A = "AM_8258N_Wire"A  -o  "$1"A = "AM_8268"A -o "$1"A = "AM_8268_4M"A -o "$1"A = "AM_8268B_Wire"A -o  "$1"A = "AM_8268B"A ];then
	echo "****************"
	echo "To build snor"
	echo "****************"
	cd $ROOT/sdk/bootloader/snor/scripts
	if [ "$1"A = "AM_8258"A -o "$1"A = "AM_8258_4M"A -o  "$1"A = "AM_8258B"A -o "$1"A = "AM_8258N_Wire"A ];then
		sh am_sdk_config.sh AM_8258
	elif [ "$1"A = "AM_8268"A -o "$1"A = "AM_8268_4M"A -o  "$1"A = "AM_8268B"A -o "$1"A = "AM_8268B_Wire"A ];then
		sh am_sdk_config.sh AM_8268
	else
		sh am_sdk_config.sh AM_8251
	fi
    make clean
    make all
else

	cd $ROOT/sdk/bootloader/nand/scripts
	sh am_sdk_config.sh $1
    make clean
    make all
    cd ../../eeprom/scripts
    sh am_sdk_config.sh $1
    make clean
    make all
fi
cd ../../../../sdk/doc

#****************************************
#      copy modules into rootfs 
#****************************************
echo "[	step 4 : mkfs...	]"
cd $ROOT/scripts
if [ "$1"A == "ezwire"A -o "$1"A = "AM_8252n"A -o "$1"A = "AM_8252n_8M"A -o "$1"A = "AM_8252n_4M"A -o "$1"A = "AM_8258_4M"A -o  "$1"A = "AM_8258B"A -o "$1"A = "AM_8258N_Wire"A  -o "$1"A = "AM_8268_4M"A -o  "$1"A = "AM_8268B"A -o "$1"A = "AM_8268B_Wire"A ];then
	sh mkfs.sh ezwire
elif [ "$1"A = "AM_8252n_ezcast"A -o  "$1"A = "AM_8258"A  -o  "$1"A = "AM_8268"A ];then
	sh mkfs.sh realtek
else
	sh mkfs.sh 
fi
cd ../sdk/doc

#****************************************
#      clean the case objs
#****************************************
echo "[	step 5 : clean case objects...	]"
cd $ROOT/scripts
make z=c .case
cd ../sdk/doc

#****************************************
#      release nand flash module 
#****************************************
#echo "release nand flash"
#echo "release nand makefile&&kconfig"

#grep -v -E   "MTD|AM_NFTL|FTL|SSFDC" $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE >/tmp/config
#mv /tmp/config   $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE

#sed -i '/# CONFIG_PARPORT is not set/i\# CONFIG_MTD is not set'  $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE 

#grep -v  "AM_NFTL"  $LINUX_HOME/drivers/mtd/Makefile > /tmp/config
#mv /tmp/config   $LINUX_HOME/drivers/mtd/Makefile
 
#grep -v "am7x_nftl" $LINUX_HOME/drivers/mtd/Kconfig > /tmp/config
#mv /tmp/config  $LINUX_HOME/drivers/mtd/Kconfig

#echo "remove nand source codes" 
#rm -Rf $LINUX_HOME/drivers/mtd/am7x_nftl/

#****************************************
#      release hantro module
#****************************************
echo "release hantro module"
echo "release hantro makefile&&kconfig"

grep -v -E   "AM7X_HX170DEC" $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE >/tmp/config
mv /tmp/config   $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE

sed -e '/config  AM7X_HX170DEC/,/  For AM hatro decoder. Say M to make a module and say Y to choose kernel support/d' $LINUX_HOME/drivers/char/Kconfig  >/tmp/config
mv /tmp/config $LINUX_HOME/drivers/char/Kconfig

grep -v  "CONFIG_AM7X_HX170DEC"  $LINUX_HOME/drivers/char/Makefile > /tmp/config
mv /tmp/config   $LINUX_HOME/drivers/char/Makefile

echo "remove hantro source codes"
rm -Rf $LINUX_HOME/drivers/char/hx170dec

#****************************************
#      release lcm module 
#****************************************
#echo "release lcm module"
#echo "release lcm makefile&&kconfig"

#grep -v -E   "LCM|LVDS" $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE >/tmp/config
#mv /tmp/config   $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE

#sed -e '/config  AM7X_LCM/,/endchoice	/d' $LINUX_HOME/drivers/char/Kconfig  >/tmp/config
#mv /tmp/config $LINUX_HOME/drivers/char/Kconfig

#grep -v  "CONFIG_AM7X_LCM"  $LINUX_HOME/drivers/char/Makefile > /tmp/config
#mv /tmp/config   $LINUX_HOME/drivers/char/Makefile

#echo "remove lcm source codes"
#rm -Rf $LINUX_HOME/drivers/char/lcm

#****************************************
#      release dac module 
#****************************************

echo "release dac module"
echo "release dac makefile&&kconfig"

grep -v -E   "AM7X_DAC" $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE >/tmp/config
mv /tmp/config   $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE

sed -e '/config AM7X_DAC/,/	default m/d' $LINUX_HOME/drivers/char/Kconfig  >/tmp/config
mv /tmp/config $LINUX_HOME/drivers/char/Kconfig

grep -v  "CONFIG_AM7X_DAC"  $LINUX_HOME/drivers/char/Makefile > /tmp/config
mv /tmp/config   $LINUX_HOME/drivers/char/Makefile

echo "remove dac source codes"
rm -Rf $LINUX_HOME/drivers/char/dac

#****************************************
#      release spdif module 
#****************************************

echo "release spdif module"
echo "release spdif makefile&&kconfig"

grep -v -E   "AM7X_SPDIF" $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE >/tmp/config
mv /tmp/config   $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE

sed -e '/config  AM7X_SPDIF/,/	default m/d' $LINUX_HOME/drivers/char/Kconfig  >/tmp/config
mv /tmp/config $LINUX_HOME/drivers/char/Kconfig

grep -v  "CONFIG_AM7X_SPDIF"  $LINUX_HOME/drivers/char/Makefile > /tmp/config
mv /tmp/config   $LINUX_HOME/drivers/char/Makefile

echo "remove spdif source codes"
rm -Rf $LINUX_HOME/drivers/char/spdif

#****************************************
#      release 2d module 
#****************************************

echo "release 2d module"
echo "release 2d makefile&&kconfig"

grep -v -E   "CONFIG_AM7X_G" $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE >/tmp/config
mv /tmp/config   $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE

sed -e '/config  AM7X_G/,/	  This option can support 2D device. Say M to make a module and say Y to choose kernel support/d' $LINUX_HOME/drivers/char/Kconfig  >/tmp/config
mv /tmp/config $LINUX_HOME/drivers/char/Kconfig

grep -v  "CONFIG_AM7X_G"  $LINUX_HOME/drivers/char/Makefile > /tmp/config
mv /tmp/config   $LINUX_HOME/drivers/char/Makefile

echo "remove 2d source codes"
rm -Rf $LINUX_HOME/drivers/char/2d

#****************************************
#      release usb module 
#****************************************

#****************************************
#      release card update module 
#****************************************

echo "release card update module"
echo "release card update makefile&&kconfig"

grep -v -E   "AM7X_UPgrade" $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE >/tmp/config
mv /tmp/config   $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE

sed -e '/config AM7X_UPgrade/,/	  to the specific driver for your MMC interface./d' $LINUX_HOME/drivers/char/Kconfig  >/tmp/config
mv /tmp/config $LINUX_HOME/drivers/char/Kconfig

grep -v  "CONFIG_AM7X_UPgrade"  $LINUX_HOME/drivers/char/Makefile > /tmp/config
mv /tmp/config   $LINUX_HOME/drivers/char/Makefile

echo "remove card update source codes"
rm -Rf $LINUX_HOME/drivers/char/cardupdate



#****************************************
#      release recover module 
#****************************************

echo "release code recover module"
echo "release code recover makefile&&kconfig"

grep -v -E   "CONFIG_AM7X_RECOVER" $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE >/tmp/config
mv /tmp/config   $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE

sed -e '/config  AM7X_RECOVER/,/	  For AM recover support. Say M to make a module and say Y to choose kernel support/d' $LINUX_HOME/drivers/char/Kconfig  >/tmp/config
mv /tmp/config $LINUX_HOME/drivers/char/Kconfig

grep -v  "CONFIG_AM7X_RECOVER"  $LINUX_HOME/drivers/char/Makefile > /tmp/config
mv /tmp/config   $LINUX_HOME/drivers/char/Makefile

echo "remove code recover source codes"
rm -Rf $LINUX_HOME/drivers/char/recover

#****************************************
#      release card module 
#****************************************
echo "release card reader module"
echo "release card reader makefile&&kconfig"

grep -v -E   "CARD_DETECT|SD_CARD|MS_CARD|XD_CARD|CF_CARD|EMMC_CARD" $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE >/tmp/config
mv /tmp/config   $ROOT/scripts/cfg/$AM_CHIP_ID/$CONFIG_LINUX_FILE

grep -v -E "DetCard|SDCard|MSCard|XDCard|CFCard|eMMC|Test"  $LINUX_HOME/drivers/card/Kconfig > /tmp/config
mv /tmp/config   $LINUX_HOME/drivers/card/Kconfig

grep -v -E "DetCard|SDCard|MSCard|XDCard|CFCard|eMMC|Test"  $LINUX_HOME/drivers/card/Makefile > /tmp/config
mv /tmp/config   $LINUX_HOME/drivers/card/Makefile

echo "remove card reader source codes"
rm -Rf $LINUX_HOME/drivers/card/DetCard
rm -Rf $LINUX_HOME/drivers/card/eMMC
rm -Rf $LINUX_HOME/drivers/card/CFCard
rm -Rf $LINUX_HOME/drivers/card/SDCard
rm -Rf $LINUX_HOME/drivers/card/MSCard
rm -Rf $LINUX_HOME/drivers/card/XDCard
rm -Rf $LINUX_HOME/drivers/card/Test




echo "remove un release lib"
rm -rf $SDK_HOME/app
rm -rf $SDK_HOME/bootloader
#rm -rf $SDK_HOME/busybox
rm -rf $SDK_HOME/uclibc
rm -rf $SDK_HOME/glibc
rm -rf $SDK_HOME/library
rm -rf $SDK_HOME/wireless
rm -rf $CASE_HOME/library/wPlayer

echo "remove svn files"
rm -rf `find $SDK_HOME -name *.svn`
rm -rf `find $CASE_HOME -name *.svn`
rm -rf `find $SCRIPTS_HOME -name *.svn`
rm -rf `find $FLASH_HOME -name *.svn`
rm -rf $ROOT/.svn/

check_git_local_branch=`git branch -vv | cut -c 3- | awk '$3 !~/\[/ { print $1 }'`
if [[ -z $check_git_local_branch ]]
then
    rm -rf $ROOT/.git/
else
    echo "You have local branch as below!!!"
    echo $check_git_local_branch
    echo "Do you confirm to remove git repository? type YES and press [Enter]"
    read confirm_to_remove_git
    if [[ $confirm_to_remove_git == "YES" ]]
    then
        rm -rf $ROOT/.git/
    else
        echo "Abort release!!!!!!!!!!"
        exit 1
    fi
fi

make -C $LINUX_HOME clean

cd $LINUX_HOME
rm -rf .config
cd ../doc

