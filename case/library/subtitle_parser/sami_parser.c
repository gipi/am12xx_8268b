#include "sami_parser.h"

#define LINE_LEN 1000
static char *text_buf=NULL;
static int sub_slacktime = 20000; //20 sec
int sub_no_text_pp=0;   // 1 => do not apply text post-processing
                        // like {\...} elimination in SSA format.
/* Remove leading and trailing space */
static void trail_space(char *s) {
	int i = 0;
	while (isspace(s[i])) ++i;
	if (i) strcpy(s, s + i);
	i = strlen(s) - 1;
	while (i > 0 && isspace(s[i])) s[i--] = '\0';
}

static char *stristr(const char *haystack, const char *needle) {
    int len = 0;
    const char *p = haystack;

    if (!(haystack && needle)) return NULL;

    len=strlen(needle);
    while (*p != '\0') {
	if (strncasecmp(p, needle, len) == 0) return (char*)p;
	p++;
    }

    return NULL;
}

size_t (*_sami_getline)(char **lineptr, size_t *n,parser_info_t *parser_info);

static size_t _sami_getline_utf16le(char **lineptr, size_t *n,parser_info_t *parser_info)
{
	size_t res = 0;
	char c;
	int nbyte=0;
	int is_nextline = 0;
	
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
	
	for (;nbyte!= 0; nbyte =get_char(parser_info,parser_info->fp,&c)) 
	{
		if (res + 1 >= *n) 
		{
			char *tmp = parser_info->fops.realloc(*lineptr, *n * 2);
			if (tmp == NULL)
				return -1;
			*lineptr = tmp;
			*n *= 2;
		}
		(*lineptr)[res++] = c;
		if(c==0x0A)
		{
			is_nextline = 1;
		}
		else if( (0x00==c) && (1==is_nextline) )
		{
			if((*lineptr)[res-4] ==0x0D)
				(*lineptr)[res-4] =0;
			(*lineptr)[res-2] =0;
			
			TransformLineUtf16ToUtf8((short int*)*lineptr,res);
	
			return res;
		}
	}
	if (res == 0)
	{
		return -1;
	}
	(*lineptr)[res] = 0;
	return res;
}

static size_t _sami_getline_utf16be(char **lineptr, size_t *n,parser_info_t *parser_info)
{
	size_t res = 0;
	char c;
	int nbyte=0;
	int is_nextline = 0;
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
	
	for (;nbyte!= 0; nbyte =get_char(parser_info,parser_info->fp,&c)) 
	{
		if (res + 1 >= *n) 
		{
			char *tmp = parser_info->fops.realloc(*lineptr, *n * 2);
			if (tmp == NULL)
				return -1;
			*lineptr = tmp;
			*n *= 2;
		}
		(*lineptr)[res++] = c;
		if(c==0x0A)
		{
			is_nextline = 1;
		}
		else if( (0x00==c) && (1==is_nextline) )
		{
			if((*lineptr)[res-3] ==0x0D)
				(*lineptr)[res-3] =0;
			(*lineptr)[res-1] =0;
			
			TransformLineUtf16beToUtf16le(*lineptr,res);
			TransformLineUtf16ToUtf8((short int*)*lineptr,res);
			
			return res;
		}
	}
	if (res == 0)
	{
//printf(" [ERROR] %s:%d: res == 0 !\n",__FUNCTION__,__LINE__);
		return -1;
	}
	(*lineptr)[res] = 0;
	return res;
}

static size_t _sami_getline_utf8(char **lineptr, size_t *n,parser_info_t *parser_info)
{
	size_t res = 0;
	char c;
	int nbyte=0;
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
	if( nbyte == 0 )
		printf(" [ERROR] %s:%d: get_char return 0\n",__FUNCTION__,__LINE__);
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
			if((*lineptr)[res-2] ==0x0D)
				(*lineptr)[res-2] =0;
			(*lineptr)[res-1] =0;
			(*lineptr)[res] = 0;
			return res;
		}
	}
	if (res == 0)
	{
//printf(" [ERROR] %s:%d: res == 0 !\n",__FUNCTION__,__LINE__);
		return -1;
	}
	(*lineptr)[res] = 0;
	return res;
}

