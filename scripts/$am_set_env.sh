#!/bin/bash
#
#  AUTHOR:  PRC
#  DATE:    Jul 7, 2008
#

if [ -n "$__GLOBAL_GUID__" ]; then
	return 0
fi
	
export __GLOBAL_GUID__=c317924c-63ab-4902-80a5-8db495950877
source ./\$utils.sh

function parse_cfg()
{
	if [ -z "$1" ]; then
		echo "No configuration file to parse" >&2;
		exit 1;
	fi
	eval -- $(strip_comments $1 | sed 's/\([^ 	]\+\)[ 	]*[=][ 	]*\(.*\)/export \1=\2;/')
}

parse_cfg global.cfg
if [ -f local.cfg ]; then 
	parse_cfg local.cfg
fi

if [ -z "$TOP_DIR" ]; then
	echo TOP_DIR is empty >&2
	exit 1
fi

