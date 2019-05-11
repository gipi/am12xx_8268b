#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "sys_cfg.h"
#include <dlfcn.h>
#include "iconv.h"
#include "sys_resource.h"
#include "ipc_key.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fui_common.h>



#define _MLANG_DIRECT_CONVER

#define	GET_FONT_WIDTH(charinfo)		(charinfo >> 26)
#define	GET_FONT_OFFADDR(charinfo)	(charinfo & 0x3ffffff)
typedef struct {
	unsigned short First;         /* first character               */
	unsigned short Last;          /* last character                */
	int OffAddr;      /* 指向的是当前SECTION包含的 UFL_CHAR_INFO第一个字符信息的起始地址 */
} UFL_SECTION_INF;

typedef struct {
	char		magic[4]; //'U', 'F', 'L', X---Unicode Font Library,
	//X: 表示版本号。分高低4位。如 0x10表示 Ver 1.0
	int 	Size;		 /* File total size */
	unsigned char nSection; // 共分几段数据。
	unsigned char YSize;    /* height of font  */  
	//unsigned char YDist;    /* space of font y */
	unsigned char XMag;     /* magnification x */
	unsigned char YMag;	   /* magnification y */
	char		reserved[4];
	UFL_SECTION_INF  *pData;
} UFL_head_t;


#ifndef _MLANG_DIRECT_CONVER

/**
* if we call iconv_convert() directly, do not use these definitions.
*/
typedef int *(*mlang_covert)(PICONV_PARAMETER piconvPara);
static mlang_covert international_convert = NULL;
static void *mdlhandle=NULL;

#endif

UFL_head_t	UFL_head;
int charInfo;
FILE* fontHandle;

static char LanArray[][16]={
	"gb2312",
	"UTF-8",
	"UTF-16LE",
	"UTF-16BE",
	"ascii",
	"cp874",
	"cp932",
	"cp949",
	"cp950",
	"cp1250",
	"cp1251",
	"cp1252",
	"cp1253",
	"cp1254",
	"cp1255",
	"cp1256",
	"cp1257",
	"cp1258",
	
};

void *sys_shmget(int size,int key)
{
	int shmid = -1; /* ipc shared memory id */
	void *shbuf;

	if(size)
		shmid = shmget(key, size, IPC_CREAT |IPC_EXCL| 0666);
	if (!size || shmid == -1)
	{
		printf("shmget error,size:%d shmid:%d",size,shmid);
		return NULL;
	}

	shbuf = (void *)shmat(shmid, NULL, 0);
	if (shbuf == (void*) -1L) 
	{ /* shmat has bizarre error return */
		printf("shmget shbuf error,size:%d shmid:%d",size,shmid);
		return NULL;
	}
	memset(shbuf, 0,size);
	return shbuf;
}

int sys_shmrm(int key)
{
	void *shbuf;
	int shmid = -1; /* ipc shared memory id */

	shmid = shmget(key, 0, 0);
	if (shmid == -1)
	{
		printf("shmget error,shmid:%d",shmid);
		return -1;
	}

	shbuf = (void *)shmat(shmid, NULL, 0);
	if (shmdt(shbuf) == -1) 
	{
		printf("shmdt failed");
		return -1;
	}
	if (shmctl(shmid, IPC_RMID, 0) == -1)
	{	
		printf("IPC_RMID error,shmid:%d",shmid);
		return -2;
	}
	
	return 0;
}
static char __set_charset(int charset_ID,char **char_set)
{
	char rtn=0;
	
	switch(charset_ID){//目前的转换规则是一一对应的，后续可能不是这样的情况
		case LAN_GB2312:
		case LAN_UTF8:
		case LAN_UTF16LE:
		case LAN_UTF16BE:
		case LAN_ASCII:
		case LAN_CP874:
		case LAN_CP932:
		case LAN_CP949:
		case LAN_CP950:
		case LAN_CP1250 :
		case LAN_CP1251 :
		case LAN_CP1252 :
		case LAN_CP1253 :
		case LAN_CP1254 :
		case LAN_CP1255 :
		case LAN_CP1256 :
		case LAN_CP1257 :
		case LAN_CP1258 :
			*char_set = LanArray[charset_ID];
			break;
		default:
			printf("%s,%d:Set  CharSet Error, charset_ID=%d\n",__FILE__,__LINE__,charset_ID);
			rtn = -1;
			break;
	}
	//printf("ID=%d,LanArray=%s\n",charset_ID,LanArray[charset_ID]);
	return rtn;
}