static void * _sami_save_text(char *text,parser_info_t *parser_info)
{	
	char *tmpbuf=NULL;
	unsigned long str_len=strlen(text);
	tmpbuf = parser_info->fops.malloc(str_len+1);
	if(tmpbuf!=NULL){
		memset(tmpbuf,0,str_len+1);
		memcpy(tmpbuf,text,str_len);
	}
	//printf("save text len=%d,inf=%s\n",str_len,tmpbuf);
	return tmpbuf;
}

static subtitle *_sami_read_line(subtitle *current,parser_info_t *parser_info) {
	char *s = NULL, *slacktime_s;
	char* text=NULL, *p=NULL, *q;
	char *line=NULL;
	size_t line_reserved=0;
	int state;
	int line_get_len=0;
	unsigned int filecharset = get_file_charset(parser_info);

	switch( filecharset )
	{
		case SUBTITLE_UNI16_LIT:
			_sami_getline = _sami_getline_utf16le;
			break;
		case SUBTITLE_UNI16_BIG:
			_sami_getline = _sami_getline_utf16be;
			break;
		case SUBTITLE_UTF8:		
		default:
			_sami_getline = _sami_getline_utf8;
			break;
	}
	
	current->lines = current->start = current->end = 0;
	current->alignment = SUB_ALIGNMENT_BOTTOMCENTER;
	state = 0;
	text = text_buf;
    /* read the first line */
	if (!s){
		line_get_len = _sami_getline(&line,&line_reserved,parser_info);
		if (line_get_len==-1)
		{
			goto READ_LINE_ERR;
		}
		s = line;
	}

	do 
	{
		switch (state) 
		{

		case 0: /* find "START=" or "Slacktime:" */
			slacktime_s = stristr (s, "Slacktime:");
			if (slacktime_s) 
				sub_slacktime = strtol (slacktime_s+10, NULL, 0) / 10;

			s = stristr (s, "Start=");
			if (s) 
			{
				current->start = strtol (s + 6, &s, 0) / 10;
				/* eat '>' */
				for (; *s != '>' && *s != '\0'; s++);
				s++;
				state = 1; 
				continue;
			}
			break;
 
		case 1: /* find (optionnal) "<P", skip other TAGs */
			for  (; *s == ' ' || *s == '\t'; s++); /* strip blanks, if any */
			if (*s == '\0')
				break;
			if (*s != '<') { 
				state = 3; 
				memset(text,0,LINE_LEN+1);
				p = text; 
				continue; 
			} /* not a TAG */
			s++;
			if (*s == 'P' || *s == 'p') { 
				s++; 
				state = 2; 
				continue; 
			} /* found '<P' */
			for (; *s != '>' && *s != '\0'; s++); /* skip remains of non-<P> TAG */
			if (s == '\0')
				break;
			s++;
			continue;
 
		case 2: /* find ">" */
			if ((s = strchr (s, '>'))) {
				s++; 
				state = 3;
				memset(text,0,LINE_LEN+1);
				p = text; 
				continue;
			}
			break;
		case 3: /* get all text until '<' appears */
			if (*s == '\0') 
				break;
			else if (!strncasecmp (s, "<br>", 4)) {
				*p = '\0'; 
				p = text; 
				trail_space (text);
				if (text[0] != '\0')
				    current->text[current->lines++] = _sami_save_text(text,parser_info);
				s += 4;
			}
			else if ((*s == '{') && !sub_no_text_pp) { state = 5; ++s; continue; }
			else if (*s == '<') { state = 4; }
			else if (!strncasecmp (s, "&nbsp;", 6)) { *p++ = ' '; s += 6; }
			else if (*s == '\t') { *p++ = ' '; s++; }
			else if (*s == '\r' || *s == '\n') { s++; }
			else *p++ = *s++;

	    /* skip duplicated space */
			if (p > text + 2) if (*(p-1) == ' ' && *(p-2) == ' ') p--;
	    		continue;

		case 4: /* get current->end or skip <TAG> */
			q = stristr (s, "Start=");
			if (q) {
				current->end = strtol (q + 6, &q, 0) / 10 - 1;
				*p = '\0'; trail_space (text);
				if (text[0] != '\0')
				{
					// record a line of subtitle -> current->text[current->lines]
					current->text[current->lines++] = _sami_save_text(text,parser_info);
				}
				if (current->lines > 0)
				{ 
					// we've got a complete line of subtitle !
					//
					// however,
					// file pointer has pointed to the next line,
					// so, we needs to set it back
					parser_info->fops.seek_cur(parser_info->fp,(0-line_get_len));
					//fseek(parser_info->fp,(0-line_get_len),SEEK_CUR);
					state = 99; 
					break; 
				}
				state = 0; continue;
			}
			s = strchr (s, '>');
			if (s) { s++; state = 3; continue; }
	   		 break;
		case 5: /* get rid of {...} text, but read the alignment code */
			if ((*s == '\\') && (*(s + 1) == 'a') && !sub_no_text_pp) {
				if (stristr(s, "\\a1") != NULL) {
					current->alignment = SUB_ALIGNMENT_BOTTOMLEFT;
					s = s + 3;
				}
				if (stristr(s, "\\a2") != NULL) {
					current->alignment = SUB_ALIGNMENT_BOTTOMCENTER;
					s = s + 3;
				} else if (stristr(s, "\\a3") != NULL) {
					current->alignment = SUB_ALIGNMENT_BOTTOMRIGHT;
					s = s + 3;
				} else if ((stristr(s, "\\a4") != NULL) || (stristr(s, "\\a5") != NULL) || (stristr(s, "\\a8") != NULL)) {
					current->alignment = SUB_ALIGNMENT_TOPLEFT;
					s = s + 3;
				} else if (stristr(s, "\\a6") != NULL) {
					current->alignment = SUB_ALIGNMENT_TOPCENTER;
					s = s + 3;
				} else if (stristr(s, "\\a7") != NULL) {
					current->alignment = SUB_ALIGNMENT_TOPRIGHT;
					s = s + 3;
				} else if (stristr(s, "\\a9") != NULL) {
					current->alignment = SUB_ALIGNMENT_MIDDLELEFT;
					s = s + 3;
				} else if (stristr(s, "\\a10") != NULL) {
					current->alignment = SUB_ALIGNMENT_MIDDLECENTER;
					s = s + 4;
				} else if (stristr(s, "\\a11") != NULL) {
					current->alignment = SUB_ALIGNMENT_MIDDLERIGHT;
					s = s + 4;
				}
			}
			if (*s == '}') state = 3;
				++s;
			continue;
		}

		/* read next line */
		if (state != 99 )
		{
			if( ((line_get_len = _sami_getline(&line,&line_reserved,parser_info))==-1) )
			{
				if (current->start > 0) {
					break; // if it is the last subtitle
				} else {
					goto READ_LINE_ERR;
				}
				if(line_get_len==-1)
				{
					goto READ_LINE_ERR;
				}
			} 
		}
		s=line;
    } while (state != 99);

    // For the last subtitle
	if (current->end <= 0) {
	    	current->end = current->start + sub_slacktime;
		*p = '\0'; trail_space (text);
		if (text[0] != '\0')
	   	 current->text[current->lines++] = _sami_save_text(text,parser_info);
	}
	if(line!=NULL){
		parser_info->fops.free(line);
	}
	return current;
READ_LINE_ERR:
	if(line!=NULL){
		parser_info->fops.free(line);
	}
	return NULL;

}


