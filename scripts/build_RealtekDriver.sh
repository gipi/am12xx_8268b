#!/bin/sh
#
# build RTL8192EU/8192DU/8192CU/8188EU driver.
# 
# 
#

do_8188eu_build()
{
	if [ -d $RTL8188EU_DIR -o -f $RTL8188EU_DIR ]; then
		rm -rf $RTL8188EU_DIR
		echo "  ########## remove "$RTL8188EU_DIR
	fi
	cd $NET_DIR
#	tar xf rtl8188EUS_linux_v4.1.7_8310.20140521_beta_release.tar.gz
	tar xf rtl8188EUS_linux_v4.3.0.6_12167.20140828.tar.gz
	cd $RTL8188EU_DIR
	make clean
	make 
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 	   	  8188eu driver make error 	  		      #"
		echo "#####################################################"
		exit 1;
	fi
	make install
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 			  8188eu driver make error 			  #"
		echo "#####################################################"
		exit 1;
	fi
	cd $TOPDIR/scripts
}

build_8188eu_driver()
{
	
	echo "#############################################################"
	echo "#                build 8188eus driver                       #"
	echo "#############################################################"
	if [ ! -d $RTL8188EU_DIR ]; then
		echo "  ########## "$RTL8188EU_DIR" not exist!!!"
		do_8188eu_build
	else
		if [ "$actions_arg" = "rebuild" ]; then
			echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
			echo "# if you modify makefile ,then you should rebuid 8188EU driver #"
			echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
			cd $RTL8188EU_DIR
			make clean
			make
			make install 
		else
			DRIVER_DIR=$TOPDIR/sdk/bin/1213/rootfs
			if [ -f $RTL8188EU_DIR/8188eu.ko -a -f $DRIVER_DIR/8188eu.ko ]; then
				cmp $DRIVER_DIR/8188eu.ko $RTL8188EU_DIR/8188eu.ko
				if [ $? -ne 0 ]; then
					echo "  ########## Driver file different, and rebuild RTL8188EUS driver!!!"
					do_8188eu_build
				else
					echo "  ########## RTL8188EUS driver have been built"
				fi
			else
				echo "  ########## Driver file not exist, and rebuild RTL8188EUS driver!!!"
				do_8188eu_build
			fi
		fi
	fi
}

do_8192eu_build()
{
	if [ -d $RTL8192EU_DIR -o -f $RTL8192EU_DIR ]; then
		rm -rf $RTL8192EU_DIR
		echo "  ########## remove "$RTL8192EU_DIR
	fi
	cd $NET_DIR
	#tar xf rtl8192EU_linux_v4.3.1.1_11320.20140505.tar.gz
	tar xf rtl8192EU_linux_v4.3.8_12406.20140929.tar.gz
	cd $RTL8192EU_DIR
	make clean
	make 
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 	   	  8192eu driver make error 	  		      #"
		echo "#####################################################"
		exit 1;
	fi
	make install
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 			  8192eu driver make error 			  #"
		echo "#####################################################"
		exit 1;
	fi
}

build_8192eu_driver()
{
	
	echo "#############################################################"
	echo "#                build 8192eu driver                       #"
	echo "#############################################################"
	if [ ! -d $RTL8192EU_DIR ]; then
		echo "  ########## "$RTL8192EU_DIR" not exist!!!"
		do_8192eu_build
	else
		if [ "$actions_arg" = "rebuild" ]; then
			echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
			echo "# if you modify makefile ,then you should rebuid 8192EU driver #"
			echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
			cd $RTL8192EU_DIR
			make clean
			make
			make install 
		else
			DRIVER_DIR=$TOPDIR/sdk/bin/1213/rootfs
			if [ -f $RTL8192EU_DIR/8192eu.ko -a -f $DRIVER_DIR/8192eu.ko ]; then
				cmp $DRIVER_DIR/8192eu.ko $RTL8192EU_DIR/8192eu.ko
				if [ $? -ne 0 ]; then
					echo "  ########## Driver file different, and rebuild RTL8192EUS driver!!!"
					do_8192eu_build
				else
					echo "  ########## RTL8192EUS driver have been built"
				fi
			else
				echo "  ########## Driver file not exist, and rebuild RTL8192EUS driver!!!"
				do_8192eu_build
			fi
		fi
	fi
}

