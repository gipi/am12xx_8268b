#include "subtitle_parser.h"
#include "subtitle_common.h"
#include "srt_parser.h"
#define CONV_BUF_LEN 1024

size_t (*_srt_getline)(char **lineptr, size_t *n,parser_info_t *parser_info);

/**
获取一行信息，
将一行信息放到指针lineptr指向的内存中，这块内存的大小为*n
注意 : 需要外部释放
**/
static size_t _srt_getline_utf8(char **lineptr, size_t *n,parser_info_t *parser_info)
{
	size_t res = 0;
	char c;
	int nbyte=0;
	int is_nextline=0;
	if(*lineptr!=NULL)
		memset(*lineptr,0,*n);

	/**先分配4096个字节**/
	if (*lineptr == NULL) {
		*lineptr = parser_info->fops.malloc(4096);
		if (*lineptr)
			*n = 4096;
	}
	else if (*n == 0) {
		char *tmp = parser_info->fops.realloc(*lineptr, 4096);
		if (tmp) {
			*lineptr = tmp;
			*n = 4096;
		}
	}
	if (*lineptr == NULL || *n == 0)
		return -1;
	nbyte = get_char(parser_info,parser_info->fp,&c);
	for (;nbyte!= 0; nbyte =get_char(parser_info,parser_info->fp,&c)) {
		if (res + 1 >= *n) {//空间不够，需要再分配
			char *tmp = parser_info->fops.realloc(*lineptr, *n * 2);
			if (tmp == NULL)
				return -1;
			*lineptr = tmp;
			*n *= 2;
		}
		(*lineptr)[res++] = c;
		if(c==0x0A){	
			(*lineptr)[res] = 0;
			return res;
		}
	}
	if (res == 0)
		return -1;
	(*lineptr)[res] = 0;
	return res;
}

static size_t _srt_getline_utf16BE(char **lineptr, size_t *n,parser_info_t *parser_info)
{
	size_t res = 0;
	char c;
	int nbyte=0;
	int is_nextline=0;
	if(*lineptr!=NULL)
		memset(*lineptr,0,*n);
	if (*lineptr == NULL) {
		*lineptr = parser_info->fops.malloc(4096);
		if (*lineptr)
			*n = 4096;
	}
	else if (*n == 0) {
		char *tmp = parser_info->fops.realloc(*lineptr, 4096);
		if (tmp) {
			*lineptr = tmp;
			*n = 4096;
		}
	}
	if (*lineptr == NULL || *n == 0)
		return -1;
	nbyte = get_char(parser_info,parser_info->fp,&c);
	for (;nbyte!= 0; nbyte =get_char(parser_info,parser_info->fp,&c)) {
		if (res + 1 >= *n) {
			char *tmp = parser_info->fops.realloc(*lineptr, *n * 2);
			if (tmp == NULL)
				return -1;
			*lineptr = tmp;
			*n *= 2;
		}
		(*lineptr)[res++] = c;
		if(c==0x0A){	
			(*lineptr)[res] = 0;
			return res;
		}
	}
	if (res == 0)
		return -1;
	(*lineptr)[res] = 0;
	return res;
}
static size_t _srt_getline_utf16LE(char **lineptr, size_t *n,parser_info_t *parser_info)
{
	size_t res = 0;
	char c;
	int nbyte=0;
	int is_nextline=0;
	if(*lineptr!=NULL)
		memset(*lineptr,0,*n);
	if (*lineptr == NULL) {
		*lineptr = parser_info->fops.malloc(4096);
		if (*lineptr)
			*n = 4096;
	}
	else if (*n == 0) {
		char *tmp = parser_info->fops.realloc(*lineptr, 4096);
		if (tmp) {
			*lineptr = tmp;
			*n = 4096;
		}
	}
	if (*lineptr == NULL || *n == 0)
		return -1;
	nbyte = get_char(parser_info,parser_info->fp,&c);
	for (;nbyte!= 0; nbyte =get_char(parser_info,parser_info->fp,&c)) {
		if (res + 1 >= *n) {
			char *tmp = parser_info->fops.realloc(*lineptr, *n * 2);
			if (tmp == NULL)
				return -1;
			*lineptr = tmp;
			*n *= 2;
		}
		(*lineptr)[res++] = c;
		if(c==0x0A)
			is_nextline=1;
		if (c == 0x00 &&is_nextline==1) {	
			(*lineptr)[res] = 0;
			return res;
		}
	}
	if (res == 0)
		return -1;
	(*lineptr)[res] = 0;
	return res;
}

