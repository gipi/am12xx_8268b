#!/bin/bash
#
# Use to get the firmware version for the git server.
# Calculating like this,
#    version = base_svn_version + number_of_git_commits.
#
myPath=$(pwd)/$(dirname $BASH_SOURCE)
function get_version_from_git()
{
    base_svn_version=13545
    total_commit_log=`git log --pretty=oneline | wc -l`
    let transmit_svn_version=base_svn_version+total_commit_log-1
    echo $transmit_svn_version
}

function get_ota_version()
{
#	__version=`cat $myPath/../sdk/rootfs/etc/version.conf | grep FIRMWARE | cut -d= -f2`
	__version=$(get_version_from_git)
	echo $__version"000"
}

# for test only
#version=$(get_version_from_git)
#echo $version

