#include "subtitle_parser.h"
#include "stdio.h"
#include "string.h"
#include "swf_ext.h"
#include "sys_resource.h"
#include "stddef.h"
#include "system_info.h"



#define sp_info(fmt,arg...)	//printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define sp_err(fmt,arg...) //printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#define STRTEMPBUF_LEN 4096
#define PARSER_RES_LEN 2
#define HANDLE_BASE 100

#define LANGE_ID_CHINESE 1

static void print_test(char* strprint, int len,int eachline,char *title);
#define OSD_ALIGN_H_LEFT 		0x01
#define OSD_ALIGN_H_RIGHT 		0x02
#define OSD_ALIGN_H_CENTER 		0x04
#define OSD_ALIGN_V_TOP			0x10
#define OSD_ALIGN_V_BOTTOM		0x20
#define OSD_ALIGN_V_CENTER		0x40

static void *subtitle_target=NULL;
static int fileidx=0;
enum sp_cmd_e{
	CMD_GET_TIME_START,
	CMD_GET_TIME_END,
};

typedef struct{
	int  subtitle_lines;
	int  subtitle_length[20];
	unsigned short  subtitle_width[20];
	unsigned short  *subtitle_ptr[20];
} subtitle_infomations;

subtitle_infomations  subtitle_info;
char *g_linebuf=NULL;
	
	
char StrTempBuf[STRTEMPBUF_LEN];
char StrTmpTransf[STRTEMPBUF_LEN];
line_info_t TmpLineInfo;
#define IS_GBK_INIT (-2)
static int sub_fileformat=0;	//记录文件的编码格式
static int sub_fileext=0;		//记录文件的后缀名
static int charset_change_cnt=0;	///需要多次尝试转换之后才能确定最后的字符转换方式
static int sub_filefix=0;			///修改判断策略，加载字幕之后，前面4次的转换结果将决定后续是否需要再进行转换

typedef struct parser_res_s{
	char is_free;
	int handle;
	parser_info_t parser_info;
}parser_res_t;



parser_res_t parser_res[PARSER_RES_LEN];


static void _init_parser_res()
{	
	int i=0;
	for(i=0;i<PARSER_RES_LEN;i++){
		parser_res[i].is_free = 1;
		parser_res[i].handle = HANDLE_BASE+i;
		memset(&parser_res[i].parser_info,0,sizeof(parser_info_t));
	}
	memset(StrTempBuf,0,STRTEMPBUF_LEN);
	memset(&TmpLineInfo,0,sizeof(line_info_t));
}

static int _get_free_parser_res()
{
	int i=0;
	for(i=0;i<PARSER_RES_LEN;i++){
		if(parser_res[i].is_free==1){
			return i;
		}
	}
	sp_err("Parser Res is not enough, free first");
	return -1;
}

static int _free_parser_res(int idx)
{
	parser_res[idx].is_free = 1;
	parser_res[idx].handle = HANDLE_BASE+idx;
	memset(&parser_res[idx].parser_info,0,sizeof(parser_info_t));
	return 0;
}

static int _handle2idx(int handle)
{
	int i;
	for(i=0;i<PARSER_RES_LEN;i++){
		if(parser_res[i].handle==handle &&parser_res[i].is_free==0)
			return i;
	}
	sp_err("The handle=%d is not exit",handle);
	return -1;
}

static void _timesec2timeline(int timesec,struct time_line *timeline)
{
	int time_pot = timesec;
	timeline->hour = time_pot/3600;
	time_pot = time_pot-timeline->hour*3600;
	timeline->min = time_pot/60;
	timeline->sec = time_pot%60;
	timeline->millsec = 0;
}

static int _subtitle_bitmap_get_status(int id)
{
	return 1;
}


static int get_cur_lange()
{
	return LANGE_ID_CHINESE;
}

/**
return 0 means successful
***/