static void _srt_utf16BE_to_utf16LE(char *line)
{
	int i=0;
	unsigned char ch=0;
	while(1){
		if(*(line+i)==0 && *(line+i+1)==0)
			break;
		ch = *(line+i);
		*(line+i) = *(line+i+1);
		*(line+i+1) = ch;
		i +=2;
	}
}
/**
字符串转换函数，将utf16转换为utf8
line : utf16字符串指针
line_reserve : 字符串占用总空间

将转换之后的utf8字符串返回到line中，注意line的buffer需要留有余量
**/
static int _srt_utf16_to_utf8(char *line,size_t line_reserve,parser_info_t *parser_info)
{
	int ret = 0;
	short int *tmp_uni16=NULL;
	int utf8_len=0;
	char *line_conv=NULL;
	
	line_conv = parser_info->fops.malloc(CONV_BUF_LEN);
	if(!line_conv){
		ret  = 1;
		goto _conv_end;
	}

	tmp_uni16 = (short int*)line;
	utf8_len = utf16_to_utf8(tmp_uni16,line_conv,CONV_BUF_LEN);
	if(utf8_len>=CONV_BUF_LEN||utf8_len>=line_reserve){
		printf("Error:Enlargy conv_buf_len utf8_len=%d,conv_buf_len=%d,line_reserve=%d\n",utf8_len,CONV_BUF_LEN,line_reserve);
		ret = 1;
		goto _conv_end;
	}
	memset(line,0,line_reserve);
	memcpy(line,line_conv,utf8_len);
_conv_end:
	if(line_conv)
		parser_info->fops.free(line_conv);
	return ret;
}

static int is_digital_line(unsigned char *line)
{
	int ret=0;
	int i=0;
	while(*(line+i)!=0 &&(*(line+i)!=0x0D && *(line+i)!=0x0A)){
		if(!IS_DIGITAL(line+i)){
			ret = 0;
			return ret;
		}
		i++;
	}
	ret = 1;
	return ret;
}
/**
srt字幕行获取函数，
file_charset : 字幕文件格式
linptr : malloc出来的指针将保存到这个变量中，需要外部自己释放
n : 保存当前malloc到的空间大小
parser_info : 解析信息结构体
return :
 -1失败
 其他值，获取到的行字符数
**/
static size_t srt_getline(unsigned int file_charset,char **lineptr, size_t *n,parser_info_t *parser_info)
{
	int ret=0;
	size_t line_get_len;
	if(file_charset==SUBTITLE_UNI16_LIT)		///这个地方需要对格式进行判断
		_srt_getline =_srt_getline_utf16LE;
	else if(file_charset==SUBTITLE_UNI16_BIG)
		_srt_getline =_srt_getline_utf16BE;
	else
		_srt_getline = _srt_getline_utf8;

	line_get_len =  _srt_getline(lineptr,n,parser_info);
	if (line_get_len==-1) {
		goto __end;
	}

	if(file_charset==SUBTITLE_UNI16_LIT){
		ret = _srt_utf16_to_utf8(*lineptr,*n,parser_info);
		if(ret==1){
			line_get_len=-1;
			goto __end;
		}
	}
	else if(file_charset==SUBTITLE_UNI16_BIG){
		_srt_utf16BE_to_utf16LE(*lineptr);
		ret = _srt_utf16_to_utf8(*lineptr,*n,parser_info);
		if(ret==1){
			line_get_len=-1;
			goto __end;
		}
	}

__end:
	return line_get_len;
}