do_8192cu_build()
{
	if [ -d $RTL8192CU_DIR -o -f $RTL8192CU_DIR ]; then
		rm -rf $RTL8192CU_DIR
		echo "  ########## remove "$RTL8192CU_DIR
	fi
	cd $NET_DIR
	tar xf rtl8188C_8192C_usb_linux_v4.0.5_11249.20140422.tar.gz
	cd $RTL8192CU_DIR
	make clean
	make 
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 	   	  8192cu driver make error 	  		      #"
		echo "#####################################################"
		exit 1;
	fi
	make install
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 			  8192cu driver make error 			  #"
		echo "#####################################################"
		exit 1;
	fi
}

build_8192cu_driver()
{
	
	echo "#############################################################"
	echo "#                build 8192cu driver                       #"
	echo "#############################################################"
	if [ ! -d $RTL8192CU_DIR ]; then
		echo "  ########## "$RTL8192CU_DIR" not exist!!!"
		do_8192cu_build
	else
		if [ "$actions_arg" = "rebuild" ]; then
			echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
			echo "# if you modify makefile ,then you should rebuid 8192CU driver #"
			echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
			cd $RTL8192CU_DIR
			make clean
			make
			make install 
		else
			DRIVER_DIR=$TOPDIR/sdk/bin/1213/rootfs
			if [ -f $RTL8192CU_DIR/8192cu.ko -a -f $DRIVER_DIR/8192cu.ko ]; then
				cmp $DRIVER_DIR/8192cu.ko $RTL8192CU_DIR/8192cu.ko
				if [ $? -ne 0 ]; then
					echo "  ########## Driver file different, and rebuild RTL8192cu driver!!!"
					do_8192cu_build
				else
					echo "  ########## RTL8192CU driver have been built"
				fi
			else
				echo "  ########## Driver file not exist, and rebuild RTL8192CU driver!!!"
				do_8192cu_build
			fi
		fi
	fi
}

do_8192du_build()
{
	if [ -d $RTL8192DU_DIR -o -f $RTL8192DU_DIR ]; then
		rm -rf $RTL8192DU_DIR
		echo "  ########## remove "$RTL8192DU_DIR
	fi
	cd $NET_DIR
	tar xf rtl8192DU_linux_v4.0.4_10867.20140321_beta.tar.gz
	cd $RTL8192DU_DIR
	make clean
	make 
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 	   	  8192du driver make error 	  		      #"
		echo "#####################################################"
		exit 1;
	fi
	make install
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 			  8192du driver make error 			  #"
		echo "#####################################################"
		exit 1;
	fi
}

build_8192du_driver()
{
	
	echo "#############################################################"
	echo "#                build 8192du driver                       #"
	echo "#############################################################"
	if [ ! -d $RTL8192DU_DIR ]; then
		echo "  ########## "$RTL8192DU_DIR" not exist!!!"
		do_8192du_build
	else
		if [ "$actions_arg" = "rebuild" ]; then
			echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
			echo "# if you modify makefile ,then you should rebuild 8192DU driver #"
			echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
			cd $RTL8192DU_DIR
			make clean
			make
			make install 
		else
			DRIVER_DIR=$TOPDIR/sdk/bin/1213/rootfs
			if [ -f $RTL8192DU_DIR/8192du.ko -a -f $DRIVER_DIR/8192du.ko ]; then
				cmp $DRIVER_DIR/8192du.ko $RTL8192DU_DIR/8192du.ko
				if [ $? -ne 0 ]; then
					echo "  ########## Driver file different, and rebuild RTL8192du driver!!!"
					do_8192du_build
				else
					echo "  ########## RTL8192DU driver have been built"
				fi
			else
				echo "  ########## Driver file not exist, and rebuild RTL8192DU driver!!!"
				do_8192du_build
			fi
		fi
	fi
}