static void set_utf16(char * str, int * index,unsigned short int u16c)
{
	int i = *index;
	if(u16c >= 0x800)
	{
		// 3 byte
		*index = i + 3;
		str[i] = (u16c >> 12) | 0xe0;
		str[i+1] = ((u16c >> 6) & 0x3f) | 0x80;
		str[i+2] =( u16c & 0x3f) | 0x80;
	}
	else if(u16c >= 0x80)
	{
		// 2 byte
		*index = i + 2;
		str[i] = ((u16c >> 6) & 0x1f) | 0xc0;
		str[i+1] = (u16c & 0x3f) | 0x80;
	}
	else
	{
		// 1 byte
		*index = i + 1;
		str[i] = (char)u16c;
	}
}
static int utf16_to_utf8(short int * u16str, char * u8str)
{
	int i, j;
	i = 0, j = 0;
	do 
	{
		set_utf16(u8str, &i, u16str[j]);
	} while (u16str[j++] != 0);
	return i;
}

static int utf82utf16(char*srcbuf,int *src_size,char *destbuf,int *dest_size)
{	
	int is_change_ok=0;
	charsetchg_info_t change_info;
	memset(&change_info,0,sizeof(charsetchg_info_t));
	change_info.src_charset = LAN_UTF8;
	change_info.dest_charset = LAN_UTF16LE;
	change_info.inputbuf = srcbuf;
	change_info.inputbuflen = (size_t*)src_size;
	change_info.outputbuf = destbuf;
	change_info.outputbuflen = (size_t*)dest_size;
	//print_test(StrTempBuf,TmpLineInfo.len,16,"Str Get from File");
	is_change_ok = change_chardec(&change_info);

	return is_change_ok;
}
static void insert(unsigned short*ptr_insert,unsigned short*ptr_end,unsigned short context)
{
	unsigned short *ptr_temp;
	unsigned short context_temp;
	for(ptr_temp=ptr_end;ptr_temp>=ptr_insert;)
		{
			context_temp=*ptr_temp;
			ptr_temp++;
			*ptr_temp=context_temp;
			ptr_temp-=2;
			
		}
	*ptr_insert=context;
}