static subtitle *_srt_read_line(subtitle *current,parser_info_t *parser_info) 
{
	int rtn_error=0;
	char *line=NULL;
	size_t line_reserve = 0,line_get_len;
	int a1,a2,a3,a4,b1,b2,b3,b4;
	char *p=NULL;
	char *curptr=NULL;
	int i,len;
	unsigned int file_charset=0;
	file_charset = get_file_charset(parser_info);
	
 
	while (!current->text[0]) {
		line_get_len = srt_getline(file_charset,&line, &line_reserve, parser_info);
		if (line_get_len==-1) {
			rtn_error = 1;
			goto READ_LINE_END;
		}	
		if ((len=sscanf (line, "%d:%d:%d%[,.:]%d --> %d:%d:%d%[,.:]%d",&a1,&a2,&a3,(char *)&i,&a4,&b1,&b2,&b3,(char *)&i,&b4)) < 10)
			continue;

_fill_time:
		/**填充时间**/
		current->start = a1*360000+a2*6000+a3*100+a4/10;
		current->end   = b1*360000+b2*6000+b3*100+b4/10;
		
		for (i=0; i<SUB_MAX_TEXT;) {
			int blank = 1;
				
			line_get_len = srt_getline(file_charset,&line, &line_reserve, parser_info);///这是一行字幕信息
			if (line_get_len==-1) {
				rtn_error = 1;
				goto READ_LINE_END;
			}
_new_line:
			len=0;
			
			for (p=line; *p!='\n' && *p!='\r' && *p; p++,len++)
				if (*p != ' ' && *p != '\t')
					blank = 0;
			
			if (len && !blank) {
				int j=0,skip=0;
				
				curptr=current->text[i]=parser_info->fops.malloc(len+1);
				if (!current->text[i]){
					rtn_error = 1;
					goto READ_LINE_END;
				}
				for(; j<len; j++) {
				/* let's filter html tags ::atmos */
				if(line[j]=='>') {
					skip=0;
					continue;
				}
				if(line[j]=='<') {
					skip=1;
					continue;
				}
				if(skip) {
					continue;
				}
				*curptr=line[j];
				curptr++;
				}
				*curptr='\0';
				i++;
			} else {
				if(len==0){///如果该行没有内容，需要预读取下一行判断是否是新的一行
					line_get_len = srt_getline(file_charset,&line, &line_reserve, parser_info);///这是一行字幕信息
					if (line_get_len==-1) {
						rtn_error = 1;
						goto READ_LINE_END;
					}
					if ((len=sscanf (line, "%d:%d:%d%[,.:]%d --> %d:%d:%d%[,.:]%d",&a1,&a2,&a3,(char *)&i,&a4,&b1,&b2,&b3,(char *)&i,&b4)) < 10){
							if(is_digital_line((unsigned char *)line)==1)///新的一行全部是数字，当它是另外一个新时间点的行index
								break;
							///是新的时间点字幕信息，需要加入一行空行字幕，然后解析已经预读到的该行字幕
							curptr=current->text[i]=parser_info->fops.malloc(1);
							if (!current->text[i]){
								rtn_error = 1;
								goto READ_LINE_END;
							}
							*curptr='\0';
							i++;
							goto _new_line;
					}
					else///不是新的时间点字幕信息
						goto _fill_time;
				}
				break;
			}
		}
		current->lines=i;
	}


READ_LINE_END:
	
	if(line!=NULL)
		parser_info->fops.free(line);
	if(rtn_error==1)
		return NULL;
	return current;
}



