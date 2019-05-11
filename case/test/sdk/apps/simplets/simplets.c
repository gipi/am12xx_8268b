#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "ez_avutil.h"
#include "ez_mpegts.h"


static void *_simplets_fopen(char *path, char *mode)
{
	return (void*)fopen(path, mode);
}

static int _simplets_fclose(void *fp)
{
	return fclose((FILE*)fp);
}
static long _simplets_read(void *opaque, unsigned char *buf, long buf_size)
{
    return (long)fread(buf, sizeof(char), buf_size, (FILE*)opaque);
}

long (*write)(void *fp, unsigned char *buf, long buf_size);

static long _simplets_write(void *opaque, unsigned char *buf, long buf_size)
{
	return (long)fwrite(buf, sizeof(char), buf_size, (FILE*)opaque);
}

static long _simplets_file_seek_set(void *opaque, int64_t offset)
{
	return fseeko((FILE*)opaque, offset, SEEK_SET);
}

static long _simplets_file_seek_end(void *opaque, int64_t offset)
{
	return fseeko((FILE*)opaque, offset, SEEK_END);
}

static int64_t _simplets_get_pos(void *opaque)
{
	return ftello((FILE*)opaque);
}

static int _simplets_get_seekable(void *opaque)
{
	return 1;
}



int main(int argn, char * argc[])
{
    int ret = 0;
    AVPacket pkt1, *pkt = &pkt1;
    FILE * fp = NULL;
    const char * filename = "/mnt/udisk/stream";
    char buf[256];
    int index = 0, i = 0;
    
    AVFormatContext *ic;
    av_file_iocontext_s fio = {
        .open = _simplets_fopen,
        .read = _simplets_read,
        .write = _simplets_write,
        .seek_set = _simplets_file_seek_set,
        .seek_end = _simplets_file_seek_end,
        .tell = _simplets_get_pos,
        .close = _simplets_fclose,
        .get_seekable = NULL,
        .set_stop = NULL,
    };

    if(argn < 2){
        ez_avlog("parameter error, please set first parameter is ts file name! argn=%d\n ", argn);
        return -1;
    }
    ez_avlog("read find name is %s\n", argc[1]);

    ret = mpegts_read_header(&ic, argc[1], &fio);

    if(ret >= 0){
        if(argn > 2){
            ez_avlog("read stream index=%s\n", argc[2]);
            index = atoi(argc[2]);
        }
        snprintf(buf, sizeof(buf), "%s%d", filename, index);
        fp = fopen(buf, "wb");

        do{
            ret = mpegts_read_packet(ic, pkt);

            if(ret >= 0){
                if(ic->audio_stream_index<0 && 
                (AVMEDIA_TYPE_AUDIO==ic->streams[pkt->stream_index]->codec->codec_type)){
                    ic->audio_stream_index = pkt->stream_index;
                    ez_avlog("audio index = %d, nbstreams=%d\n", ic->audio_stream_index, ic->nb_streams);
                }else if(ic->video_stream_index<0 && 
                (AVMEDIA_TYPE_VIDEO==ic->streams[pkt->stream_index]->codec->codec_type)){
                    ic->video_stream_index = pkt->stream_index;
                    ez_avlog("video index = %d\n", ic->video_stream_index);
                }

                ez_avlog("index=%d, pts=%lld\n", pkt->stream_index, pkt->pts);
                if(fp>0 && pkt->stream_index==index){
                    ret = fwrite(pkt->data, sizeof(char), pkt->size, fp);
                    ez_avlog("WRITE:[%d][%d][%d]\n",pkt->size,ret,i++);
                }
                av_free_packet(pkt);
            }else{
                break;
            }
        }while(1);
    }

    mpegts_read_close(ic);

    if(fp>0){
        fclose(fp);
    }
    ez_avlog("ret =%d\n", ret);
    return 0;
}