int sami_parser(parser_info_t *parser_info)
{
	int n_max = 32; 
	int sub_num=0,sub_errs = 0;
	subtitle *first, *sub;
	sami_info_t *sami_info = (sami_info_t*)parser_info->finfo;
	
	first=parser_info->fops.malloc(n_max*sizeof(subtitle));
	if(first==NULL){
		printf("%s,%d:Malloc Space Failed!\n",__FILE__,__LINE__);
		return 0;
	}
	text_buf = parser_info->fops.malloc(LINE_LEN+1);
	if(text_buf==NULL){
		parser_info->fops.free(first);
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
		
		sub = _sami_read_line(sub,parser_info);
		if(sub==NULL){
			++sub_errs;
			break;
		}	
		else 
		{
#if 0		
			printf("*********************************\n");
			printf(" %lu -> %lu !!\n",sub->start,sub->end);
			printf("*********************************\n");
#endif
			++sub_num;
		}
	}
	sami_info->head = first;
	sami_info->cur_num = 0;
	sami_info->real_num = sub_num;
	sami_info->reserve_num = n_max;
	printf("Sub_num==%d,sub_errs==%d\n",sub_num,sub_errs);
	if(text_buf!=NULL){
		parser_info->fops.free(text_buf);
		text_buf=NULL;
	}
	return 1;
}

