#!/bin/sh

#-------------------------------------------------------------------------
#
#  Show symbolic names for each address of kernel dump stack information. 
#
#  AUTHOR: Pan Ruochen <reachpan@actions-micro.com>
#  History:
#   1.  The first version was created on 2010/8/24.
#
#-------------------------------------------------------------------------

CT_FILE=$1
DS_FILE=$2
FILE_SEP='^--^--@@@@@@@@@@@@@@@@@@@@@@--^--^'

if [ -z "$DS_FILE" ]; then DS_FILE=vmlinux.lst; fi
if [ -z "$CT_FILE" ]; then CT_FILE=calltrace.txt; fi

{
	awk '{if(NF==2)print $0;last=$0}END{print last}' $DS_FILE;
	echo $FILE_SEP;
	cat $CT_FILE;
} | gawk -v CURFILE=1 -v FILE_SEP="$FILE_SEP" '
func bin_search(key, array, count, __ARGV_END__, front, back, i, a, b)
{
	front = 1; back = count;
	while(front <= back) {
		i = (front + back) / 2;
		i = sprintf("%d", i)
		a = strtonum("0x" array[i]);
		b = strtonum("0x" array[i+1]);
		if(key >= a && key < b)
			return i;
		else if(key < a)
			back = i-1;
		else
			front = i+1;
	}
	return -1;
}
CURFILE==1{
	if( $0 == FILE_SEP ) {
		if( CURFILE==1 ) {
			end = sprintf("%08x", strtonum("0x" end)+4)
			addr_tab[nr_sym+1] = end;
			func_tab[end]      = "__fake_linux_kernel_text_end";
		}
		CURFILE++
		next
	}
	n = match($0,/([0-9A-Fa-f]+)[ 	]+<([A-Z_a-z][A-Z_0-9a-f]*)>/,a)
	if( n > 0 ) {
		addr = substr($0, a[1,"start"], a[1,"length"]);
		sym  = substr($0, a[2,"start"], a[2,"length"]);
		++nr_sym;
		addr_tab[nr_sym] = addr;
		func_tab[addr]   = sym;
	}
	end = $1
}
CURFILE==2 {
	y = strtonum($2)
	i = bin_search(y, addr_tab, nr_sym);
	if( i > 0 ) {
		addr  = addr_tab[i]
		printf("%08x: %s\n", y, func_tab[addr]);
	}
}'