do_8189es_build()
{
	cd $RTL8189ES_DIR
	make clean
	make 
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 	   	  8189es driver make error 	  		      #"
		echo "#####################################################"
		exit 1;
	fi
	make install
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 			  8189es driver make error 			  #"
		echo "#####################################################"
		exit 1;
	fi
}

build_8189es_driver()
{


	
	echo "#############################################################"
	echo "#                build 8189es driver                       #"
	echo "#############################################################"
	if [ ! -f $RTL8189ES_DIR/8189es.ko ]; then
		echo "  ########## "$RTL8189ES_DIR/8189es.ko" not exist!!!"
		do_8189es_build
	else
			echo  "##########"   "$RTL8189ES_DIR/8189es.ko"" exist!!!"
			DRIVER_DIR=$TOPDIR/sdk/bin/1213/rootfs
			if [ -f $RTL8189ES_DIR/8189es.ko -a -f $DRIVER_DIR/8189es.ko ]; then
				cmp $DRIVER_DIR/8189es.ko $RTL8189ES_DIR/8189es.ko
				if [ $? -ne 0 ]; then
					echo "  ########## Driver file different, and rebuild RTL8189es driver!!!"
					do_8189es_build
				else
					echo "  ########## RTL8189ES driver have been built"
				fi
			else
				echo "  ########## Driver file not exist, and rebuild RTL8189ES driver!!!"
				do_8189es_build
			fi
	fi
	
}



do_8811au_build()
{


	if [ -d $RTL8811AU_DIR -o -f $RTL8811AU_DIR ]; then
		rm -rf $RTL8811AU_DIR
		echo "  ########## remove " $RTL8811AU_DIR
	fi
	cd $NET_DIR
	pwd
	tar xf  rtl8811AU_linux_v4.3.0_10674.20140509.tar.gz
	cd $RTL8811AU_DIR
	make clean
	make 
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 	   	  8811au driver make error 	  		      #"
		echo "#####################################################"
		exit 1;
	fi
	make install
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 		  8811au driver make error 			  #"
		echo "#####################################################"
		exit 1;
	fi


}


build_8811au_driver()
{
	echo "#############################################################"
	echo "#                build 8811au driver                       #"
	echo "#############################################################"

	if [ ! -f $RTL8811AU_DIR/8821au.ko ]; then
		echo "  ########## "$RTL8811AU_DIR/8821au.ko" not exist!!!"
		do_8811au_build
	else
			echo  "##########"   "$RTL8811AU_DIR/8821au.ko"" exist!!!"
			DRIVER_DIR=$TOPDIR/sdk/bin/1213/rootfs
			if [ -f $RTL8811AU_DIR/8821au.ko -a -f $DRIVER_DIR/8821au.ko ]; then
				cmp $DRIVER_DIR/8821au.ko $RTL8811AU_DIR/8821au.ko
				if [ $? -ne 0 ]; then
					echo "  ########## Driver file different, and rebuild RTL8811AU driver!!!"
					do_8811au_build
				else
					echo "  ########## RTL8811AU driver have been built"
				fi
			else
				echo "  ########## Driver file not exist, and rebuild RTL8811AU driver!!!"
				do_8811au_build
			fi
	fi


}

do_8821cu_build()
{
	if [ -d $RTL8821CU_DIR -o -f $RTL8821CU_DIR ]; then
		rm -rf $RTL8821CU_DIR
		echo "  ########## remove " $RTL8821CU_DIR
	fi
	cd $NET_DIR
	pwd
	tar xf  rtl8821CU_WiFi_linux_v5.2.5.2_24506.20171018_COEX20170310-1212.tar.gz
	cd $RTL8821CU_DIR
	make clean
	make 
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 	   	  8821cu driver make error 	  		      #"
		echo "#####################################################"
		exit 1;
	fi
	make install
	if [ $? -ne 0 ]; then
		echo "#####################################################"
		echo "# 		  8821cu driver make error 			  #"
		echo "#####################################################"
		exit 1;
	fi
}

