#! /usr/bin/gawk -f
#********************************************************************************
#   Script for Bash Shell of Cygwin
#   -------------------------------
#	A basic utility library for GAWK scripting.
#
#
#@Version:  1.0
#@File:  $util-libs.awk
#@Author:  Pan Ruochen (reachpan@actions-micro.com)
#@History: 
#     1. The first version was created on 2008/8/8
#*******************************************************************************

function get_unique_path(path,  __ARGVEND__, i,n,z,a,level,abs)
{
	n = split(path, a, "/");
	ln = 0;
	for(i=1; i<=n; i++) {
		switch(a[i]) {
		case "":
			break;
		case ".":
			break;
		case "..":
			ln--;
			break;
		default:
			level[++ln] = a[i];
			break;
		}
	}

	if( a[1] == "" )
		abs = "/";
	for(i=1; i<=ln; i++) {
		abs = abs level[i];
		if( i != ln )
			abs = abs "/";
	}
	return abs;
}

