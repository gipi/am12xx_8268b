#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "ez_avutil.h"


av_i64 ff_gcd(av_i64 a, av_i64 b){
    if(b) return ff_gcd(b, a%b);
    else  return a;
}

void av_set_pts_info(AVStream *s, int pts_wrap_bits,
                     int pts_num, int pts_den)
{
    unsigned int gcd= ff_gcd((av_i64)pts_num, (av_i64)pts_den);
    s->time_base.num = pts_num/gcd;
    s->time_base.den = pts_den/gcd;
}


int ff_find_stream_index(AVFormatContext *s, int id)
{
    int i;
    for (i = 0; i < s->nb_streams; i++) {
        if (s->streams[i]->id == id)
            return i;
    }
    return -1;
}


AVCodecContext *avcodec_alloc_context(void)
{
   AVCodecContext *s= (AVCodecContext *)av_mallocz(sizeof(AVCodecContext));
   if(s==NULL) return NULL;
   s->codec_type = AV_CODEC_ID_NONE;
   return s;
}

AVStream *avformat_new_stream(AVFormatContext *s, int id)
{
    AVStream *st;
	
    if (s->nb_streams >= MAX_STREAMS)
        return NULL;
	
    st = (AVStream *)av_mallocz(sizeof(AVStream));
    if (!st)
        return NULL;
	
    st->codec= avcodec_alloc_context();

    st->index = s->nb_streams;
    st->id = id;
    st->start_time = AV_NOPTS_VALUE;
    st->duration = AV_NOPTS_VALUE;
	/* we set the current DTS to 0 so that formats without any timestamps
	but durations get some timestamps, formats with some unknown
	timestamps have their first few packets buffered and the
	timestamps corrected before they are returned to the user */
	
    /* default pts setting is MPEG-like */
    av_set_pts_info(st, 33, 1, 90000);
	
    s->streams[s->nb_streams++] = st;
    return st;
}


void * packet_malloc(int size, unsigned int *pbus)
{
    void * ptr;

    ptr = av_malloc(size);
    *pbus = 0;

    return ptr;
}

void packet_free(void * ptr)
{
    if(ptr){
        av_free(ptr);
    }
}

void packet_freep(void *arg)
{
    void **ptr = (void **)arg;
    packet_free(*ptr);
    *ptr = NULL;
}


void av_init_packet(AVPacket *pkt)
{
    pkt->pts                  = AV_NOPTS_VALUE;
    pkt->dts                  = AV_NOPTS_VALUE;
    pkt->pos                  = -1;
    pkt->duration             = 0;
    pkt->bus_addr            = 0;
    pkt->convergence_duration = 0;
    pkt->flags                = 0;
    pkt->stream_index         = 0;
    pkt->destruct             = NULL;
}

int av_new_packet(AVPacket *pkt, int size)
{
    uint8_t *data = NULL;
    if ((unsigned)size < (unsigned)size + FF_INPUT_BUFFER_PADDING_SIZE)
        data = (uint8_t *)packet_malloc(size + FF_INPUT_BUFFER_PADDING_SIZE, &pkt->bus_addr);
    if (data) {
        memset(data + size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
    } else
        size = 0;

    av_init_packet(pkt);
    pkt->data     = data;
    pkt->size     = size;
    pkt->destruct = av_destruct_packet;
    if (!data)
        return AVERROR(ENOMEM);
    return 0;
}

void av_destruct_packet(AVPacket *pkt)
{
    packet_free(pkt->data);
    pkt->data = NULL;
    pkt->size = 0;
    pkt->bus_addr            = 0;
}

void av_free_packet(AVPacket *pkt)
{
    if (pkt) {
        if (pkt->destruct)
            pkt->destruct(pkt);
        pkt->data            = NULL;
        pkt->size            = 0;
        pkt->bus_addr            = 0;
    }
}