int change_chardec(charsetchg_info_t *change_info)
{
	ICONV_PARAMETER iconvPara;
	
	memset(&iconvPara, 0, sizeof(ICONV_PARAMETER));
	
	if(__set_charset(change_info->src_charset,&iconvPara.from_charset)!=0){
		return -1;
	}
	
	if(__set_charset(change_info->dest_charset,&iconvPara.to_charset)!=0){
		return -1;
	}
	
	//printf("Change CharDEC:form=%s,to=%s\n",iconvPara.from_charset,iconvPara.to_charset);
	iconvPara.inbuf = change_info->inputbuf;
	iconvPara.inbytesleft = change_info->inputbuflen;
	iconvPara.outbuf = change_info->outputbuf;
	iconvPara.outbytesleft = change_info->outputbuflen;	

#ifdef _MLANG_DIRECT_CONVER
	return	iconv_convert(&iconvPara);   
#else
	return	(int)international_convert(&iconvPara);   	
#endif
 		
}

static int _mlang_dll_open(void)
{
#ifdef _MLANG_DIRECT_CONVER
	return 0;
#else
	char mlanglib[64];
	
	sprintf(mlanglib,"%s%s",AM_DYNAMIC_LIB_DIR,"libiconv.so");
	if(mdlhandle){
		dlclose(mdlhandle);
	}
	mdlhandle = dlopen(mlanglib,RTLD_LAZY|RTLD_GLOBAL);
	
	if(mdlhandle==NULL){
		printf("dl open error\n");
		goto M_OPEN_ERROR1;
	}

	/**
	* resolve the mlang APIs.
	*/
	dlerror();
	international_convert = dlsym(mdlhandle,"libiconv_convert");
	if(international_convert == NULL){
		printf("iconv_convert link error:%s\n",dlerror());
		goto M_OPEN_ERROR2;
	}

	return 0;
	
M_OPEN_ERROR2:
	
	if(mdlhandle){
		dlclose(mdlhandle);
		mdlhandle = NULL;
	}
	
M_OPEN_ERROR1:
	
	return -1;
#endif	

}
static int _mlang_dll_close(void)
{
#ifdef _MLANG_DIRECT_CONVER
	/**
	* do nothing.
	*/
#else
	international_convert = NULL;
	
	if(mdlhandle){
		dlclose(mdlhandle);
		mdlhandle = NULL;
	}
#endif

	return 0;
}

int	gb2312_to_utf8(char   *inbuf,size_t   *inlen, char   *outbuf, size_t   *outlen)   
{   
	ICONV_PARAMETER iconvPara;
	
	memset(&iconvPara, 0, sizeof(ICONV_PARAMETER));
	iconvPara.from_charset = "gb2312";
	iconvPara.to_charset = "UTF-8";
	iconvPara.inbuf = inbuf;
	iconvPara.inbytesleft = inlen;
	iconvPara.outbuf = outbuf;
	iconvPara.outbytesleft = outlen;
	
#ifdef _MLANG_DIRECT_CONVER
	return	iconv_convert(&iconvPara);   
#else
	return	(int)international_convert(&iconvPara);   	
#endif
} 
     
  //GB2312码转为U16码
