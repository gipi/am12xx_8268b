#!/bin/sh
#LIBS='sdk/lib/libAA1.so case/lib/libBB1.so -L case/lib -lBB2'
if [ -z "$LIBS" ]; then exit 0; fi
awk '{
	for(i=1; i<=NF; i++) {
		n = match($i,/((.+\/)*)lib(.+)\.so/,a);
		if(n > 0) {
			dir = substr($i, a[2,"start"], a[2,"length"])
			lib = substr($i, a[3,"start"], a[3,"length"])
			printf("-L%s -l%s ", dir,lib);
		} else
			printf("%s ", $i);
	}

}' <<< "$LIBS"
