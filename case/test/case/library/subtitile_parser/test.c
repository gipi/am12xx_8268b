#include "subtitle_parser.h"
#include <stdio.h>
#include <string.h>


int main(int argc,char* argv[])
{
	int rtn=0;
	parser_info_t p_info;
	char *filename=NULL;
	if(argc<2)
	{
		printf("usage :%s filename\n",argv[0]);
		exit(-1);
	}
	filename = argv[1];
	if(init_instance(filename,&p_info)==0){
		subtitle_parser(&p_info);
		_printf_line_info(&p_info);
		remove_instance(&p_info);
	}
	return 0;
}