int	gb2312_to_utf16le(char   *inbuf, size_t *inlen, char   *outbuf, size_t   *outlen)   
{  	
	ICONV_PARAMETER iconvPara;
	
	memset(&iconvPara, 0, sizeof(ICONV_PARAMETER));
	iconvPara.from_charset = "gb2312";
	iconvPara.to_charset = "UTF-16LE";
	iconvPara.inbuf = inbuf;
	iconvPara.inbytesleft = inlen;
	iconvPara.outbuf = outbuf;
	iconvPara.outbytesleft = outlen;		

#ifdef _MLANG_DIRECT_CONVER
	return	iconv_convert(&iconvPara);   
#else
	return	(int)international_convert(&iconvPara);   	
#endif
	   	  	
} 

int	utf16le_to_utf8(char   *inbuf, size_t *inlen, char   *outbuf, size_t   *outlen)   
{  	
	ICONV_PARAMETER iconvPara;
	
	memset(&iconvPara, 0, sizeof(ICONV_PARAMETER));
	iconvPara.from_charset = "UTF-16LE";
	iconvPara.to_charset = "UTF-8";
	iconvPara.inbuf = inbuf;
	iconvPara.inbytesleft = inlen;
	iconvPara.outbuf = outbuf;
	iconvPara.outbytesleft = outlen;		

#ifdef _MLANG_DIRECT_CONVER
	return	iconv_convert(&iconvPara);   
#else
	return	(int)international_convert(&iconvPara);   	
#endif
	   	  	
}

int	utf16be_to_utf8(char   *inbuf, size_t *inlen, char   *outbuf, size_t   *outlen)   
{  	
	ICONV_PARAMETER iconvPara;

	memset(&iconvPara, 0, sizeof(ICONV_PARAMETER));
	iconvPara.from_charset = "UTF-16BE";
	iconvPara.to_charset = "UTF-8";
	iconvPara.inbuf = inbuf;
	iconvPara.inbytesleft = inlen;
	iconvPara.outbuf = outbuf;
	iconvPara.outbytesleft = outlen;		

#ifdef _MLANG_DIRECT_CONVER
	return	iconv_convert(&iconvPara);   
#else
	return	(int)international_convert(&iconvPara);   	
#endif
 	  	
} 
int get_char_width(unsigned short aChar)
{
	int charDist;
	int  offset ;
	int   first,last,i;	
	charInfo = 0;

	if(fontHandle != 0)
	{
		for(i = 0;i<UFL_head.nSection;i++)
		{
			if(aChar >= UFL_head.pData[i].First && aChar <= UFL_head.pData[i].Last)
			{	
				break;
			}
		}
		if(i >= UFL_head.nSection)
		{
			charInfo = 0;
			return 0;
		}
		
		offset = UFL_head.pData[i].OffAddr+4*(aChar - UFL_head.pData[i].First );
		fseek(fontHandle,offset,SEEK_SET);
		fread(&charInfo,sizeof(char),sizeof(int), fontHandle);	
	}
	charDist = GET_FONT_WIDTH(charInfo);
	return charDist;

}

int system_res_init(void)
{
	int ret =0;
	char fontpath[128];

	get_touch_calibrate_params();
	
	sprintf(fontpath,"%s%s",AM_CASE_DAT_DIR,"/U16.bin");
	fontHandle = fopen(fontpath,"rb");
	if(!fontHandle)
		return -1;
	fread(&UFL_head,sizeof(char),sizeof(UFL_head_t)-4,fontHandle);
	if(UFL_head.magic[0] != 'U' || UFL_head.magic[1] != 'F' || UFL_head.magic[2] != 'L')
	{
		printf("Invaild font file()!\n",fontpath);
		return -1;
	}
	if(UFL_head.pData==NULL){
	    UFL_head.pData = (UFL_SECTION_INF  *)malloc(UFL_head.nSection*sizeof(UFL_SECTION_INF));
	}
		
	fseek(fontHandle,sizeof(UFL_head_t)-4,SEEK_SET);
	fread(UFL_head.pData,sizeof(char),UFL_head.nSection*sizeof(UFL_SECTION_INF),fontHandle);

#ifdef _MLANG_DIRECT_CONVER
	if(_mlang_dll_open()<0)
		ret = -1;
#endif

	return ret;
}
