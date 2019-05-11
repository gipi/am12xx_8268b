#ifndef FUI_COMMON_H
#define FUI_COMMON_H

#include "swf_types.h"
#include "fui_list.h"
#include "fui_input.h"

typedef struct FUI_param_s
{
	unsigned int or_x;
	unsigned int or_y;
	unsigned int pic_w;
	unsigned int pic_h;
	unsigned int flag;
}FUI_param_t;

void jpeg_dec(INT8U * JPEGData, int JPEGSize, int mode, INT8U ** Pixel, int * w, int * h, int * stride);
int  decode_bitmap(SWF_IMAGE_INFO * info);
int  play_audio(SWF_AUDIO_INFO * info);


int  fui_interval();
void fui_show(int frame_addr, void * dev);
int  fui_frame_ready(INT8U * frame, void * arg);

#endif

