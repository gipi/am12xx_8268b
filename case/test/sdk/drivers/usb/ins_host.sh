#!/bin/bash
#-------------------------------------------------------------------------------
#-                                                                            --
#-       This software is confidential and proprietary and may be used        --
#-        only as expressly authorized by a licensing agreement from          --
#-                                                                            --
#-                            Hantro Products Oy.                             --
#-                                                                            --
#-                   (C) COPYRIGHT 2006 HANTRO PRODUCTS OY                    --
#-                            ALL RIGHTS RESERVED                             --
#-                                                                            --
#-                 The entire notice above must be reproduced                 --
#-                  on all copies and should not be removed.                  --
#-                                                                            --
#-------------------------------------------------------------------------------
#-
#--  Description : Load device driver
#--
#--------------------------------------------------------------------------------
#--
#--  Version control information, please leave untouched.
#--
#--  $RCSfile: driver_load.sh,v $
#--  $Revision: 1.1 $
#--  $Date: 2006/12/20 15:17:10 $
#--
#--------------------------------------------------------------------------------

module="/lib/modules/2.6.27.29/"

#insert module
insmod $(module)am7x_uoc.ko detect_gpio=34
insmod $(module)am7x_hcd.ko
insmod $(module)usb-storage.ko

if [ $? -eq 0 ]; then
	echo "storage ins ok"
fi

#the end
echo
