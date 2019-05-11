#!/bin/bash
set -e -o pipefail

new_dep=$($MKDEP_CMD)
if [ ! -f $DEPFILE ]; then
	echo changed;
elif [ "$new_dep" = "$(cat $DEPFILE)" ]; then
	echo unchanged;
	exit 0;
else
	echo changed;
fi
echo "$new_dep" > $DEPFILE

