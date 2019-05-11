#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

int main( int argc, char **argv )
{
	 void* handle;

	 while( 1 ){
	 	handle = dlopen(argv[1], RTLD_NOW | RTLD_GLOBAL );
		if(!handle){
			printf("dl open error\n");
			exit(1);
		}
		dlclose( handle );
	}

	return 0;
}

