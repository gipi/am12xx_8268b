#!/bin/bash
#********************************************************************************
#   Script for Bash Shell of Redhat Linux
#   -------------------------------------
#	A basic utility library for BASH scripting.
#
#
#@Version:  1.0
#@File:  $resolve-alias.sh
#@Author:  Pan Ruochen (ijkxyz@msn.com)
#@History: 
#     1. The first version. (2009/12/24)
#*******************************************************************************
set -e -o pipefail
if [ ! -f "$1" ]; then false; fi
tmpmakefile=$(cat "$1";
cat << '__EOF__'
all: ; @echo $(foreach l,$(filter-out CURDIR SHELL MAKEFILE_LIST MAKEFLAGS .DEFAULT_GOAL,$(foreach i,$(.VARIABLES),$(if $(findstring $(origin $i),file),$i))),$l)
__EOF__
)
make -R -r -f <(echo "$tmpmakefile")
