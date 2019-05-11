#include "subtitle_parser.h"
#include "string.h"
#include "stdio.h"
#include "srt_parser.h"
#include "subtitle_common.h"
#include "ssa_parser.h"
#include "sami_parser.h"
#include "vobsub_parser.h"
#include "vobsub_spudec.h"
/**********************Extern Functions********************/
extern void *subp_fopen(char *path, char *mode);
extern int subp_fclose(void *fp);
extern long subp_fread(void *fp, unsigned char *ptr, unsigned long nbytes);
extern long subp_fwrite(void *fp, unsigned char *ptr, unsigned long nbytes);
extern long subp_fseek_set(void *fp, long offset);
extern long subp_fseek_cur(void *fp, long offset);
extern long subp_fseek_end(void *fp, long offset);
extern long subp_ftell(void *fp);
extern void *subp_malloc(int size);
extern void subp_free(void * pfree);
extern void *subp_remalloc(void*buf,unsigned int size);
extern int subp_feof(void* fp);

//初始化相关的文件操作指针
int _init_parser_fops(parser_info_t *parser_info)
{
	parser_info->fops.open = subp_fopen;
	parser_info->fops.close = subp_fclose;
	parser_info->fops.read  = subp_fread;
	parser_info->fops.write = subp_fwrite;
	parser_info->fops.seek_set = subp_fseek_set;
	parser_info->fops.seek_cur = subp_fseek_cur;
	parser_info->fops.seek_end = subp_fseek_end;
	parser_info->fops.tell = subp_ftell;
	parser_info->fops.malloc = subp_malloc;
	parser_info->fops.free = subp_free;
	
	parser_info->fops.realloc = subp_remalloc;
	parser_info->fops.osfeof = subp_feof;
	parser_info->fp = NULL;
	parser_info->fp1 = NULL;
	return 0;
}



//获取文件类型
static unsigned int _subtitle_get_file_ext_type(char *filename)
{
	unsigned int rtn = 0;
	char fileext[4];
	int idxext=2;
	char c=0;
	int filenamelen=0;
	filenamelen = strlen(filename);
	printf("filelen=%d,name=%s ",filenamelen,filename);
	memset(fileext,0,4);
	while(filenamelen>=0){
		c = *(filename+filenamelen-1);
		filenamelen--;
		if(c=='.')
			break;
		else{
			if(idxext<0)
				break;
			fileext[idxext]=c;
			idxext--;
		}
	}
	printf("File ext=%s   ",fileext);
	if(strncasecmp("srt",fileext,3)==0)
		rtn =  FILE_SRT;
	else if(strncasecmp("idx",fileext,3)==0){
		rtn = FILE_SUB;
	}
	else if(strncasecmp("sub",fileext,3)==0){
		rtn = FILE_SUB;
	}
	else if(strncasecmp("ssa",fileext,3)==0){
		rtn = FILE_SSA;
	}
	else if(strncasecmp("smi",fileext,3)==0){
		rtn = FILE_SMI;
	}
	else
		rtn= FILE_INVAILD;
	printf("Filetype=0x%x\n",rtn);
	return rtn;
}


unsigned char _subtitle_get_file_type(parser_info_t *parser_info)
{
	unsigned char fileformat[4]="";
	if(parser_info->fp!=NULL){
		int nbytesread=0;
		parser_info->fops.seek_set(parser_info->fp,0); //move the file point to the head
		nbytesread = parser_info->fops.read(parser_info->fp,(unsigned char*)fileformat,(unsigned long)4);
		if(nbytesread!=4){
			printf("%s,%d:The file is too short!\n",__FILE__,__LINE__);
			return 0;
		}
		if(fileformat[0]==0xff && fileformat[1]==0xfe)
			parser_info->ftype |=SUBTITLE_UNI16_LIT;
		else if(fileformat[0]==0xfe && fileformat[1]==0xff)
			parser_info->ftype |=SUBTITLE_UNI16_BIG;
		else if(fileformat[0]==0xef && fileformat[1]==0xbb && fileformat[2]==0xbf)
			parser_info->ftype |=SUBTITLE_UTF8;
		else
			parser_info->ftype |=SUBTITLE_MBCS; //default type is utf8
		
		if(parser_info->ftype>>8==0)
			parser_info->ftype |=SUBTITLE_UTF8; //没有特殊标记的时候默认为UTF8
		
		printf("%s,%d:File Type=0x%x[U16 LIT=0x%x,U16 BIG=0x%x,UTF8=0x%x,MBCX=0x%x,\n",__FILE__,__LINE__,parser_info->ftype,\
			SUBTITLE_UNI16_LIT,SUBTITLE_UNI16_BIG,SUBTITLE_UTF8,SUBTITLE_MBCS);
		parser_info->fops.seek_set(parser_info->fp,0); //move the file point to the head
		return 1;
		
	}
	else
		printf("%s,%d:Crazy File Handle is NULL!\n",__FILE__,__LINE__);
	return 0;
}