#define DESTBUF_LEN 4096
static subtitle_infomations* subtitle_line_feed(char *srcbuf,int *src_size,int font_size,int line_width)
{
	int i=0;
	int j=0;
	int k=0;
	int ok=0;
	int sum_subtitle=0;
	unsigned short *destbuf_max=NULL;
	unsigned short *destbuf_min=NULL;

	char *tmpbuf=NULL;
	unsigned short  *destbuf=NULL;
	int size=DESTBUF_LEN;

	memset(&subtitle_info,0,sizeof(subtitle_infomations));		//clear subtitle_info
	
	tmpbuf=g_linebuf;
	unsigned short  *g_linebufMAX=(unsigned short* )tmpbuf+DESTBUF_LEN;
	if(tmpbuf==NULL)
		return NULL;
	
	destbuf =(unsigned short  *)tmpbuf;
	(char *)tmpbuf;
	
	ok=utf82utf16(srcbuf,src_size,tmpbuf,&size);
	
	if(ok==-1)  
		{
			printf("Failing to transefer UTF8 to UTF16,please transefer again!");
			return NULL;
		}
		
	else
		{
			destbuf_min=destbuf;
			
			while(*destbuf!=0x0000)
				{
					destbuf++;
				}
			destbuf_max=destbuf;
			destbuf_max+=100;
			if(destbuf_max>g_linebufMAX)
				{
					printf("No enough memory,please apply again!");
					return  NULL;
				}
			destbuf_max-=100;	
			destbuf=destbuf_min;
			
			subtitle_info.subtitle_lines=1;
			subtitle_info.subtitle_length[i]=0;
			subtitle_info.subtitle_ptr[j]=destbuf;
			subtitle_info.subtitle_width[k]=0;
			while((*destbuf)!=0x0000)
				{
					subtitle_info.subtitle_length[i]++;
					sum_subtitle+=SWF_GetCharWidth(*destbuf, font_size);
					subtitle_info.subtitle_width[k]=sum_subtitle;
					//sp_info("renturn Char Width============%d",SWF_GetCharWidth(*destbuf, font_size));
					if(sum_subtitle>=line_width)
						{	
							subtitle_info.subtitle_lines++;

							sum_subtitle-=SWF_GetCharWidth(*destbuf, font_size);
							subtitle_info.subtitle_width[k]=sum_subtitle;
							k++;
							sum_subtitle=0;
							
							subtitle_info.subtitle_length[i]--;
							i++;
							subtitle_info.subtitle_length[i]=0;
							
														
							insert(destbuf,destbuf_max,0x000D);
							destbuf_max++;
							destbuf++;
							insert(destbuf,destbuf_max,0x000A);
							destbuf++;
							j++;
							subtitle_info.subtitle_ptr[j]=destbuf;

							
														
							--destbuf;
							
					
						}
									
					if((*destbuf==0x000D)&&((*++destbuf)==0x000A)&&(*++destbuf)!=0x0000)
							{	
								subtitle_info.subtitle_lines++;

								//sum_subtitle-=SWF_GetCharWidth(*destbuf, font_size);
								//subtitle_info.subtitle_width[k]=sum_subtitle;
								k++;				//the next line
								sum_subtitle=0;
								
								
								subtitle_info.subtitle_length[i]--;
								i++;
								subtitle_info.subtitle_length[i]=0;
								*(destbuf-2)=0x0000;				//if not,the ....subtitle_ptr[j] while contain the j line and j+1 line or more
								j++;
								subtitle_info.subtitle_ptr[j]=destbuf;
							
								//sp_info("one enter subtitle_info._ptr[%d]===========%s\n",j,subtitle_info.subtitle_ptr[j]);				
												
								--destbuf;
						}	

					destbuf++;
                                
					
				}
		}
	 	
	return  &subtitle_info;

}
					

					
					
					


#ifdef MODULE_CONFIG_VIDEO_SUBTITLE
static int parser_init_instance(void *handle)
{
	int rtn=0;
	int ext_ok=0;
	char *filename;
	char *filename_ext=NULL;
	int idx;
	SWFEXT_FUNC_BEGIN(handle);
	
	filename = Swfext_GetString();
	idx = _get_free_parser_res();
	printf("idx====%d\n",idx);
	sp_info("@@@@@@Call Parser SubTitle@@filename=%s,idx=%d@@@",filename,idx);
	//ext_ok = subtitle_extend_filename(filename,&filename_ext);
	//if(ext_ok==0)
	//	filename = filename_ext;
	get_system_lang();		//get the system lang and catch the codepage.
	if(init_instance(filename,&parser_res[idx].parser_info)==0){
		parser_res[idx].is_free = 0;
		rtn = parser_res[idx].handle;
		g_linebuf = (char *)malloc(DESTBUF_LEN);
	}
	else
		rtn = 0;

	//printf("goto point 000000000\n");
	if(ext_ok==0)
		subtitle_free_filename(&filename_ext);
	//printf("goto point 111111111\n");
	Swfext_PutNumber(rtn);	
	SWFEXT_FUNC_END();
}

static int parser_remove_instance(void *handle)
{
	int reshandle=0;
	int idx=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	reshandle = Swfext_GetNumber();
	idx = _handle2idx(reshandle);
	if(idx!=-1){
		remove_instance(&parser_res[idx].parser_info);
		_free_parser_res(idx);
		if(g_linebuf){
			free(g_linebuf);
			g_linebuf=NULL;
		}
	}
	
	Swfext_PutNumber(0);	
	SWFEXT_FUNC_END();
}