int srt_parser(parser_info_t *parser_info)
{
	int n_max = 32; 
	int sub_num=0,sub_errs = 0;
	subtitle *first, *sub;
	srt_info_t *srt_info = (srt_info_t*)parser_info->finfo;
	first=parser_info->fops.malloc(n_max*sizeof(subtitle));
	if(first==NULL){
		printf("%s,%d:Malloc Space Failed!\n",__FILE__,__LINE__);
		return 0;
	}
	sub = &first[sub_num];
	while(1){
		if(sub_num>=n_max){//先分配32 行字幕空间，不够时再进行分配
			n_max+=16;
			first=parser_info->fops.realloc(first,n_max*sizeof(subtitle));
		}
		sub = &first[sub_num];
		memset(sub, 0, sizeof(subtitle));

		sub = _srt_read_line(sub,parser_info);
		if(sub==NULL){
			++sub_errs;
			break;
		}	
		else 
			++sub_num;
	}
	srt_info->head = first;
	srt_info->cur_num=0;
	srt_info->real_num = sub_num;
	srt_info->reserve_num = n_max;
	printf("Sub_num==%d\n",sub_num);
	return 1;
}

// initialization
void * srt_get_info(parser_info_t *parser_info)
{
	srt_info_t *srt_info=NULL;
	srt_info = parser_info->fops.malloc(sizeof(srt_info_t));
	if(srt_info!=NULL){
		srt_info->cur_num=-1;
		srt_info->head = NULL;
		srt_info->real_num = 0;
		srt_info->reserve_num = 0;
	}
	return srt_info;
}


int srt_depose_res(parser_info_t *parser_info)
{
	srt_info_t *srt_info = (srt_info_t*)parser_info->finfo;	
	subtitle *tmpsub=NULL;
	int i,j;
	if(srt_info!=NULL){
		for(i=0;i<srt_info->real_num;i++){
			tmpsub = srt_info->head+i;
			for(j=0;j<tmpsub->lines;j++)
				parser_info->fops.free(tmpsub->text[j]);

		}
		parser_info->fops.free(srt_info->head);
		srt_info->head = NULL;
		parser_info->fops.free(parser_info->finfo);
		parser_info->finfo=NULL;
	}
	return 0;
}



int srt_get_next_line(parser_info_t *parser_info,line_info_t *line_info,char*strbuf,int len)
{
	srt_info_t *srt_info = (srt_info_t*)parser_info->finfo;	
	subtitle *tmpsub=NULL;
	int curidx=0,str_len=0;
	tmpsub = srt_info->head+srt_info->cur_num;
	if(srt_info->cur_num>=srt_info->real_num){
		printf("%s,%d:It is the last line\n",__FILE__,__LINE__);
		return -1;
	}
	if(get_line_info(tmpsub,line_info,strbuf,len)==0)
		srt_info->cur_num++;
	else
		return -1;
	return 0;
}




int srt_query_line_info(parser_info_t *parser_info,struct time_line *time_query,char*strbuf,int len,line_info_t *line_info)
{
	srt_info_t *srt_info = (srt_info_t*)parser_info->finfo;	
	subtitle * tmpsub=NULL;
	int rtn=-1;
	unsigned long hunsec;
	int idx=0;
	memset(strbuf,0,len);
	memset(line_info,0,sizeof(line_info_t));
	hunsec = time2hunsec(time_query);
	
	//printf("Query time(hunsec)=%d\n",hunsec);
	idx = srt_info->cur_num;
	if(idx>=srt_info->real_num)
		idx = srt_info->real_num - 1;
	tmpsub = srt_info->head + idx;
	if(hunsec>=tmpsub->start && hunsec <= tmpsub->end){
		get_line_info(tmpsub,line_info,strbuf,len);
		rtn = 0;
	}
	else if(hunsec>tmpsub->end){
		idx = srt_info->cur_num+1;
		if(idx>=srt_info->real_num){
			//printf("%s,%d:Can't Match the query time(hundred sec)=%d\n",__FILE__,__LINE__,hunsec);
			goto EXIT;
		}
		while(1){
			tmpsub = srt_info->head+idx;
			if(hunsec>=tmpsub->start && hunsec <= tmpsub->end){
				if(get_line_info(tmpsub,line_info,strbuf,len)==0){
					srt_info->cur_num = idx;
					rtn = 0;
				}
				break;
			}
			else
				idx++;
			if(idx>=srt_info->real_num){
				//printf("%s,%d:Can't Match the query time(hundred sec)=%d\n",__FILE__,__LINE__,hunsec);
				break;
			}
		}
	}
	else if(hunsec<tmpsub->start){
		idx = srt_info->cur_num-1;
		if(idx<0){
			//printf("%s,%d:Can't Match the query time(hundred sec)=%d\n",__FILE__,__LINE__,hunsec);
			goto EXIT;
		}
		while(1){
			tmpsub = srt_info->head+idx;
			if(hunsec>=tmpsub->start && hunsec <= tmpsub->end){
				if(get_line_info(tmpsub,line_info,strbuf,len)==0){
					srt_info->cur_num = idx;
					rtn = 0;
				}
				break;
			}
			else
				idx--;
			if(idx<0){
				//printf("%s,%d:Can't Match the query time(hundred sec)=%d\n",__FILE__,__LINE__,hunsec);
				break;
			}
		}
	}
	else 
		printf("%s,%d:Crazy:Erro\n",__FILE__,__LINE__);
	if(rtn==-1)
		line_info->len = 0;
		
EXIT:		
	return rtn;
}

