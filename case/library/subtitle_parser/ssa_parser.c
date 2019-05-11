#include <string.h>
#include "ssa_parser.h"
#include "subtitle_parser.h"
#include "subtitle_common.h"

#define LINE_LEN 1000


size_t (*_ssa_getline)(char **lineptr, size_t *n,parser_info_t *parser_info);
static size_t _ssa_getline_utf16(char **lineptr, size_t *n,parser_info_t *parser_info)
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
		if (c == 0x00 &&is_nextline==1) 
		{	
			(*lineptr)[res] = 0;
			return res;
		}
	}
	if (res == 0)
		return -1;
	(*lineptr)[res] = 0;
	return res;
}


static size_t _ssa_getline_utf8(char **lineptr, size_t *n,parser_info_t *parser_info)
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

static void* _ssa_save_text(char* text,parser_info_t* parser_info)
{
	char *tmpbuf=NULL;
	unsigned long str_len=strlen(text);
	tmpbuf = parser_info->fops.malloc(str_len+1);
	if(tmpbuf!=NULL){
		memset(tmpbuf,0,str_len+1);
		memcpy(tmpbuf,text,str_len);
	}
	
/*	
    int i=0;
	printf("save text len=%d,inf=%s\n",str_len,tmpbuf);
	for(i=0;i<str_len;i++){
		printf("%02x ",tmpbuf[i]);
	}
*/	
	return tmpbuf;
}

static subtitle *_ssa_read_line(subtitle *current,parser_info_t *parser_info)
{
/*
* Sub Station Alpha v4 (and v2?) scripts have 9 commas before subtitle
* other Sub Station Alpha scripts have only 8 commas before subtitle
* Reading the "ScriptType:" field is not reliable since many scripts appear
* w/o it
*
* http://www.scriptclub.org is a good place to find more examples
* http://www.eswat.demon.co.uk is where the SSA specs can be found
	*/
	unsigned int file_charset=0;
	int comma;
	static int max_comma = 32; /* let's use 32 for the case that the */
	/*  amount of commas increase with newer SSA versions */
	int rtn_error=0;
	int hour1, min1, sec1, hunsec1,
		hour2, min2, sec2, hunsec2, nothing;
	int num;
	char *line=NULL,*line2=NULL,*line3=NULL,*tmp=NULL;
	size_t line_reserve = 0,line_get_len;
	short int *tmp_uni16=NULL;
	int utf8_len=0;
	
	file_charset = get_file_charset(parser_info);
	if(file_charset==SUBTITLE_UNI16_LIT)		///这个地方需要对格式进行判断
	{
		//MSG_P(SUBTITLE_MESSAGE,"ssa: This line is UTF-16\n");
		_ssa_getline = _ssa_getline_utf16;
	}
	else
	{
		//MSG_P(SUBTITLE_MESSAGE,"ssa: This line is UTF-8\n");
		_ssa_getline = _ssa_getline_utf8;
	}

	do 
	{
		// 1. get "line_get_len" as the length of "line", excluding the final '\0'
		line_get_len = _ssa_getline (&line, &line_reserve, parser_info);
		if (line_get_len==-1) 
		{
			rtn_error = 1;
			goto READ_LINE_END;
		}
		else
		{
			if(line3!=NULL){
				parser_info->fops.free(line3);
				line3 = NULL;
			}

			// 2. get "line3" to store its UTF-8-converted result of "line"
			line3 = parser_info->fops.malloc((1+line_get_len)); // mingcheng: memory access protection
			if( NULL == line3 )
			{
				MSG_P(SUBTITLE_ERROR,"ssa: Malloc Failed");
				rtn_error = 1;
				goto READ_LINE_END;
			}
		}
	
		// 3. convert to UTF-8 if it is UTF-16
		if(file_charset==SUBTITLE_UNI16_LIT)
		{///需要进行编码转换
#if CONVERT_UTF_V1
			const UTF16 *Src = (UTF16*)line;
			UTF8 *Dst = (UTF8*)line3;
			ConvertUTF16toUTF8(&Src,(UTF16*)(line+line_get_len),&Dst,(UTF8*)(line3+(1+line_get_len)),strictConversion);
			memcpy(line,line3,((char*)Dst-line3));
#endif
#if CONVERT_UTF_V0
			tmp_uni16 = (short int*)line;
			// 3-1. convert UTF-16 to UTF-8 from "line"("tmp_uni16") to "line3"
			utf8_len = utf16_to_utf8(tmp_uni16,line3,(1 + (*(int*)&line_get_len))); // mingcheng: memory access protection
			
			// 3-2.
			// copy "line3" to "line"
			// after that, "line" is in UTF-8 form
			memset(line,0,line_reserve);
			if( utf8_len < line_reserve )
				memcpy(line,line3,utf8_len);
			else
				memcpy(line,line3,(line_reserve-1));
#endif
		}
		
		// 4. parse subtitle "line", get the result in "line3"
	}  while ( (sscanf (line, "Dialogue: Marked=%d,%d:%d:%d.%d,%d:%d:%d.%d,",
				&nothing, &hour1, &min1, &sec1, &hunsec1, &hour2, &min2, &sec2, &hunsec2) < 9)
				&&
				(sscanf (line, "Dialogue: %d,%d:%d:%d.%d,%d:%d:%d.%d,",
				&nothing, &hour1, &min1, &sec1, &hunsec1, &hour2, &min2, &sec2, &hunsec2) < 9)
		/*
		while (sscanf (line, "Dialogue: Marked=%d,%d:%d:%d.%d,%d:%d:%d.%d,"
		"%s", &nothing,
		&hour1, &min1, &sec1, &hunsec1, 
		&hour2, &min2, &sec2, &hunsec2,
		line3) < 9
		*/
		/*&&                                                           // mingcheng remove: not necessary ...
		sscanf (line, "Dialogue: %d,%d:%d:%d.%d,%d:%d:%d.%d,"
		"%s", &nothing,
		&hour1, &min1, &sec1, &hunsec1, 
		&hour2, &min2, &sec2, &hunsec2,
		line3) < 9*/);

#if 0
	line2=strchr(line3, ',');
	
	for (comma = 4; comma < max_comma; comma ++)
	{
		tmp = line2;
		if(!(tmp=strchr(++tmp, ','))) break;
		if(*(++tmp) == ' ') break; 
		/* a space after a comma means we're already in a sentence */
		line2 = tmp;
	}

	if(comma < max_comma)max_comma = comma;
	/* eliminate the trailing comma */
	if(*line2 == ',') line2++;
#else
	// 5.
	// Given "line3" as subtitle-parsing result (after 3rd comma)
	// get "line2" as subtitle-advanced-parsing result (after 9th comma)
	tmp = line; /*line3;*/
	for( comma = 0; comma < 9; comma++ )
	{
		line2 = strchr(tmp, ',');
		if ( line2 )
			tmp = line2 + 1;
		else
		{
			rtn_error = 1;
			goto READ_LINE_END;
		}
	}
	line2 = tmp;
#endif
	
	// 6-1. record time information
	current->lines=0;num=0;
	current->start = 360000*hour1 + 6000*min1 + 100*sec1 + hunsec1;
	current->end   = 360000*hour2 + 6000*min2 + 100*sec2 + hunsec2;

	// 6-2. record subtitle
	while ( ((tmp=strstr(line2, "\\n")) != NULL) || ((tmp=strstr(line2, "\\N")) != NULL) )
	{
		current->text[num]=parser_info->fops.malloc(tmp-line2+1);
		strncpy (current->text[num], line2, tmp-line2);
		current->text[num][tmp-line2]='\0';
		line2=tmp+2;
		num++;
		current->lines++;
		if (current->lines >=  SUB_MAX_TEXT) 
			goto READ_LINE_END;
	}
	current->text[num]=(char*)_ssa_save_text(line2,parser_info);
	current->lines++;

READ_LINE_END:

	if(line3!=NULL)
		parser_info->fops.free(line3);
	if(line!=NULL)
		parser_info->fops.free(line);
	if(rtn_error==1)
	{
		return NULL;
	}

	return current;
}