void * sami_get_info(parser_info_t *parser_info)
{
	sami_info_t *sami_info=NULL;
	sami_info = parser_info->fops.malloc(sizeof(sami_info_t));
	if(sami_info!=NULL){
		sami_info->cur_num=-1;
		sami_info->head = NULL;
		sami_info->real_num = 0;
		sami_info->reserve_num = 0;
	}
	return sami_info;
}


int sami_depose_res(parser_info_t *parser_info)
{
	sami_info_t *sami_info = (sami_info_t*)parser_info->finfo;	
	subtitle *tmpsub=NULL;
	int i,j;
	if(sami_info!=NULL){
		for(i=0;i<sami_info->real_num;i++){
			tmpsub = sami_info->head+i;
			for(j=0;j<tmpsub->lines;j++){
				//printf("[j=%d]addr=0x%x\n",j,tmpsub->text[j]);
				parser_info->fops.free(tmpsub->text[j]);
			}

		}
		parser_info->fops.free(sami_info->head);
		sami_info->head=NULL;
		parser_info->fops.free(parser_info->finfo);
		parser_info->finfo = NULL;
	}
	return 0;
}



int sami_get_next_line(parser_info_t *parser_info,line_info_t *line_info,char*strbuf,int len)
{
	sami_info_t *sami_info = (sami_info_t*)parser_info->finfo;	
	subtitle *tmpsub=NULL;
	int curidx=0,str_len=0;
	tmpsub = sami_info->head+sami_info->cur_num;
	if(sami_info->cur_num>=sami_info->real_num){
		printf("%s,%d:It is the last line\n",__FILE__,__LINE__);
		return -1;
	}
	if(get_line_info(tmpsub,line_info,strbuf,len)==0)
		sami_info->cur_num++;
	else
		return -1;
	return 0;
}



