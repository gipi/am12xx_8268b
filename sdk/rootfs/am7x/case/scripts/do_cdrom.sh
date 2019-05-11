#script
lib_dir="/lib/modules/2.6.27.29"

rmmod -f g_file_storage

if [ -r "/sys/module/am7x_udc" ]; then
	rmmod -f am7x_udc
	echo "am7x udc"
	sleep 1
	insmod $lib_dir/am7x_udc.ko		
elif [ -r "/sys/module/am7x_udc_next" ]; then
	rmmod -f am7x_udc_next
	echo "am7x udc next"
	sleep 1
	insmod $lib_dir/am7x_udc_next.ko				
fi
	
#lsmod