int ssa_parser(parser_info_t *parser_info)
{
	int n_max = 32; 
	int sub_num=0,sub_errs = 0;
	subtitle *first, *sub;
	ssa_info_t *ssa_info = (ssa_info_t*)parser_info->finfo;
	first=parser_info->fops.malloc(n_max*sizeof(subtitle));
	if(first==NULL){
		printf("%s,%d:Malloc Space Failed!\n",__FILE__,__LINE__);
		return 0;
	}
	sub = &first[sub_num];
	while(1){
		if(sub_num>=n_max){
			n_max+=16;
			first=parser_info->fops.realloc(first,n_max*sizeof(subtitle));
		}
		sub = &first[sub_num];
		memset(sub, 0, sizeof(subtitle));

		sub = _ssa_read_line(sub,parser_info);
		if(sub==NULL){
			++sub_errs;
			break;
		}	
		else 
			++sub_num;
	}
	ssa_info->head = first;
	ssa_info->cur_num=0;
	ssa_info->real_num = sub_num;
	ssa_info->reserve_num = n_max;
	printf("Sub_num==%d\n",sub_num);
	return 1;
}

void * ssa_get_info(parser_info_t *parser_info)
{
	ssa_info_t *ssa_info=NULL;
	ssa_info = parser_info->fops.malloc(sizeof(ssa_info_t));
	if(ssa_info!=NULL){
		ssa_info->cur_num=-1;
		ssa_info->head = NULL;
		ssa_info->real_num = 0;
		ssa_info->reserve_num = 0;
	}
	return ssa_info;
}

int ssa_depose_res(parser_info_t *parser_info)
{
	ssa_info_t *ssa_info = (ssa_info_t*)parser_info->finfo;	
	subtitle *tmpsub=NULL;
	int i,j;
	if(ssa_info!=NULL){
		for(i=0;i<ssa_info->real_num;i++){
			tmpsub = ssa_info->head+i;
			for(j=0;j<tmpsub->lines;j++)
				parser_info->fops.free(tmpsub->text[j]);

		}
		parser_info->fops.free(ssa_info->head);
		ssa_info->head = NULL;
		parser_info->fops.free(parser_info->finfo);
		parser_info->finfo=NULL;
	}
	return 0;
}