build_8821cu_driver()
{
	echo "#############################################################"
	echo "#                build 8821cu driver                       #"
	echo "#############################################################"

	if [ ! -f $RTL8821CU_DIR/8821cu.ko ]; then
		echo "  ########## "$RTL8821CU_DIR/8821cu.ko" not exist!!!"
		do_8811au_build
	else
			echo  "##########"   "$RTL8821CU_DIR/8821cu.ko"" exist!!!"
			DRIVER_DIR=$TOPDIR/sdk/bin/1213/rootfs
			if [ -f $RTL8821CU_DIR/8821cu.ko -a -f $DRIVER_DIR/8821cu.ko ]; then
				cmp $DRIVER_DIR/8821cu.ko $RTL8821CU_DIR/8821cu.ko
				if [ $? -ne 0 ]; then
					echo "  ########## Driver file different, and rebuild RTL8821CU driver!!!"
					do_8821cu_build
				else
					echo "  ########## RTL8821CU driver have been built"
				fi
			else
				echo "  ########## Driver file not exist, and rebuild RTL8821CU driver!!!"
				do_8821cu_build
			fi
	fi
}

remove_original_driver_file()
{
	cd $NET_DIR
	rm -rf $RTL8188EU_DIR
	rm -rf $RTL8192EU_DIR
	rm -rf $RTL8192CU_DIR
	rm -rf $RTL8192DU_DIR
	echo "remove original driver file"
}
input_arg=$1
actions_arg=
WORK_DIR=$(cd $(dirname $0) && pwd)
TOPDIR=$(cd $WORK_DIR/.. && pwd)
NET_DIR=$TOPDIR/sdk/library/net
#RTL8188EU_DIR=$NET_DIR/rtl8188EUS_linux_v4.1.7_8310.20140521_beta
RTL8188EU_DIR=$NET_DIR/rtl8188EUS_linux_v4.3.0.6_12167.20140828
#RTL8192EU_DIR=$NET_DIR/rtl8192EU_linux_v4.3.1.1_11320.20140505
RTL8192EU_DIR=$NET_DIR/rtl8192EU_linux_v4.3.8_12406.20140929
RTL8192CU_DIR=$NET_DIR/rtl8188C_8192C_usb_linux_v4.0.5_11249.20140422
RTL8192DU_DIR=$NET_DIR/rtl8192DU_linux_v4.0.4_10867.20140321_beta
RTL8189ES_DIR=$NET_DIR/rtl8189ES_linux_v4.3.10.1_13373.20150129
RTL8811AU_DIR=$NET_DIR/rtl8811AU_linux_v4.3.0_10674.20140509
RTL8821CU_DIR=$NET_DIR/rtl8821CU_WiFi_linux_v5.2.5.2_24506.20171018_COEX20170310-1212

do_main()
{
	echo "######################################################"
	echo "#                   REALTEK DRIVER                   #"
	echo "######################################################"
	echo "TOPDIR: "$TOPDIR
	actions_arg="rebuild"
	if [ "$input_arg" = "8188eu" ]; then
		build_8188eu_driver
	elif [ "$input_arg" = "8192cu" ]; then
		build_8192cu_driver
	elif [ "$input_arg" = "8192du" ]; then
		build_8192du_driver
	elif [ "$input_arg" = "8192eu" ]; then
		build_8192eu_driver
	elif [ "$input_arg" = "8189es" ]; then
		build_8189es_driver
	elif [ "$input_arg" = "8811au" ]; then
		build_8811au_driver
	elif [ "$input_arg" = "8821cu" ]; then
		build_8821cu_driver
	else
		remove_original_driver_file
		build_8188eu_driver
		build_8192eu_driver
		build_8192cu_driver
		build_8192du_driver
		
	fi
	
	echo "######################################################"
	echo "#                    Successful                      #"
	echo "######################################################"
}

do_main