static int parser_parse_subtitle(void *handle)
{
	int reshandle=0;
	int ret=0;
	int idx=0;
	SWFEXT_FUNC_BEGIN(handle);
	reshandle = Swfext_GetNumber();
	idx = _handle2idx(reshandle);
	if(idx!=-1){
		ret = subtitle_parser(&parser_res[idx].parser_info);
		sub_fileformat = parser_res[idx].parser_info.ftype&0xf000;
		sub_fileext= parser_res[idx].parser_info.ftype&0xf;
		charset_change_cnt = 0;
		sub_filefix = 0;
		printf("%s,%d:ret=%d,filemat===%x\n",__FILE__,__LINE__,ret,sub_fileformat);
		/*switch(filemat){
			case SUBTITLE_UNI16_BIG:
				break;
			case SUBTITLE_UNI16_LIT:
				break;
			case SUBTITLE_UTF8: //UTF-8
				is_gbk = -1;
				break;
			case SUBTITLE_MBCS://
				break;
			default:
				printf("%s,%d:No Handle yet!\n",__FILE__,__LINE__);
				break;
		}*/
	}
	else
		ret = -1;
	Swfext_PutNumber(ret);	
	SWFEXT_FUNC_END();
}

static int parser_get_next_line_info(void *handle)
{
	int rtn=0;
	int reshandle=0;
	int idx=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	reshandle = Swfext_GetNumber();
	idx = _handle2idx(reshandle);
	if(idx!=-1){
		TmpLineInfo = get_next_line(&parser_res[idx].parser_info,StrTempBuf,STRTEMPBUF_LEN);
		if(TmpLineInfo.len==0)//get line failed
			rtn = -1;
		else
			rtn = 0;
	}

	Swfext_PutNumber(rtn);	
	SWFEXT_FUNC_END();
}

static int parser_query_line_info(void *handle)
{
	int rtn=-1;
	int reshandle=0;
	int idx=0;
	int time_pot;
	struct time_line time_query;
	SWFEXT_FUNC_BEGIN(handle);
	reshandle = Swfext_GetNumber();
	time_pot = Swfext_GetNumber();//the unit is second
	_timesec2timeline(time_pot,&time_query);
	idx = _handle2idx(reshandle);
	//printf("idx================%d\n",idx);
	if(idx!=-1){
		
		TmpLineInfo =  query_line_info(&parser_res[idx].parser_info,&time_query,StrTempBuf,STRTEMPBUF_LEN);
		//printf("TmpLineInfo.len========================%d\n",TmpLineInfo.len);
		
		if(TmpLineInfo.len==0)//query failed
			rtn = -1;
		else{
			if(TmpLineInfo.str_show==NULL)
				rtn = 0;
			else 
				rtn = 1;
		}
	}
	Swfext_PutNumber(rtn);	
	SWFEXT_FUNC_END();
}



int _subtitle_test_save_buf(void *buf,unsigned long len)
{
	FILE *ftest=NULL;
	char filename[32]="";
	unsigned long realnum=0;
	unsigned long offset=0;
	unsigned long lenleft=len;
	sprintf(filename,"/mnt/udisk/test_%d.dat",fileidx);
	ftest = fopen(filename,"wb");
	if(ftest==NULL){
		printf("Open File failed!\n");
		return 0;
	}
	printf("Buf=0x%x,len=%d\n",buf,len);
	while(1){
		if(lenleft>512){
			realnum = fwrite(buf+offset, sizeof(unsigned char), 512,ftest);
			if(realnum!=512)
				printf("Err Write!");
			offset +=realnum;
			lenleft -=realnum;
		}
		else{
			realnum = fwrite(buf+offset, sizeof(unsigned char), lenleft,ftest);
			if(realnum!=lenleft)
				printf("Erro write 222\n");
			break;
		}	
	}
	fileidx++;
	fclose(ftest);
	return 1;
}

