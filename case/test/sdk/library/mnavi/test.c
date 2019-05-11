#include "mnavi_fop.h"

#include <stdio.h>
#include <stdlib.h>

/***
** usage :  argv[0]  cmd  src  des
***/
int main(int argc,char* argv[])
{
	int ret;

	if(argc<3)
	{
		printf("usage :%s cmd src  des\n",argv[0]);
		exit(-1);
	}

	int cmd = atoi(argv[1]);
	double process;
	struct mnavi_work_info work;
	struct mnavi_work_handle_t * handle; 

	switch(cmd)
	{
		case 1: //  copy file
		case 2: //copy dir
		case 5://overlap file
		case 6: //overlap dir
			if(argc!=4)
			{
				printf("error arg\n");
				return -1;
			}
			work.src = (char*)argv[2];
			work.des = (char*)argv[3];
			break;
		case 3://del file 
		case 4://del dir
			if(argc!=3)
			{
				printf("error arg\n");
				return -1;
			}
			
			work.src = (char*)argv[2];


			break;
		default : //
			break;
	}
	work.cmd = cmd+MNAVI_CMD_BASE;
	handle = mnavi_create_work(&work);

	mnavi_start_work(handle);
	
	while(1)
	{
		process = mnavi_get_process(handle);
		if(process >=0&&process < 1)
			printf("now process :%f\n",process);
		else{
			printf("finish process:%f\n",process);
			if(process < 0)
				printf("error status:%d\n",mnavi_get_status(handle));
			break;
		}
	}
	mnavi_release_work(handle);
	
	return 0;
}