int srt_dump_file(parser_info_t * parser_info)
{
		FILE *fp=NULL;
		int i;
		line_info_t line_info;
		char strbuf[128];
		srt_info_t *srt_info = (srt_info_t*)parser_info->finfo;	
		fp = fopen("srt_test.txt","wb+");
		if(fp==NULL){
			printf("open test dump file error!\n");
			return -1;
		}

/*

		for(i=0;i<srt_info->real_num;i++){
			srt_get_next_line(parser_info,&line_info,strbuf,128);
			fprintf(fp,"[%d]:%02d:%02d:%02d,%03d-->%02d:%02d:%02d,%03d\n",i,line_info.t_start.hour,line_info.t_start.min,\
				line_info.t_start.sec,line_info.t_start.millsec,line_info.t_end.hour,\
				line_info.t_end.min,line_info.t_end.sec,line_info.t_end.millsec);
			fprintf(fp,"%s\n",strbuf);
		}
*/
		fprintf(fp,"%d",12121212);	
		fclose(fp);
		return 0;

}

///just for test
int _test_srt_print_subtitle(parser_info_t * parser_info)
{
	int i;
	line_info_t line_info;
	char strbuf[128];
	srt_info_t *srt_info = (srt_info_t*)parser_info->finfo;	
	for(i=0;i<srt_info->real_num;i++){
		srt_get_next_line(parser_info,&line_info,strbuf,128);
		//printf("[%d]:%02d:%02d:%02d,%03d-->%02d:%02d:%02d,%03d\n",i,line_info.t_start.hour,line_info.t_start.min,
			//line_info.t_start.sec,line_info.t_start.millsec,line_info.t_end.hour,
			//line_info.t_end.min,line_info.t_end.sec,line_info.t_end.millsec);
		//printf("%s\n",strbuf);
	}
//	srt_dump_file(parser_info);
	return 0;
}

int _test_srt_query_line_info(parser_info_t *parser_info)
{
	struct time_line time_query;
	char strbuf[128];
	line_info_t line_info;
	time_query.hour =0;
	time_query.min =1;
	time_query.sec =35;
	time_query.millsec = 0;
	srt_query_line_info(parser_info,&time_query,strbuf,128,&line_info);
	printf("%02d:%02d:%02d,%03d-->%02d:%02d:%02d,%03d\n",line_info.t_start.hour,line_info.t_start.min,\
								line_info.t_start.sec,line_info.t_start.millsec,line_info.t_end.hour,\
								line_info.t_end.min,line_info.t_end.sec,line_info.t_end.millsec);
	printf("%s\n",strbuf);
	return 0;
}
///////

