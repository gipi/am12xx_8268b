#ifndef _VOBSUB_PARSER_H_
#define _VOBSUB_PARSER_H_
#include "subtitle_parser.h"
#include "vobsub_spudec.h"

enum{
	GET_PACKET_CUR,
	GET_PACKET_NEXT,
	GET_PACKET_PREV
};

enum{
	OUTPUT_FORMAT_YUV422,
	OUTPUT_FORMAT_RGB565,
	OUTPUT_FORMAT_OSD2BIT,
	OUTPUT_FORMAT_OSD8BIT
};
/* XXX Kludge ahead, this MUST be the same as the definitions found in ../spudec.c */
typedef struct packet_s packet_t;
struct packet_s {
	unsigned char *packet;
	unsigned int palette[4];
	unsigned int alpha[4];
	unsigned int control_start;	/* index of start of control data */
	unsigned int current_nibble[2]; /* next data nibble (4 bits) to be
									processed (for RLE decoding) for
	even and odd lines */
	int deinterlace_oddness;	/* 0 or 1, index into current_nibble */
	unsigned int start_col, end_col;
	unsigned int start_row, end_row;
	unsigned int width, height, stride;
	unsigned int start_pts, end_pts;
	packet_t *next;
};

typedef struct spudec_handle_s{
	packet_t *queue_head;
	packet_t *queue_tail;
	unsigned int global_palette[16];
	unsigned int orig_frame_width, orig_frame_height;
	unsigned char* packet;
	size_t packet_reserve;	/* size of the memory pointed to by packet */
	unsigned int packet_offset;	/* end of the currently assembled fragment */
	unsigned int packet_size;	/* size of the packet once all fragments are assembled */
	unsigned int packet_pts;	/* PTS for this packet */
	unsigned int palette[4];
	unsigned int alpha[4];
	unsigned int cuspal[4];
	unsigned int custom;
	unsigned int now_pts;
	unsigned int start_pts, end_pts;
	unsigned int start_col, end_col;
	unsigned int start_row, end_row;
	unsigned int width, height, stride;
	size_t image_size;		/* Size of the image buffer */
	unsigned char *image;		/* Grayscale value */
	unsigned char *aimage;	/* Alpha value */
	unsigned int scaled_frame_width, scaled_frame_height;
	unsigned int scaled_start_col, scaled_start_row;
	unsigned int scaled_width, scaled_height, scaled_stride;
	size_t scaled_image_size;
	unsigned char *scaled_image;
	unsigned char *scaled_aimage;
	int auto_palette; /* 1 if we lack a palette and must use an heuristic. */
	int font_start_level;  /* Darkest value used for the computed font */
	//vo_functions_t *hw_spu;
	int spu_changed;
	unsigned int forced_subs_only;     /* flag: 0=display all subtitle, !0 display only forced subtitles */
 	unsigned int is_forced_sub;         /* true if current subtitle is a forced subtitle */
	void* output_buf;
	int output_w;
	int output_h;
	char output_format;
} spudec_handle_t;


typedef struct {
    unsigned int pts100;
	unsigned int ptsend;
    long filepos;
    unsigned int size;
    unsigned char *data;
} raw_packet_t;

typedef struct {
    char *id;
    raw_packet_t *packets;
    int packets_reserve;
    int packets_size;
    int current_index;
} packet_queue_t;

typedef struct vobsub_s{
    unsigned int palette[16];
    unsigned int cuspal[4];
    int delay;
    unsigned int custom;
    unsigned int have_palette;
    unsigned int orig_frame_width, orig_frame_height;
    unsigned int origin_x, origin_y;
    unsigned int forced_subs;
    /* index */
    packet_queue_t *spu_streams;
    unsigned int spu_streams_size;
    unsigned int spu_streams_current;
} vobsub_t;


typedef struct mpeg_s{
    void *fp;
    unsigned int pts;
    int aid;
    unsigned char *packet;
    unsigned int packet_reserve;
    unsigned int packet_size;
} mpeg_t;

typedef struct vobsub_info_s{
	vobsub_t *vob;
	spudec_handle_t *spudec;
	unsigned int cur_time;
}vobsub_info_t;

int vobsub_init_fp(char *file,parser_info_t *parser_info);
void * vobsub_open(const int force,parser_info_t *parser_info);
int vobsub_get_next_packet(void *vobhandle, void** data,unsigned int* timestamp);
void vobsub_close(parser_info_t *parser_info);
int vobsub_depose_res(parser_info_t *parser_info);
void *vobsub_get_info(parser_info_t*parser_info);
int vobsub_show_image(unsigned char *packet,unsigned int packet_len,unsigned int pts100,parser_info_t *parser_info,line_info_t *line_info);
int _test_vobsub_query_line_info(parser_info_t *parser_info);

#endif
