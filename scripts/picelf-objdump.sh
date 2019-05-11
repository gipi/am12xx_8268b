#/bin/bash
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# An enhanced objdump tool.
#
# Changelog:
#
#  2010/7/15: Pan Ruochen <ijkxyz@msn.com>
#				- The first available version

if [ ! -f $1 ]; then
	exit 2
fi
arch=$(file -L $1 | grep -o 'MIPS\|ARM'|tr '[A-Z]' '[a-z]')
case $arch in
*mips*) arch=mips;;
*arm*)  arch=arm;;
*) exit 3
esac

save_ifs="$IFS"
IFS=:
for d in $PATH
do
	objdump=$(ls $d/*$arch*objdump 2>/dev/null)
	if test -n "$objdump"; then
		CROSS=${objdump%%objdump}
		break;
	fi
done
IFS=$save_ifs

#echo "GNU objdump found: $gcc" >&2
#echo "$CROSS: $CROSS" >&2

if test -z "$objdump"; then
	objdump=${CROSS}objdump
fi

TMP_LST_FILE=~1.lst
TMP_MAP_FILE=~2.map
TMP_GP_FILE=~3.gp

${CROSS}objdump -d $1 > $TMP_LST_FILE &&
${CROSS}readelf -A $1 > $TMP_GP_FILE &&
${CROSS}nm -D $1 > $TMP_MAP_FILE &&
gawk '
FILENAME==ARGV[1] {
	symbols[$1] = $3;
}
FILENAME==ARGV[2]{
	if($1=="Local"  && $2=="entries:") {
		flag = 1
		next
	} else if($1=="Global" && $2=="entries:") {
		flag = 2
		next
	}
	if( $1 ) {
		if ( flag == 1 ) {
			if ( $3 in symbols )
				name = symbols[$3];
			else
				name = $3
			gp_local[$2] = name
		} else if( flag == 2 ) {
			gp_global[$2] = $7
		}
	}
}
FILENAME==ARGV[3] {
	n = match($0, /[ls]w	t9,(.+\(gp\))/, m);
	if(n > 0) {
		gp = substr($0, m[1,"start"], m[1,"length"]);
		if( gp in gp_local )
			name = gp_local[gp];
		else if ( gp in gp_global )
			name = gp_global[gp];
		t9 = name
	}
	if( name != "" )
		printf("%s /*PRC:%s*/\n", $0, name)
	else {
		if( $0 ~ /jalr	t9/ && t9 != "" ) {
			printf("%s <%s>\n", $0, t9)
			t9 = ""
		} else
			print $0
	}
	name = "";
} ' $TMP_MAP_FILE $TMP_GP_FILE $TMP_LST_FILE &&
rm -f $TMP_MAP_FILE $TMP_GP_FILE $TMP_LST_FILE