int ssa_get_next_line(parser_info_t *parser_info,line_info_t *line_info,char*strbuf,int len)
{
	ssa_info_t *ssa_info = (ssa_info_t*)parser_info->finfo;	
	subtitle *tmpsub=NULL;
	int curidx=0,str_len=0;
	tmpsub = ssa_info->head+ssa_info->cur_num;
	if(ssa_info->cur_num>=ssa_info->real_num){
		printf("%s,%d:It is the last line\n",__FILE__,__LINE__);
		return -1;
	}
	if(get_line_info(tmpsub,line_info,strbuf,len)==0)
		ssa_info->cur_num++;
	else
		return -1;
	return 0;
}


int ssa_query_line_info(parser_info_t *parser_info,struct time_line *time_query,char*strbuf,int len,line_info_t *line_info)
{
	ssa_info_t *ssa_info = (ssa_info_t*)parser_info->finfo;	
	subtitle * tmpsub=NULL;
	int rtn=-1;
	unsigned long hunsec;
	int idx=0;
	memset(strbuf,0,len);
	memset(line_info,0,sizeof(line_info_t));
	hunsec = time2hunsec(time_query);
	idx = ssa_info->cur_num;
	if(idx>=ssa_info->real_num)
		idx = ssa_info->real_num - 1;
	tmpsub = ssa_info->head + idx;
//printf(" %s:%d: Query time(hunsec)=%d, tmpsub->start(%d), tmpsub->end(%d)\n",__FUNCTION__,__LINE__,hunsec,tmpsub->start,tmpsub->end);
	if(hunsec>=tmpsub->start && hunsec <= tmpsub->end){
		get_line_info(tmpsub,line_info,strbuf,len);
		rtn = 0;
	}
	else if(hunsec>tmpsub->end){
		idx = ssa_info->cur_num+1;
		if(idx>=ssa_info->real_num){
			//printf("%s,%d:Can't Match the query time(hundred sec)=%d\n",__FILE__,__LINE__,hunsec);
			return -1;
		}
		while(1){
			tmpsub = ssa_info->head+idx;
//printf(" %s:%d: Query time(hunsec)=%d, tmpsub->start(%d), tmpsub->end(%d)\n",__FUNCTION__,__LINE__,hunsec,tmpsub->start,tmpsub->end);			
			if(hunsec>=tmpsub->start && hunsec <= tmpsub->end){
				if(get_line_info(tmpsub,line_info,strbuf,len)==0){
					ssa_info->cur_num = idx;
					rtn = 0;
				}
				break;
			}
			else
				idx++;
			if(idx>=ssa_info->real_num){
				//printf("%s,%d:Can't Match the query time(hundred sec)=%d\n",__FILE__,__LINE__,hunsec);
				break;
			}
		}
	}
	else if(hunsec<tmpsub->start){
		idx = ssa_info->cur_num-1;
		if(idx<0){
			//printf("%s,%d:Can't Match the query time(hundred sec)=%d\n",__FILE__,__LINE__,hunsec);
			return -1;
		}
		while(1){
			tmpsub = ssa_info->head+idx;
			if(hunsec>=tmpsub->start && hunsec <= tmpsub->end){
				if(get_line_info(tmpsub,line_info,strbuf,len)==0){
					ssa_info->cur_num = idx;
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
	return rtn;
}


///just for test
int _test_ssa_print_subtitle(parser_info_t * parser_info)
{
	int i;
	line_info_t line_info;
	char strbuf[128];
	ssa_info_t *ssa_info = (ssa_info_t*)parser_info->finfo;	
	for(i=0;i<ssa_info->real_num;i++){
		ssa_get_next_line(parser_info,&line_info,strbuf,128);
		/*
		//printf("[%d]:%02d:%02d:%02d,%03d-->%02d:%02d:%02d,%03d\n",i,line_info.t_start.hour,line_info.t_start.min,\
			line_info.t_start.sec,line_info.t_start.millsec,line_info.t_end.hour,\
			line_info.t_end.min,line_info.t_end.sec,line_info.t_end.millsec);
		//printf("%s\n",strbuf);
		*/
	}
	return 0;
}

int _test_ssa_query_line_info(parser_info_t *parser_info)
{
	struct time_line time_query;
	char strbuf[128];
	line_info_t line_info;
	time_query.hour =0;
	time_query.min =1;
	time_query.sec =35;
	time_query.millsec = 0;
	ssa_query_line_info(parser_info,&time_query,strbuf,128,&line_info);
	/*
	//printf("%02d:%02d:%02d,%03d-->%02d:%02d:%02d,%03d\n",line_info.t_start.hour,line_info.t_start.min,\
								line_info.t_start.sec,line_info.t_start.millsec,line_info.t_end.hour,\
								line_info.t_end.min,line_info.t_end.sec,line_info.t_end.millsec);
	//printf("%s\n",strbuf);
	*/
	return 0;
}
///////
