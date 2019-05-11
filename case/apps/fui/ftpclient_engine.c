#ifdef MODULE_CONFIG_FTP_CLIENT
#include "swf_ext.h"
#include <string.h>
#include <stdio.h>
#include "ftp.h"
#include <stdlib.h>
#define ftp_info(fmt,arg...)		printf("INFO[%s-%d]:"fmt"\n",__FILE__,__LINE__,##arg)
#define ftp_err(fmt,arg...)			printf("ERROR[%s-%d]:"fmt"\n",__FILE__,__LINE__,##arg)

enum{
	FTP_CMD_FILETYPE = 0,
	FTP_CMD_FILENAME = 1,
	FTP_CMD_FILESIZE = 2,
	FTP_CMD_FILECREATEDATE = 3
};

#define FILEINFO_BUF_LEN 256
static char FileInfoBuf[FILEINFO_BUF_LEN];

static  ftp_ls_info* FtpListInfo;

static int ftp_release_res()
{
	if(FtpListInfo!=NULL){
		free((void*)FtpListInfo);
		FtpListInfo=NULL;
	}
	return 0;
}

static int ftp_connect_server(void * handle)
{
	int rtn=0;
	char * ipaddr=NULL,*username=NULL,*passwd=NULL,*rootpath=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	ipaddr = Swfext_GetString();
	username = Swfext_GetString();
	passwd = Swfext_GetString();
	rootpath = Swfext_GetString();
	
	ftp_info("ip=%s,user=%s,pass=%s",ipaddr,username,passwd);
	ftp_info("rootpath=%s",rootpath);

	rtn = ftpclient_connect(ipaddr,username,passwd);
	if(rtn ==0)
		FtpListInfo= ftpclient_get_list();
	else
		FtpListInfo = NULL;
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int ftp_disconnect_server(void * handle)
{
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	rtn = ftpclient_close();
	ftp_release_res();
	SWFEXT_FUNC_END();
}

static int ftp_get_filenum(void * handle)
{
	int filenum=0;
	SWFEXT_FUNC_BEGIN(handle);
	if(FtpListInfo!=NULL)
		filenum = FtpListInfo->itemnum;
	ftp_info("filenum==%d",filenum);
	Swfext_PutNumber(filenum);
	SWFEXT_FUNC_END();
}

static int ftp_get_fileinfo(void * handle)
{
	int cmd=0,idx=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	cmd = Swfext_GetNumber();
	idx = Swfext_GetNumber();
	memset(FileInfoBuf,0,FILEINFO_BUF_LEN);
	if(FtpListInfo==NULL)
		goto GET_ERROR;
	switch(cmd){
		case FTP_CMD_FILETYPE:
			if(FtpListInfo->pinfo[idx].pathtype==1)
				FileInfoBuf[0] = 'D';
			else
				FileInfoBuf[0] = 'F';
			break;
		case FTP_CMD_FILENAME:
			strcpy(FileInfoBuf,FtpListInfo->pinfo[idx].filename);
			break;
		case FTP_CMD_FILESIZE:
			break;
		case FTP_CMD_FILECREATEDATE:
			break;
	}
	
GET_ERROR:
	ftp_info("cmd=%d,idx=%d,info=%s",cmd,idx,FileInfoBuf);
	Swfext_PutString(FileInfoBuf);
	SWFEXT_FUNC_END();
}


static int ftp_donwload_file(void * handle)
{
	int idx=0,issucceed=0;
	char *newname=NULL;
	SWFEXT_FUNC_BEGIN(handle);

	idx = Swfext_GetNumber();
	newname = Swfext_GetString();
	ftp_info("idx=%d,newname=%s",idx,newname);
	memset(FileInfoBuf,0,FILEINFO_BUF_LEN);
	strcpy(FileInfoBuf,FtpListInfo->pinfo[idx].filename);
	ftp_info("Filename=%s\n",FileInfoBuf);
	
	issucceed = ftpclient_get_file(FileInfoBuf);
	Swfext_PutNumber(issucceed);
	SWFEXT_FUNC_END();
}


static int ftp_enter_dir(void * handle)
{
	int idx=0,rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	idx = Swfext_GetNumber();
	ftp_info("idx=%d",idx);
	memset(FileInfoBuf,0,FILEINFO_BUF_LEN);
	strcpy(FileInfoBuf,FtpListInfo->pinfo[idx].filename);
	ftp_info("Dirname=%s\n",FileInfoBuf);
	
	rtn = ftpclient_enter_dir(FileInfoBuf);
	ftp_release_res();
	if(rtn ==0)
		FtpListInfo= ftpclient_get_list();
	else
		FtpListInfo = NULL;
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


static int ftp_exit_dir(void * handle)
{
	int idx=0,rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	idx = Swfext_GetNumber();
	ftp_info("idx=%d",idx);
	memset(FileInfoBuf,0,FILEINFO_BUF_LEN);
	strcpy(FileInfoBuf,FtpListInfo->pinfo[idx].filename);
	
	rtn = ftpclient_exit_dir();
	ftp_release_res();
	if(rtn ==0)
		FtpListInfo= ftpclient_get_list();
	else
		FtpListInfo = NULL;
	SWFEXT_FUNC_END();
}


int swfext_ftpclient_register(void)
{
	SWFEXT_REGISTER("ftp_connectServer", ftp_connect_server);
	SWFEXT_REGISTER("ftp_disconnectServer", ftp_disconnect_server);
	SWFEXT_REGISTER("ftp_getFileNum", ftp_get_filenum);
	SWFEXT_REGISTER("ftp_getFileInfo", ftp_get_fileinfo);
	SWFEXT_REGISTER("ftp_downloadFile", ftp_donwload_file);
	SWFEXT_REGISTER("ftp_enterDir", ftp_enter_dir);
	SWFEXT_REGISTER("ftp_exitDir", ftp_exit_dir);
	return 0;
}
#else

int swfext_ftpclient_register(void)
{
	return 0;
}

#endif	/** MODULE_CONFIG_FTP_CLIENT */
