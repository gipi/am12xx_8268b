#ifndef _SAMI_PARSER_H_
#define _SAMI_PARSER_H_

#include "subtitle_parser.h"
#include "subtitle_common.h"

#define SUB_ALIGNMENT_BOTTOMLEFT       1
#define SUB_ALIGNMENT_BOTTOMCENTER     2
#define SUB_ALIGNMENT_BOTTOMRIGHT      3
#define SUB_ALIGNMENT_MIDDLELEFT       4
#define SUB_ALIGNMENT_MIDDLECENTER     5
#define SUB_ALIGNMENT_MIDDLERIGHT      6
#define SUB_ALIGNMENT_TOPLEFT          7
#define SUB_ALIGNMENT_TOPCENTER        8
#define SUB_ALIGNMENT_TOPRIGHT         9


typedef struct sami_info_s
{
	subtitle * head;		//the pointer to the array where the subtitle info stored
	int reserve_num;		//the max space which is malloced
	int real_num;			//the real num of space which are be used
	int cur_num;			//the current idx of the num
}sami_info_t;


int sami_depose_res(parser_info_t *parser_info);
int sami_parser(parser_info_t *parser_info);
void * sami_get_info(parser_info_t *parser_info);
int sami_get_next_line(parser_info_t *parser_info,line_info_t *line_info,char*strbuf,int len);
int sami_query_line_info(parser_info_t *parser_info,struct time_line *time_query,char*strbuf,int len,line_info_t *line_info);


int _test_sami_print_subtitle(parser_info_t * parser_info);
int _test_sami_query_line_info(parser_info_t *parser_info);

#endif



