int init_instance(char*file,parser_info_t *parser_info)
{
	unsigned int fileextention=0;
	_init_parser_fops(parser_info);
	parser_info->ftype = _subtitle_get_file_ext_type(file);
	
	fileextention = parser_info->ftype&0x000f;
	printf("%s,%d:File EXT====%d,filetype=0x%x\n",__FILE__,__LINE__,fileextention,parser_info->ftype);
	if(fileextention==FILE_SRT){//初始化与SRT文件相关的资源
		parser_info->fp = parser_info->fops.open(file,"rb");
		if(parser_info->fp==NULL){
			printf("%s,%d:Open File error!\n",__FILE__,__LINE__);
			return -1;
		}
		if(_subtitle_get_file_type(parser_info)==0)
			return -1;
		parser_info->get_file_info = srt_get_info;
		parser_info->depose_res = srt_depose_res;
		parser_info->finfo =(void*)parser_info->get_file_info(parser_info);
	}
	else if(fileextention==FILE_SUB){
		if(vobsub_init_fp(file,parser_info)==-1)
			return -1;
		if(_subtitle_get_file_type(parser_info)==0)
			return -1;
		parser_info->get_file_info= vobsub_get_info;
		parser_info->depose_res = vobsub_depose_res;
		parser_info->finfo =(void*)parser_info->get_file_info(parser_info);
	}
	else if(fileextention==FILE_SSA){
		parser_info->fp = parser_info->fops.open(file,"rb");
		if(parser_info->fp==NULL){
			printf("%s,%d:Open File error!\n",__FILE__,__LINE__);
			return -1;
		}
		if(_subtitle_get_file_type(parser_info)==0)
			return -1;
		parser_info->depose_res = ssa_depose_res;
		parser_info->get_file_info = ssa_get_info;
		parser_info->finfo =(void*)parser_info->get_file_info(parser_info);
	}
	else if(fileextention==FILE_SMI){
		parser_info->fp = parser_info->fops.open(file,"rb");
		if(parser_info->fp==NULL){
			printf("%s,%d:Open File error!\n",__FILE__,__LINE__);
			return -1;
		}
		if(_subtitle_get_file_type(parser_info)==0)
			return -1;
		parser_info->get_file_info = sami_get_info;
		parser_info->depose_res = sami_depose_res;
		parser_info->finfo = (void*)parser_info->get_file_info(parser_info);
	}
	else{
		printf("%s,%d:Do not support now\n",__FILE__,__LINE__);
		return -1;
	}
	return 0;
}

int remove_instance(parser_info_t *parser_info)
{
	//关闭文件句柄
	if(parser_info->fp!=NULL){
		parser_info->fops.close(parser_info->fp);
		parser_info->fp =NULL;
	}
	if(parser_info->fp1!=NULL){
		parser_info->fops.close(parser_info->fp1);
		parser_info->fp1 =NULL;
	}

	//释放相关资源
	if(parser_info->depose_res!=NULL)
		parser_info->depose_res(parser_info);
	
	parser_info->depose_res = NULL; // indicate "remove_instance" is done
		
	return 0;
}


