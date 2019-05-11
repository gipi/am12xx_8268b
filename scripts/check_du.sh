#/bin/bash
#
# Copyright (C) 2013 Actions MicroEletronics Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# Create filesystem images described by a configuration file, default
# to be AM_FS.cfg.
#
#

set -e
set -o pipefail


full_path=$(pwd)/$1

#echo ">>>>>>>>>>> full_path=$full_path"


du -a $full_path >tmp.txt

rm -rf check_result.txt

while read line
do
    file_or_dir=$(echo $line | awk '{print $2}')
    
	if [ ! -d $file_or_dir ];then
	    echo $line >>check_result.txt
	fi
done <tmp.txt

#rm -rf tmp.txt

#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# sort by file size
#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

cat check_result.txt | sort -nr >check_sorted.txt


#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# get sorted total size
#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
total_size=0
while read line
do
    item_size=$(echo $line | awk '{print $1}')
    let total_size+=item_size
done <check_sorted.txt

echo ">>>>>>>>>>>>>>>>>>>total size is : $total_size"



