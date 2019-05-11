#ifndef _VOBSUB_SPUDEC_H_
#define _VOBSUB_SPUDEC_H_
//#include "srt_parser.h"
#include "vobsub_parser.h"
#include "subtitle_parser.h"



void spudec_assemble(void *pspudec, unsigned char *packet, unsigned int len, unsigned int pts100,parser_info_t *parser_info);
void spudec_heartbeat(void *pthis, unsigned int pts100,parser_info_t *parser_info);
int spudec_changed(void * pthis);
void *spudec_new_scaled_vobsub(unsigned int *palette, unsigned int *cuspal, unsigned int custom, unsigned int frame_width, unsigned int frame_height,parser_info_t *parser_info);

#endif