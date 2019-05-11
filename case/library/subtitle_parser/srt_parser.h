#ifndef __SRT_PARSER_NEW_H_

#define __SRT_PARSER_NEW_H_

#include "subtitle_parser.h"
#include "subtitle_common.h"

typedef struct srt_info_s
{
	subtitle * head;//the pointer to the array where the subtitle info stored
	int reserve_num;//the max space which is malloced
	int real_num;//the real num of space which are be used
	int cur_num;//the current idx of the num
}srt_info_t;



int srt_parser(parser_info_t *parser_info);
int srt_depose_res(parser_info_t *parser_info);
void * srt_get_info(parser_info_t *parser_info);

int _test_srt_print_subtitle(parser_info_t * parser_info);
int _test_srt_query_line_info(parser_info_t *parser_info);

#endif
