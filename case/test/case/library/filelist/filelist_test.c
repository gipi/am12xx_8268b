#include <stdio.h>
#if 1
#include "file_list.h"
#define fltest_inf(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define fltest_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#define PATHBUF_LEN 128
char buffer_path[PATHBUF_LEN];
int fileindexdel=-1;
/******
enum{
	FT_DIR=0,// 目录名
	FT_FILE,// 文件名
	FT_MUSIC,
	FT_VIDEO,
	FT_IMAGE,
	FT_VVD,
	FT_TXT
};
***********/
void show_filelist_info(UI_FILELIST * filelist){
	char *file_longname=NULL;
	char *shortname=NULL;
	int filetotal=0;
	int i=0;
	int filetype=0;
	char *workpath=NULL;
	workpath = ui_filelist_get_cur_workpath(filelist);	
	fileindexdel = -1;
	filetotal = ui_filelist_get_filetotal_num(filelist);
	for(i=0;i<filetotal;i++){
		file_longname = ui_filelist_get_longname(filelist,i);
		fltest_inf("%d:name=%s",i,file_longname);
		file_longname = ui_filelist_get_cur_filepath(filelist,i);
		fltest_inf("%d:path=%s",i,file_longname);
		memset(buffer_path,0,PATHBUF_LEN);
		file_longname = ui_filelist_get_cur_filepath_buf(filelist,i,buffer_path);
		fltest_inf("%d:pathbuf=%s",i,buffer_path);
		filetype = ui_filelist_get_cur_filetype(filelist,i);
		if(filetype!=FT_DIR && fileindexdel==-1){
			fileindexdel = i;
		}
		fltest_inf("%d:filetype=%d\n",i,filetype);
	}
	printf("TotalNum=%d,workpath=%s,delfileindex=%d\n",filetotal,workpath,fileindexdel);
}

int main(int argc,char* argv[]){	
	UI_FILELIST filelist;
	char path[16]="/mnt/card/";
	/***show all the file in the card, exclude dirname****/
	char filetype[16]="jpg bmp jpeg tif";
	FILST_TYPE_t listtype=FILES_BROWSE;
	//char filetype[16]="   ";
	//FILST_TYPE_t listtype=DIR_BORWSE_VALID;
	scan_status_t status;
	if(argc<2){
		printf("ARGC ERRROR###########\n");
		return -1;
	}
		
	
	printf("%s,%d:Call filelist test code\n",__func__,__LINE__);
	printf("path=%s,filetype=%s\n",argv[1],filetype);
	ui_filelist_init(&filelist,argv[1],filetype,1,listtype);
	while(1){
		status = ui_filelist_check_scan_task(&filelist);
		fltest_inf("status==%d",status);
		if(filelist.fsel->scan_task->runing == EDLE || filelist.fsel->scan_task->runing ==COMPLETED)
			break;
		else
			sleep(1);
	}
	if(argc>=3)
		show_filelist_info(&filelist);
	ui_filelist_exit(&filelist);
	printf("@@@@@@@@@END@@@@@@@@@@\n");
	return 0;
}
#endif
#if 0
int main()
{
	printf("###############TEST##################");
	return 0;
}
#endif
