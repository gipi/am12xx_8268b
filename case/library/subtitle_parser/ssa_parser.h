#ifndef _SSA_PARSER_H_

#define _SSA_PARSER_H_

#include "subtitle_parser.h"
#include "subtitle_common.h"

typedef struct ssa_info_s
{
	subtitle * head;//the pointer to the array where the subtitle info stored
	int reserve_num;//the max space which is malloced
	int real_num;//the real num of space which are be used
	int cur_num;//the current idx of the num
}ssa_info_t;


int ssa_parser(parser_info_t *parser_info);
int ssa_depose_res(parser_info_t *parser_info);
void * ssa_get_info(parser_info_t *parser_info);
int ssa_query_line_info(parser_info_t *parser_info,struct time_line *time_query,char*strbuf,int len,line_info_t *line_info);

int _test_ssa_print_subtitle(parser_info_t * parser_info);
int _test_ssa_query_line_info(parser_info_t *parser_info);

#endif