static int parser_attach_bitmap(void *handle)
{
	int w,h;
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	subtitle_target  = Swfext_GetObject();
	w = Swfext_GetNumber();
	h = Swfext_GetNumber();
	if(TmpLineInfo.str_show!=NULL){
		
		#if 0		 ///老的VideoOSD 上用RGB565显示字幕图片
		
		printf("w@@@@@@@@@@@=%d,h@@@@@@@@@@@@@=%d!\n",w,h);
		//TmpLineInfo.str_show="test you shit";
		printf("Attach Bitmap,Buf=0x%x,w=%d,h=%d!\n",TmpLineInfo.str_show,TmpLineInfo.img_w,TmpLineInfo.img_h);
		SWF_AttachBitmap(subtitle_target,(unsigned int*)TmpLineInfo.str_show,w,h,TmpLineInfo.img_w,TmpLineInfo.img_h,TmpLineInfo.img_w,\
		SWF_BMP_FMT_RGB565,_subtitle_bitmap_get_status);
		rtn = 0;
		#else
		printf("TmpLineInfo.img_w==============%d,TmpLineInfo.img_h=============%d\n",TmpLineInfo.img_w,TmpLineInfo.img_h);
		osdengine_attach_img_align(TmpLineInfo.str_show,TmpLineInfo.img_w,TmpLineInfo.img_h,OSD_ALIGN_H_CENTER|OSD_ALIGN_V_BOTTOM);
		rtn = 0;
		#endif
		
		
	}
	else
		rtn = -1;
	Swfext_PutNumber(rtn);	
	SWFEXT_FUNC_END();
}



static void print_test(char* strprint, int len,int eachline,char *title)
{
/*
	int i=0;
	//printf("\n~~~~~~~~~~len=%d~~~title=%s~~~~~~~~~\n",len,title);
	for(i=0;i<len;i++){
		if(i%eachline==0 && i!=0)
			//printf("\n");
		//printf("%02x,",(unsigned char)(*(strprint+i)));
	}
	//printf("\n~~~~~~~~~~~~~~~~~~~~~~\n");
*/
}


/**
尝试用lib提供的文件格式进行编码转换，如果超过4次，则
将文件格式修改为 UTF8。停止编码转换
**/
static void __check_fileformat(int is_change_ok)
{
	charset_change_cnt++;
	if(is_change_ok==-1){
		if(charset_change_cnt>=4){
			sub_fileformat = SUBTITLE_UTF8;
			sub_filefix = 1;
		}
	}
	else{
		if(charset_change_cnt>=4)
			sub_filefix = 1;
	}
}


void dump_subtitle_infomations(subtitle_infomations *info)
{	
	int i=0;
	if(info==NULL)
		return ;
	sp_info("lines=%d",info->subtitle_lines);
	for(i=0;i<info->subtitle_lines;i++){
		sp_info("@@@@@@@@@@@@@idx===%d",i);
		sp_info("subtitle_length=%d",info->subtitle_length[i]);
		sp_info("subtitle_width=%d",info->subtitle_width[i]);
		print_test((char*)info->subtitle_ptr[i],(info->subtitle_length[i]+2)*2,16,"line context");
	}
}

