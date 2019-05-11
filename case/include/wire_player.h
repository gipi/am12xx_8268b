#ifndef __WIRE_PLAYER_H
#define __WIRE_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EXPORT_SYMBOL
#define EXPORT_SYMBOL   __attribute__ ((visibility("default")))
#endif

#define HM_PIX_FMT_YUV420S		0x020001U
#define HM_PIX_FMT_YUV422I		0x010001U
#define VIDEO_HEAP1_SIZE    	(1920*1080*2*2+1024*128+1024*1024*3)
#define WPLAYER_DEBUG			0
#define WPLAYER_TEST			0
#define VIDEO_H264_TYPE			8

typedef struct _wplayer_rect
{
	int x,y,w,h;
	int deScale,deScaleX,deScaled,rotate;
}WPLAYER_RECT;

typedef struct _wplayer_out_buf
{
	unsigned char *out_buf;
	unsigned long out_buf_addr;
	unsigned char *out_buf_uv;
	unsigned long out_buf_addr_uv;
	int res_W;
	int angleFlag;
}WPLAYER_OUT_BUF;

typedef struct _wplayer_heap_
{
	void *p_heap;
	int heap_size;
	unsigned long stream_buf_bus_addr;
}WPLAYER_HEAP;

typedef struct _wplayer_dec_
{
	void *dec_inst;
	unsigned char *stream_buf;
	unsigned long stream_bus_addr;
	unsigned long stream_len;
	unsigned char *out_buf;
	unsigned long out_buf_addr;
	unsigned char *out_buf_uv;
	unsigned long out_buf_addr_uv;
	unsigned long out_buf_width;
	unsigned long out_buf_height;
	long out_img_x;
	long out_img_y;
	unsigned long out_img_width;
	unsigned long out_img_height;
	unsigned long out_pix_fmt;
	int infifo;
}WPLAYER_DEC;

int wire_HantroOpen();
int wire_SetFrameSPSPPS(unsigned char *sps_pps, unsigned int length);
int wire_HantroPlay(unsigned char *stream, unsigned int length, unsigned long display_width, unsigned long display_height, unsigned int retato);

void wire_HantroClose();

/*
**	For iOS streaming.
*/
int decode_open(int (*read)(void *handle, unsigned char *data, int size));
void decode_close();
int decodeSeek(unsigned int millisec);
int decodeSetAudioTime(unsigned int millisec);
int decodePause();
int decodeResume();
unsigned int getStreamingDecodeMemorySize();


#ifdef __cplusplus
}
#endif

#endif

