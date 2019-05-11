#include "subtitle_common.h"

static const char *g_MsgList[] = {"SUBTITLE MESSAGE","SUBTITLE ERROR"};
	
static const char *g_TestMsg = "SUBTITLE_TEST_MESSAGE";

void PrintMsg(const char *function,const int line,SUBTITLE_MSG_CODE MsgCode, char *Msg)
{
	const char *pMsgCode = NULL;
	
	switch( MsgCode )
	{
		case SUBTITLE_MESSAGE:
		case SUBTITLE_ERROR:
			pMsgCode = g_MsgList[MsgCode];
			break;
		default:
			pMsgCode = g_TestMsg;
	}
	
	if( Msg )
		printf(" [%s] %s:%d => %s .\n",pMsgCode,function,line,Msg);
	else
		printf(" [%s] %s:%d => !!!!!\n",pMsgCode,function,line);
}


/**
* clip a signed integer value into the 0-255 range
* @param a value to clip
* @return clipped value
*/
unsigned char av_clip_uint8(int a)
{
    if (a&(~255)) return (-a)>>31;
    else          return a;
}



#ifdef WINDOW_PC
char strncasecmp(const char*strsrc,const char* strdes,int size)
{
	int cmplen=size;
	int alpha_src=0;
	int alpha_des=0;
	int strlen_src=strlen(strsrc);
	if(strlen_src<size)
		return -2;
	while(cmplen>0){
		alpha_src = isalpha(*strsrc);
		alpha_des = isalpha(*strdes);
		if(alpha_src&&alpha_src){
			if(toupper(*strsrc)!=toupper(*strdes))
				return -1;
		}
		else if(alpha_src==0 && alpha_des==0){
			if(*strsrc!=*strdes)
				return -1;
		}
		else
			return -1;
		strsrc++;
		strdes++;
		cmplen--;
	}
	return 0;
}
#endif

int get_char(parser_info_t *parser_info,void*fp,char * c)
{
	int nbyte=0;
	nbyte = (long)parser_info->fops.read(fp,(unsigned char*)c,(unsigned long)1);
	return nbyte;
}

int get_filename(char*filename,char*namebuf,int size)
{
	int filenamelen=0;
	char c;
	filenamelen = strlen(filename);
	while(filenamelen>=0){
		c = *(filename+filenamelen-1);
		filenamelen--;
		if(c=='.')
			break;
	}
	if(size<=filenamelen){
		printf("Crazy:namebuf is not enough\n");
		return -1;
	}
	memcpy(namebuf,filename,filenamelen);
	return filenamelen;
}

static int set_utf16_to_utf8(char * str, int * index, const int str_size, unsigned short int u16c)
{
	int ret = -1;
	int i = *index;
	int str_max_length = str_size - 1;
	
	do
	{
		if(u16c >= 0x800)
		{
			// memory access check
			if( (i+3) >= str_max_length )
				break;
			// 3 byte
			*index = i + 3;
			str[i] = (u16c >> 12) | 0xe0;
			str[i+1] = ((u16c >> 6) & 0x3f) | 0x80;
			str[i+2] =( u16c & 0x3f) | 0x80;
		}
		else if(u16c >= 0x80)
		{
			// memory access check
			if( (i+2) >= str_max_length )
				break;
			// 2 byte
			*index = i + 2;
			str[i] = ((u16c >> 6) & 0x1f) | 0xc0;
			str[i+1] = (u16c & 0x3f) | 0x80;
		}
		else
		{
			// memory access check
			if( (i+1) >= str_max_length )
				break;
			// 1 byte
			*index = i + 1;
			str[i] = (char)u16c;
		}
		
		ret = 0; // ok
		
	}while(0);
	
	return ret;
}

int TransformLineUtf16beToUtf16le(char *lineBuf,const int lineLen)
{
	int i = 0;
	unsigned char ch = 0;
	
	while( i < lineLen )
	{
		ch = *(lineBuf+i);
		*(lineBuf+i) = *(lineBuf+i+1);
		*(lineBuf+i+1) = ch;
		i +=2;
	}
	
	return i;
}

int TransformLineUtf16ToUtf8(short int * utf16StrBuf, const int utf16StrSize)
{
	int utf8StrSize = utf16StrSize*3;
	char *utf8StrBuf = (char*)calloc(1,utf8StrSize);
	int utf8StrLen=0, j=0;
	
	if( utf8StrBuf )
	{
		do
		{
			set_utf16_to_utf8(utf8StrBuf, &utf8StrLen, utf8StrSize, utf16StrBuf[j]);
		}while( 0 != utf16StrBuf[j++] );
	
		memcpy( utf16StrBuf, utf8StrBuf, utf8StrLen ); // change to UTF-8
		memset( utf16StrBuf+utf8StrLen, 0, (utf16StrSize-utf8StrLen) );
		
		free(utf8StrBuf);
	}
	
	return utf8StrLen;
}

int utf16_to_utf8(short int * u16str, char * u8str, const int u8str_size)
{
	int i, j;
	i = 0, j = 0;
	do 
	{
		set_utf16_to_utf8(u8str, &i, u8str_size, u16str[j]);
	} while (u16str[j++] != 0);	
	return i;
}

int hunsec2time(struct time_line *time,unsigned long hunsec)
{
	unsigned long sec=hunsec/100;
	time->millsec = (unsigned short)((hunsec%100)*10);
	time->hour = (unsigned char)(sec/3600);
	sec = sec%3600;
	time->min = (unsigned char)(sec/60);
	time->sec = (unsigned char)(sec%60);
	return 0;
}

unsigned long time2hunsec(struct time_line *time)
{
	unsigned long hunsec=0;
	hunsec = (time->hour*3600+time->min*60+time->sec)*100+time->millsec/10;
	return hunsec;
}


int get_line_info(subtitle *cursub,line_info_t *line_info,char*strbuf,int len)
{
	int i=0,str_len=0,curidx=0;
	memset(strbuf,0,len);
	for(i=0;i<cursub->lines;i++){
		str_len = strlen(cursub->text[i]);
		if(str_len+curidx+2>len){
			printf("%s,%d:Sorry, The len=%d is less than %d\n",__FILE__,__LINE__,len,str_len+2+curidx);
			return -1;
		}
		memcpy(strbuf+curidx,cursub->text[i],str_len);
		curidx +=str_len;
		*(strbuf+curidx) = 0x0D;
		curidx ++;
		*(strbuf+curidx) = 0x0A;
		curidx++;
	}
	line_info->len = curidx;
	hunsec2time(&line_info->t_start,cursub->start);
	hunsec2time(&line_info->t_end,cursub->end);	
	return 0;
}

unsigned int get_file_charset(parser_info_t *parser_info)
{
	if(parser_info)
		return parser_info->ftype&0xf000;
	else 
		return 0;
}

////////////////////////////////////////////////////////////