int subtitle_parser(parser_info_t *parser_info)
{
	void *vobsub=NULL;
	int ret=0;
	int ret_value=0;
	unsigned int fileextention = parser_info->ftype&0x000f;
	
	if(parser_info->finfo==NULL){
		printf("%s,%d:Crazy:Parser info finfo==NULL",__FILE__,__LINE__);
		return -1;
	}
	
	if(fileextention==FILE_SRT){
		ret = srt_parser(parser_info);
	}
	else if(fileextention==FILE_SUB){
		vobsub = vobsub_open(1,parser_info);
		ret = vobsub?0:-1;
	}
	else if(fileextention == FILE_SSA){
		ret_value = ssa_parser(parser_info);
		//
		// After ssa_parser,
		// we record this "ssa" in the form of UTF-8 (from UTF-16 if it was)
		//
		ret = ret_value?0:-1;
	}
	else if(fileextention== FILE_SMI)
	{
		ret_value = sami_parser(parser_info);
		ret = ret_value?0:-1;
	}
	else
		printf("%s,%d:Not support yet!\n",__FILE__,__LINE__);
	return ret;
}


//获取下一行字幕的信息，字幕的起始终止时间和长度放在返回的结构体中，内容放在给定的strbuf中
//需要外面传入放置内容的buf和可使用的buf长度
line_info_t get_next_line(parser_info_t *parser_info,char* strbuf,int len)
{
	line_info_t line_info;
	unsigned int fileextention=0;
	fileextention = parser_info->ftype&0x000f;
	if(fileextention==FILE_SRT){
		srt_get_next_line(parser_info,&line_info,strbuf,len);
	}
	else if(fileextention==FILE_SSA){
		ssa_get_next_line(parser_info,&line_info,strbuf,len);
	}
	else if(fileextention==FILE_SMI){
		sami_get_next_line(parser_info,&line_info,strbuf,len);
	}
	else
		printf("%s,%d:Sorry No Support.\n",__FILE__,__LINE__);
	return line_info;
}


line_info_t query_line_info(parser_info_t *parser_info,struct time_line *time_query,char* strbuf,int len)
{
	line_info_t line_info;
	unsigned int fileextention=0;
	fileextention = parser_info->ftype&0x000f;
	memset(&line_info,0,sizeof(line_info_t));
	if(fileextention== FILE_SRT){
		srt_query_line_info(parser_info,time_query,strbuf,len,&line_info);
		//if(srt_query_line_info(parser_info,time_query,strbuf,len,&line_info)!=0)
			//printf("%s,%d:Sorry, can't match!\n",__FILE__,__LINE__);
	}
	else if(fileextention== FILE_SMI){
		sami_query_line_info(parser_info,time_query,strbuf,len,&line_info);
	}
	else if(fileextention== FILE_SSA){
		ssa_query_line_info(parser_info,time_query,strbuf,len,&line_info);
	}
	else  if(fileextention==FILE_SUB){
		vobsub_query_line_info(parser_info,time_query,&line_info);
	}
	else 
		printf("%s,%d:Sorry No Support.\n",__FILE__,__LINE__);
	return line_info;
}

//
int _printf_line_info(parser_info_t *parser_info){
	int i=0;
	unsigned int fileextention=0;
	fileextention = parser_info->ftype&0x000f;
	if(fileextention == FILE_SRT){
		_test_srt_print_subtitle(parser_info);
		//_test_srt_query_line_info(parser_info);
	}
	else if(fileextention==FILE_SSA){
		_test_ssa_print_subtitle(parser_info);
		//_test_ssa_query_line_info(parser_info);
	}
	else if(fileextention== FILE_SMI){
		_test_sami_print_subtitle(parser_info);
		//_test_sami_query_line_info(parser_info);
	}
	else if(fileextention== FILE_SUB){
		_test_vobsub_query_line_info(parser_info);
	}
	else
		printf("%s,%d:No Support yet!\n",__FILE__,__LINE__);
	return 0;
}

/*
int main(void)
{
	int rtn=0;
	parser_info_t p_info;
	char filename[30]="test.smi";
	if(init_instance(filename,&p_info)==0){
		subtitle_parser(&p_info);
		_printf_line_info(&p_info);
		remove_instance(&p_info);
	}
	return 0;
}
*/