int sami_query_line_info(parser_info_t *parser_info,struct time_line *time_query,char*strbuf,int len,line_info_t *line_info)
{
	sami_info_t *sami_info = (sami_info_t*)parser_info->finfo;	
	subtitle * tmpsub=NULL;
	int rtn=-1;
	unsigned long hunsec;
	unsigned long millisec;
	int idx=0;
	hunsec = time2hunsec(time_query);
	millisec = hunsec * 10;
	memset(line_info,0,sizeof(line_info_t));
	//printf("%s,%d:QueryTime=%d(milli sec)\n",__FILE__,__LINE__,millisec);
	idx = sami_info->cur_num;
	if(idx>=sami_info->real_num)
		idx = sami_info->real_num - 1;
	tmpsub = sami_info->head + idx;
	if(hunsec>=tmpsub->start && hunsec <= tmpsub->end){
		get_line_info(tmpsub,line_info,strbuf,len);
		printf("%s,%d: milli sec(%d) line is found!\n",__FILE__,__LINE__,millisec);
		rtn = 0;
	}
	else if(hunsec>tmpsub->end){
		idx = sami_info->cur_num+1;
		if(idx>=sami_info->real_num){
			//printf("%s,%d:Can't Match the query time(milli sec)=%d\n",__FILE__,__LINE__,millisec);
			return -1;
		}
		while(1){
			tmpsub = sami_info->head+idx;
			if(hunsec>=tmpsub->start && hunsec <= tmpsub->end){
				if(get_line_info(tmpsub,line_info,strbuf,len)==0){
					sami_info->cur_num = idx;
					printf("%s,%d: milli sec(%d) line is found!\n",__FILE__,__LINE__,millisec);
					rtn = 0;
				}
				else
					rtn = -1;
				break;
			}
			else
				idx++;
			if(idx>=sami_info->real_num){
				//printf("%s,%d:Can't Match the query time(milli sec)=%d\n",__FILE__,__LINE__,millisec);
				return -1;
			}
		}
	}
	else if(hunsec<tmpsub->start){
		idx = sami_info->cur_num-1;
		if(idx<0){
			//printf("%s,%d:Can't Match the query time(milli sec)=%d\n",__FILE__,__LINE__,millisec);
			return -1;
		}
		while(1){
			tmpsub = sami_info->head+idx;
			if(hunsec>=tmpsub->start && hunsec <= tmpsub->end){
				if(get_line_info(tmpsub,line_info,strbuf,len)==0){
					sami_info->cur_num = idx;
					printf("%s,%d: milli sec(%d) line is found!\n",__FILE__,__LINE__,millisec);
					rtn = 0;
				}
				else
					rtn = -1;
				break;
			}
			else
				idx--;
			if(idx<0){
				//printf("%s,%d:Can't Match the query time(milli sec)=%d\n",__FILE__,__LINE__,millisec);
				return -1;
			}
		}
	}
	else 
		printf("%s,%d:Crazy:Erro\n",__FILE__,__LINE__);
	if(rtn==-1)
		line_info->len =0;
	return rtn;
}

///just for test
int _test_sami_print_subtitle(parser_info_t * parser_info)
{
	int i;
	line_info_t line_info;
	char strbuf[128];
	sami_info_t *sami_info = (sami_info_t*)parser_info->finfo;	
	for(i=0;i<sami_info->real_num;i++){
		sami_get_next_line(parser_info,&line_info,strbuf,128);
		//printf("[%d]:%02d:%02d:%02d,%03d-->%02d:%02d:%02d,%03d\n",i,line_info.t_start.hour,line_info.t_start.min,
			//line_info.t_start.sec,line_info.t_start.millsec,line_info.t_end.hour,
			//line_info.t_end.min,line_info.t_end.sec,line_info.t_end.millsec);
		//printf("%s\n",strbuf);
	}
	return 0;
}

int _test_sami_query_line_info(parser_info_t *parser_info)
{
	struct time_line time_query;
	char strbuf[128];
	line_info_t line_info;
	time_query.hour =0;
	time_query.min =0;
	time_query.sec =23;
	time_query.millsec = 0;
	sami_query_line_info(parser_info,&time_query,strbuf,128,&line_info);
	printf("%02d:%02d:%02d,%03d-->%02d:%02d:%02d,%03d\n",line_info.t_start.hour,line_info.t_start.min,\
								line_info.t_start.sec,line_info.t_start.millsec,line_info.t_end.hour,\
								line_info.t_end.min,line_info.t_end.sec,line_info.t_end.millsec);
	printf("%s\n",strbuf);
	return 0;
}
///////
