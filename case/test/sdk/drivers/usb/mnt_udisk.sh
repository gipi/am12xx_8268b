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

mdev -s
for i in 1 2 3 4
  do
    if [ -e /dev/sda$i ]; then
      mount /dev/sda$i /mnt/usb
    else
      break
    fi
  done

if [ $i -eq 1 ]; then
    echo "disk no partition"
    mount /dev/sda /mnt/usb
fi

echo
#end