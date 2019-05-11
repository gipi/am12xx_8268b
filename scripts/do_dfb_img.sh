#! /bin/bash

set -e -o pipefail

__root_dir=$(pwd)/../
__sdk_dir=${__root_dir}/sdk
__dfb_install_dir=${__sdk_dir}/library/directfb/directfb_install/

try_umount()
{
	set +e
	sudo umount $1 >/dev/null 2>&1
	set -e
}


__current_dir=dfb_test
if [ -d "${__current_dir}" ];then
    echo "${__current_dir} exist"
else
    echo "${__current_dir} not exist, make it"
    mkdir dfb_test
fi


try_umount ${__current_dir}

__init_size=$(( 30*1024*1024 ))
__fs_image=dfb_test.img

if [ -f "$__fs_image" ];then
    rm -rf $__fs_image
fi
dd if=/dev/zero of=$__fs_image bs=$__init_size count=1
mkfs.ext2 -F -m0 $__fs_image;

sudo mount -t ext2 -o loop $__fs_image $__current_dir
cd $__current_dir

#*******************************************
# copy the dfb related libfiles here
#*******************************************
cp -dpfr ${__dfb_install_dir}/lib/*.so* .

__module_dir=directfb-1.4-5
if [ -e ${__module_dir} ]
then
    rm -rf ${__module_dir}
fi
cp -dpfr ${__dfb_install_dir}/lib/directfb-1.4-5 ${__module_dir}

#cp -dpfr ${__dfb_install_dir}/bin/* .

cd ../
try_umount ${__current_dir}
