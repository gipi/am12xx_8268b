1. linuxtool -> export specific lnx file
. sh am_sdk_config.sh AM_8251 or AM_13
. modify bitmask in sdk\library\wifi_subdisplay\ez_remoteconfig.h
. make all
. go to sdk/bootloader/nand/scripts/ and make bootloader
. go to case/apps/cgi-bin/ and do "make release"
. sh mkfs.sh ezcastpro
. with MPtool, 8250 use "fwimage_1213_ezcastpro.cfg"
			,8251 use "fwimage_1213_8251_ezcast.cfg" 
