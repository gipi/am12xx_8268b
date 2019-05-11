#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys_vram.h>

#define TASK_ID			1
#define TASK_SYS_INFO	1
#define SYS_INFO_NAME	1

int main(void)
{
	char test_buf[128];
	struct vram_param_t vram_par;
	int value=0;
	
	sprintf(test_buf,"%s","This test code is mainly for vram lib");
	printf("%s'\n",test_buf);
	
	vram_par.task_id = TASK_ID;
	vram_par.sub_type = TASK_SYS_INFO;
	vram_par.target_name = SYS_INFO_NAME;
	vram_par.pbuf = test_buf;
	vram_par.offset = 0;
	vram_par.length = sizeof(test_buf);

	value = app_write_vram(&vram_par);
	if(value<=0){
		printf("vram write fail %d\n",value);
		goto END;
	}
	memset(vram_par.pbuf,0,128);
	value = app_read_vram(&vram_par);
	if(value<=0)
		printf("vram read fail %d\n",value);
	else
		printf("vram op right: %s\n",vram_par.pbuf);

	value = app_delete_vram(&vram_par);
	if(value){
		printf("vram delete fail %d\n",value);
	}

END:
	exit(0);
}
