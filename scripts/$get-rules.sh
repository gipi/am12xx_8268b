#!/bin/bash
#********************************************************************************
#   Script for Bash Shell of Redhat Linux
#   -------------------------------------
#	A basic utility library for BASH scripting.
#
#
#@Version:  1.0
#@File:  $get-rules.sh
#@Author:  Pan Ruochen (ijkxyz@msn.com)
#@History: 
#     1. The first version. (2009/12/24)
#*******************************************************************************
set -e -o pipefail
source ./\$am_set_env.sh

function get_configs()
{
	local target dir path
	while read dir
	do
		path=$TOPDIR/$dir
		if [ ! -d $path -o ! -f $path/Makefile ]; then continue; fi;
		target=$($AM_MAKE -C $path query-target)
		echo $target $dir
	done < "$1"
}

function make_configs()
{
	local config_sdk config_case
	local format awk_text

	config_case=$(cat $CFG_MODS_CASE)
	config_sdk=$(cat $CFG_MODS_SDK)
	echo "$config_case" $'\n' "$config_sdk" | awk '$1!=""{ printf(".PHONY: %s\n", $1)
	printf("%s: ; @echo \"  ENTER %s\"&&$(MAKE) -C $(TOPDIR)/%s $(MAKE_ACTION)&&echo\n\n", $1,$3,$3);
}'
	format='BEGIN{printf("%s = ")}$1!=""{printf("%%s ",$1)}END{print ""}'
	awk_text=$(printf "$format" case-targets)
	awk "$awk_text" <<<"$config_case"
	awk_text=$(printf "$format" sdk-targets)
	awk "$awk_text" <<<"$config_sdk"

	cat << __LINUX__
.PHONY: .case .sdk .targets .casetargets .sdktargets
.case: \$(case-targets)
.sdk:  \$(sdk-targets)
.targets:      ; @echo \$(case-targets) \$(sdk-targets)
.casetargets: ; @echo \$(case-targets)
.sdktargets:  ; @echo \$(sdk-targets)
__LINUX__
}

TOPDIR=$(cd ..&&pwd)
make_configs
