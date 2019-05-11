#include <stdio.h>
#include <string.h>
int main(  int argc, char * argv[] )
{
	FILE*	fp = NULL;
	char	str[ 100 ], ch[50];
	int		intop_ch = 6;
	
	memset( str, 0x00, 100 );
	if ( argc == 1 )
	{
		printf("--------- go_ch parameter error\n");
		return( 1 );
	}
	else
	{
		intop_ch = atoi(argv[ 1 ]);
	}
	printf("--------- go_ch parameter intop_ch=%d\n",intop_ch);
	fp = fopen( "/tmp/wfd/rtl_hostapd.conf", "a+" );
	if ( fp == NULL )
	{
		printf( "Error to open the rtl_hostapd.conf file\n" );
	}
	else
	{
		memset( ch, 0x00, 50 );
		fputs( "\n", fp );
		sprintf( ch, "channel=%d\n", intop_ch );
		fputs( ch, fp );
		memset( ch, 0x00, 50 );
		if( intop_ch > 14 )
			sprintf( ch, "hw_mode=a\n");
		else
			sprintf( ch, "hw_mode=g\n");
		fputs( ch, fp );
		fclose( fp );
		printf( "------- go_ch ok\n" );
	}
	return( 1 );
}