static int parser_get_subtitle(void *handle)
{

	int out_len=STRTEMPBUF_LEN;
	int is_change_ok=-1;
	int realnum=0;
	charsetchg_info_t change_info;
	subtitle_infomations *ptr_info=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	#if 1
	memset(StrTmpTransf,0,STRTEMPBUF_LEN);
	memset(&change_info,0,sizeof(charsetchg_info_t));
	//printf("parser_get_subtitle\n");
	//printf("sub_fileformat=============0x%x\n",sub_fileformat);
	if(sub_fileformat!=SUBTITLE_UTF8){
		//printf("sub_fileformat@@@@@@@@@@\n");
		change_info.src_charset = subtitle_get_charset();
		change_info.dest_charset = LAN_UTF8;
		change_info.inputbuf = StrTempBuf;
		change_info.inputbuflen = (size_t*)&TmpLineInfo.len;
		change_info.outputbuf = StrTmpTransf;
		change_info.outputbuflen = (size_t*)&out_len;
		//print_test(StrTempBuf,TmpLineInfo.len,16,"Str Get from File2222");
		is_change_ok = change_chardec(&change_info);//会将剩余的buffer字节数放到out_len中
		sp_info("is change ok======%d, sub_fileext=%d!",is_change_ok,sub_fileext);
		//print_test(StrTmpTransf,STRTEMPBUF_LEN-out_len,16,"Str Changed");
		//sp_info("FILE_SRT is ",FILE_SRT);
		if(sub_fileext==FILE_SRT){///针对SRT的处理
			if(sub_filefix==1){///只有当file的编码格式被真正确定下来，才采取下面的策略
				if(is_change_ok==-1 && (STRTEMPBUF_LEN-out_len>=0)){
					//发现编码转换错误，但不是一开始就错误，至少有两个字是对的，还是默认为成功
					//printf("############Just used before#######\n");
					is_change_ok=0;
					StrTmpTransf[STRTEMPBUF_LEN-out_len]=0x0D;
					StrTmpTransf[STRTEMPBUF_LEN-out_len+1]=0x0A;
				}
			}
		}
		if(sub_filefix==0){
			__check_fileformat(is_change_ok);
		}
	}
	else{
		print_test(StrTempBuf,TmpLineInfo.len,16,"Str Get from File No change");
		memcpy(StrTmpTransf,StrTempBuf,STRTEMPBUF_LEN);
	}
	

	#endif
	
#if 0
		goto _____parser_get_subtitle_end____;
#endif
	if(is_change_ok==-1){
		memset(&subtitle_info,0,sizeof(subtitle_infomations));
		ptr_info=subtitle_line_feed(StrTempBuf,&TmpLineInfo.len,SUBTITLE_FONT_SIZE,(system_get_screen_para(CMD_GET_SCREEN_WIDTH)>800?800:system_get_screen_para(CMD_GET_SCREEN_WIDTH))-100);
		sp_info("ptrinfo=0x%x,subtitl_info=0x%x",ptr_info,&subtitle_info);
		dump_subtitle_infomations(ptr_info);
	}	
	else{
		out_len = STRTEMPBUF_LEN-out_len;
		memset(&subtitle_info,0,sizeof(subtitle_infomations));
		ptr_info=subtitle_line_feed(StrTmpTransf,&out_len,SUBTITLE_FONT_SIZE,(system_get_screen_para(CMD_GET_SCREEN_WIDTH)>800?800:system_get_screen_para(CMD_GET_SCREEN_WIDTH))-100);
		sp_info("ptrinfo=0x%x,subtitl_info=0x%x",ptr_info,&subtitle_info);
		dump_subtitle_infomations(ptr_info);
		
	}
	
_____parser_get_subtitle_end____:	
	Swfext_PutString(StrTmpTransf);
	sp_info("StrTmpTransf is %s",StrTmpTransf);


	SWFEXT_FUNC_END();
}

static int parser_get_linenum(void *handle)
{
	unsigned int linenum=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	linenum=subtitle_info.subtitle_lines;

	//sp_info("the linenum is %d",linenum);
		
	Swfext_PutNumber(linenum);	
	SWFEXT_FUNC_END();
}

static int parser_get_linecontent(void *handle)
{
	unsigned int line_idx=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	line_idx = Swfext_GetNumber();
	//sp_info("line_idx==%d",line_idx);
	
	memset(StrTmpTransf,0,STRTEMPBUF_LEN);
	utf16_to_utf8((short int *)subtitle_info.subtitle_ptr[line_idx], StrTmpTransf);
	//sp_info("strtmptransf===========%s",StrTmpTransf);	
	Swfext_PutString(StrTmpTransf);	

	SWFEXT_FUNC_END();
}


