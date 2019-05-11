#! /bin/bash
set -e -o pipefail

function translate_chipid()
{
    for i in $IF_NAME
    do
        if [ $i = "$1" ]; then
            echo 12${1##AM_}
            return 0
        fi
    done

    #>>>>>>>>>>>>>>>>>>>>>>>>>
    # 1213:AM_8250 AM_8251 AM_8253
    #>>>>>>>>>>>>>>>>>>>>>>>>>
    if [ "$1" == "AM_8250" ];then
        echo 1213
        return 0;
    fi
	
    if [ "$1" == "AM_8251" ];then
        echo 1213
        return 0;
    fi

    if [ "$1" == "AM_8253" ];then
        echo 1213
        return 0;
    fi
 
    if [ "$1" == "ezwire" ];then
        echo 1213
        return 0;
    fi
    if [ "$1" == "AM_8258" ];then
        echo 1213
        return 0;
    fi
    if [ "$1" == "AM_8258_4M_ADB_MirrorOnly" ];then
        echo 1213
        return 0;
    fi
    if [ "$1" == "AM_8258_4M" ];then
        echo 1213
        return 0;
    fi
    if [ "$1" == "AM_8258B_Wire" ];then
        echo 1213
        return 0;
    fi
    if [ "$1" == "AM_8258N_Wire" ];then
        echo 1213
        return 0;
    fi
    if [ "$1" == "AM_8258B" ];then
        echo 1213
        return 0;
    fi
    if [ "$1" == "AM_8252n" ];then
        echo 1213
        return 0;
    fi
	
    if [ "$1" == "AM_8252n_8M" ];then
        echo 1213
        return 0;
    fi
   if [ "$1" == "AM_8252n_4M" ];then
        echo 1213
        return 0;
    fi
      if [ "$1" == "AM_8268_4M" ];then
        echo 1213
        return 0;
    fi
    if [ "$1" == "AM_8268B_Wire" ];then
        echo 1213
        return 0;
    fi
    if [ "$1" == "AM_8268B" ];then
        echo 1213
        return 0;
    fi
    if [ "$1" == "AM_8268" ];then
        echo 1213
        return 0;
    fi
    if [ "$1" == "AM_8252n_ezcast" ];then
        echo 1213
        return 0;
    fi
    if [ "$1" == "probox" ];then
        echo 1213
        return 0;
    fi
   
    return 1
}

function translate_subchipid()
{
    #>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    # 1213:AM_8250 AM_8251 AM_8253 ezwire
    #>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    if [ "$1" = "AM_8250" ];then
        echo 8250
        return 0;
    fi
	
    if [ "$1" = "AM_8251" ];then
        echo 8251
        return 0;
    fi

    if [ "$1" = "AM_8253" ];then
        echo 8251
        return 0;
    fi

	if [ "$1" = "ezwire" ];then
        echo ezwire
        return 0;
    fi
   if [ "$1" = "AM_8258" ];then
        echo 8258
        return 0;
   fi
   if [ "$1" = "AM_8258_4M_ADB_MirrorOnly" ];then
        echo 8258
        return 0;
    fi
    if [ "$1" = "AM_8258_4M" ];then
        echo 8258
        return 0;
    fi
    if [ "$1" = "AM_8258B_Wire" ];then
        echo 8258
        return 0;
    fi
    if [ "$1" = "AM_8258N_Wire" ];then
        echo 8258
        return 0;
    fi
    if [ "$1" = "AM_8258B" ];then
        echo 8258
        return 0;
    fi
     if [ "$1" = "AM_8252n" ];then
        echo 8251
        return 0;
    fi
    if [ "$1" = "AM_8252n_8M" ];then
        echo 8251
        return 0;
    fi
     if [ "$1" = "AM_8252n_4M" ];then
        echo 8251
        return 0;
    fi
    if [ "$1" = "AM_8252n_ezcast" ];then
        echo 8251
        return 0;
    fi
    if [ "$1" = "AM_8268_4M" ];then
        echo 8268
        return 0;
    fi
    if [ "$1" = "AM_8268B_Wire" ];then
        echo 8268
        return 0;
    fi
    if [ "$1" = "AM_8268B" ];then
        echo 8268
        return 0;
    fi
    if [ "$1" = "AM_8268" ];then
        echo 8268
        return 0;
    fi
    if [ "$1" == "probox" ];then
        echo probox
        return 0;
    fi
   
    return 0
}

function translate_suffix()
{
    #>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    # 1213:AM_8250 AM_8251 AM_8253 ezwire
    #>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    if [ "$1" = "AM_8250" ];then
        echo 8250
        return 0;
    fi
	
    if [ "$1" = "AM_8251" ];then
        echo 8251
        return 0;
    fi

    if [ "$1" = "AM_8253" ];then
        echo 8251
        return 0;
    fi

    if [ "$1" = "ezwire" ];then
        echo ezwire
        return 0;
    fi
    if [ "$1" = "AM_8258" ];then
        echo 8258
        return 0;
    fi
    if [ "$1" = "AM_8258_4M_ADB_MirrorOnly" ];then
        echo 8258_4M_ADB_MirrorOnly
        return 0;
    fi
    if [ "$1" = "AM_8258_4M" ];then
        echo 8258_4M
        return 0;
    fi
    if [ "$1" = "AM_8258B_Wire" ];then
        echo 8258B_Wire
        return 0;
    fi
    if [ "$1" = "AM_8258N_Wire" ];then
        echo 8258N_Wire
        return 0;
    fi
    if [ "$1" = "AM_8258B" ];then
        echo 8258B
        return 0;
    fi
    if [ "$1" = "AM_8252n" ];then
        echo 8252n
        return 0;
    fi
	
    if [ "$1" = "AM_8252n_8M" ];then
        echo 8252n_8M
        return 0;
    fi
    if [ "$1" = "AM_8252n_4M" ];then
        echo 8252n_4M
        return 0;
    fi
    if [ "$1" = "AM_8252n_ezcast" ];then
        echo 8252n_ezcast
        return 0;
    fi
    if [ "$1" = "AM_8268_4M" ];then
        echo 8268_4M
        return 0;
    fi
    if [ "$1" = "AM_8268B_Wire" ];then
        echo 8268B_Wire
        return 0;
    fi
    if [ "$1" = "AM_8268B" ];then
        echo 8268B
        return 0;
    fi
    if [ "$1" = "AM_8268" ];then
        echo 8268
        return 0;
    fi
    if [ "$1" == "probox" ];then
        echo probox
        return 0;
    fi
	
	if [ "$1" == "projector" ];then
        echo projector
        return 0;
    fi
   
    return 0
}

IF_NAME="AM_03 AM_11 AM_20 AM_13"
IF_NAME_SUB="AM_8251"
IF_NAME_SUB_8253="AM_8253"
IF_NAME_SUB_EZWIRE="ezwire"
IF_NAME_SUB_8258="AM_8258"
IF_NAME_SUB_8258B="AM_8258B"
IF_NAME_SUB_8258_4M="AM_8258_4M"
IF_NAME_SUB_8258_4M_ADB_MirrorOnly="AM_8258_4M_ADB_MirrorOnly"
IF_NAME_SUB_8258B_Wire="AM_8258B_Wire"
IF_NAME_SUB_8258N_Wire="AM_8258N_Wire"
IF_NAME_SUB_8252N="AM_8252n"
IF_NAME_SUB_8252N_4M="AM_8252n_4M"  
IF_NAME_SUB_8252N_8M="AM_8252n_8M"  
IF_NAME_SUB_8252N_EZCAST="AM_8252n_ezcast"
IF_NAME_SUB_8268_4M="AM_8268_4M"
IF_NAME_SUB_8268B_Wire="AM_8268B_Wire"
IF_NAME_SUB_8268B="AM_8268B"
IF_NAME_SUB_82568="AM_8268"
IF_NAME_SUB_probox="probox"
function show_usage_and_exit()
{
	cat >&2 <<__USAGE__
usage: $(basename $0) CHIP_TYPE
CHIP_TYPE can be $IF_NAME $IF_NAME_SUB $IF_NAME_SUB_8253 $IF_NAME_SUB_EZWIRE $IF_NAME_SUB_PROBOX $IF_NAME_SUB_8252N $IF_NAME_SUB_8252N_4M $IF_NAME_SUB_8252N_8M $IF_NAME_SUB_8252N_EZCAST $IF_NAME_SUB_8258_4M $IF_NAME_SUB_8258B $IF_NAME_SUB_8258B_Wire $IF_NAME_SUB_8258N_Wire $IF_NAME_SUB_8258_4M_ADB_MirrorOnly $IF_NAME_SUB_82568 $IF_NAME_SUB_8268_4M $IF_NAME_SUB_8268B_Wire $IF_NAME_SUB_8268B
__USAGE__
	exit 1
}

function resort()
{
	local file
	if [ -z "$1" ]; then
		file=/dev/fd/0
	else
		file="$1"
	fi
	awk -- '
$1~/\.a$/  {a = a $0 "\n"; next}
$1~/\.so$/ {s = s $0 "\n"; next}
{x = x $0 "\n"; next}
END { printf("%s%s%s",a,s,x) }' "$file"
}

function ly_filter()
{
	local dir target path
	sed -e 's:\(\./\+\)\+::g' -e 's:/\+:/:g' -e 's:\(/$\)::g' "$1" | while read dir
	do
		path=$topdir/$dir
		if [ -f $path/Makefile ]; then
			target=$($AM_MAKE -C $path query-target)
			if [ "$target"A = ""A ];then
				exit 1
			fi
			echo "$target $dir"
		fi
	done | resort
}

if [ $# -lt 1 ]; then
	show_usage_and_exit
fi

source ./\$am_set_env.sh
topdir=$(cd $TOP_DIR && pwd)

am_chip_id=$(translate_chipid $1) || show_usage_and_exit
am_chip_id_minor=$(translate_subchipid $1)
config_suffix=$(translate_suffix $1)

#export_version version_cfg.h
export_version_git version_cfg.h


CFG_FILE=config.mak
MC_FILE=mconfig.h
uclibc_flag=`cat $MC_FILE | grep -q MODULE_CONFIG_UCLIBC && cat $MC_FILE | grep MODULE_CONFIG_UCLIBC | awk '{print $3}' || echo 0`
echo "uclibc_flag: "$uclibc_flag
if [ $uclibc_flag  -eq 1 ];then
	uclibc=" -muclibc "
else
	uclibc="" 
fi
cat >$CFG_FILE <<__CONFIG_MAK__
#--------------------------------------------------------#
# DONNOT EDIT THIS FILE MANUALLY!!!!
#
# \$Id\$  $CFG_FILE
# CREATED ON  $(date)
#--------------------------------------------------------#
DEV_CASE_DIR     := $AM_CASE_DIR
DEV_SDK_DIR      := $AM_SDK_DIR
AM_CHIP_ID_MAIN  := $am_chip_id
AM_CHIP_ID_MINOR  := $am_chip_id_minor
CFG_ALIAS        := $CFG_ALIAS
CFG_MODS_SDK     := $CFG_MODS_SDK
CFG_MODS_CASE    := $CFG_MODS_CASE
MAKE             := $AM_MAKE
TOPDIR           := $TOP_DIR
NEED_FOR_SPEED   := N
CFLAG_LIBC       := $uclibc

#--------------------------------------------------------#
__CONFIG_MAK__

#************************************************************
# Generate compile options from wol config file(wolconfig.h)
#************************************************************
wolconfig_file_path=$(pwd)/wolconfig.h
do_wolconfig_process ${wolconfig_file_path} 1

#********************************************************
# Generate compile options from module config file.
#********************************************************
mconfig_file_path=$(pwd)/mconfig.h
do_module_process ${mconfig_file_path} 1

mode=666
linux_dir=$TOP_DIR/$GNU_LINUX_DIR
install -m$mode $TOP_DIR/case/cfg/case_config.mak ./ &&
install -m$mode cfg/$am_chip_id/modules_install.cfg ./ &&
ly_filter cfg/$am_chip_id/AM_sdk.cfg > AM_sdk.cfg &&
ly_filter $TOP_DIR/case/cfg/AM_case.cfg > AM_case.cfg &&
chmod $mode AM_sdk.cfg AM_case.cfg &&
./\$get-rules.sh > .Rules

if [ $? -ne 0 ];then
	exit 1
fi

if test -d $linux_dir; then
    if [ "$config_suffix" != "" ];then
        cat cfg/$am_chip_id/config.linux.$config_suffix > $linux_dir/.config;
    else
	    cat cfg/$am_chip_id/config.linux > $linux_dir/.config; 
	fi
fi

if test -d $TOP_DIR/sdk/library ; then
    cp -f Makefile.dev Makefile
else
    cp -f Makefile.release Makefile
fi