/***********************************
###   get the subtitle line width
************************************/
static int parser_get_linewidth(void *handle)
{
	unsigned int line_idx=0;
	unsigned short width;
	SWFEXT_FUNC_BEGIN(handle);
	
	line_idx = Swfext_GetNumber();
	//sp_info("line_idx==%d",line_idx);
	
	width= subtitle_info.subtitle_width[line_idx];
			
	Swfext_PutNumber(width);
	sp_info("width is===========%d",width);

	
	SWFEXT_FUNC_END();
}


static int parser_get_timetag(void *handle)
{
	unsigned int timetag_sec=0;
	int cmd=0;
	SWFEXT_FUNC_BEGIN(handle);
	cmd = Swfext_GetNumber();
	if(cmd==CMD_GET_TIME_START)
		timetag_sec = TmpLineInfo.t_start.hour*3600+TmpLineInfo.t_start.min*60+TmpLineInfo.t_start.sec;
	else if(cmd==CMD_GET_TIME_END)
		timetag_sec = TmpLineInfo.t_end.hour*3600+TmpLineInfo.t_end.min*60+TmpLineInfo.t_end.sec;
	else
		sp_err("CMD is not supported!");
	
	Swfext_PutNumber(timetag_sec);	
	SWFEXT_FUNC_END();
}

#else
static int parser_init_instance(void *handle)
{
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	sp_err("do nothing, open the config!");
	
	Swfext_PutNumber(rtn);	
	SWFEXT_FUNC_END();
}

static int parser_remove_instance(void *handle)
{
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	sp_err("do nothing, open the config!");
	
	Swfext_PutNumber(rtn);	
	SWFEXT_FUNC_END();
}

static int parser_parse_subtitle(void *handle)
{
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	sp_err("do nothing, open the config!");
	
	Swfext_PutNumber(rtn);	
	SWFEXT_FUNC_END();
}

static int parser_get_next_line_info(void *handle)
{
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	sp_err("do nothing, open the config!");
	
	Swfext_PutNumber(rtn);	
	SWFEXT_FUNC_END();
}
static int parser_query_line_info(void *handle)
{
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	sp_err("do nothing, open the config!");
	
	Swfext_PutNumber(rtn);	
	SWFEXT_FUNC_END();
}
static int parser_get_subtitle(void *handle)
{
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	sp_err("do nothing, open the config!");
	
	Swfext_PutNumber(rtn);	
	SWFEXT_FUNC_END();;
}
static int parser_get_timetag(void *handle)
{
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	sp_err("do nothing, open the config!");
	
	Swfext_PutNumber(rtn);	
	SWFEXT_FUNC_END();
}
static int parser_attach_bitmap(void *handle)
{
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	
	sp_err("do nothing, open the config!");
	
	Swfext_PutNumber(rtn);	
	SWFEXT_FUNC_END();
}

#endif

int swfext_subtitle_parser_register(void)
{
	_init_parser_res();
	SWFEXT_REGISTER("sp_initInstance", parser_init_instance);
	SWFEXT_REGISTER("sp_removeInstance", parser_remove_instance);
	SWFEXT_REGISTER("sp_parseSubtitle", parser_parse_subtitle);
	SWFEXT_REGISTER("sp_getNextLine", parser_get_next_line_info);
	SWFEXT_REGISTER("sp_queryLineInfo", parser_query_line_info);
	SWFEXT_REGISTER("sp_getLineInfo", parser_get_subtitle);
	SWFEXT_REGISTER("sp_getlinenum",parser_get_linenum);
	SWFEXT_REGISTER("sp_getlinecontent",parser_get_linecontent);
	SWFEXT_REGISTER("sp_getlinewidth",parser_get_linewidth);
	SWFEXT_REGISTER("sp_getTimeTag", parser_get_timetag);
	SWFEXT_REGISTER("sp_attachBitmap",parser_attach_bitmap);
	return 0;
}

