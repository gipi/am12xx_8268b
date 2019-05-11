#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>


#include "ez_avutil.h"
#include "ez_mpegts.h"
#include "crc.h"



/* maximum size in which we look for synchronisation if
   synchronisation is lost */
#define MAX_RESYNC_SIZE 65536

#define MAX_PES_PAYLOAD 200*1024

enum MpegTSFilterType {
    MPEGTS_PES,
    MPEGTS_SECTION,
};

typedef struct MpegTSFilter MpegTSFilter;

typedef int PESCallback(MpegTSFilter *f, const uint8_t *buf, int len, int is_start, int64_t pos);

typedef struct MpegTSPESFilter {
    PESCallback *pes_cb;
    void *opaque;
} MpegTSPESFilter;

typedef void SectionCallback(MpegTSFilter *f, const uint8_t *buf, int len);

typedef void SetServiceCallback(void *opaque, int ret);

typedef struct MpegTSSectionFilter {
    int section_index;
    int section_h_size;
    uint8_t *section_buf;
    unsigned int check_crc:1;
    unsigned int end_of_section_reached:1;
    SectionCallback *section_cb;
    void *opaque;
} MpegTSSectionFilter;

struct MpegTSFilter {
    int pid;
    int es_id;
    int last_cc; /* last cc code (-1 if first packet) */
    enum MpegTSFilterType type;
    union {
        MpegTSPESFilter pes_filter;
        MpegTSSectionFilter section_filter;
    } u;
};

struct MpegTSContext {
    /* user data */
    AVFormatContext *stream;
    /** raw packet size, including FEC if present            */
    int raw_packet_size;

    int pos47;

    /** if true, all pids are analyzed to find streams       */
    int auto_guess;

    /** compute exact PCR for each transport stream packet   */
    int mpeg2ts_compute_pcr;

    int64_t cur_pcr;    /**< used to estimate the exact PCR  */
    int pcr_incr;       /**< used to estimate the exact PCR  */

    /* data needed to handle file based ts */
    /** stop parsing loop                                    */
    int stop_parse;
    /** packet containing Audio/Video data                   */
    AVPacket *pkt;
    int64_t start_pts;
    /** to detect seek                                       */
    int64_t last_pos;

    int8_t crc_validity[NB_PID_MAX];

    /** filters for various streams specified by PMT + for the PAT and PMT */
    MpegTSFilter *pids[NB_PID_MAX];
};

/* TS stream handling */

enum MpegTSState {
    MPEGTS_HEADER = 0,
    MPEGTS_PESHEADER,
    MPEGTS_PESHEADER_FILL,
    MPEGTS_PAYLOAD,
    MPEGTS_SKIP,
};

/* enough for PES header + length */
#define PES_START_SIZE  6
#define PES_HEADER_SIZE 9
#define MAX_PES_HEADER_SIZE (9 + 255)

typedef struct PESContext {
    int pid;
    int pcr_pid; /**< if -1 then all packets containing PCR are considered */
    int stream_type;
    MpegTSContext *ts;
    AVFormatContext *stream;
    AVStream *st;
    AVStream *sub_st; /**< stream for the embedded AC3 stream in HDMV TrueHD */
    enum MpegTSState state;
    /* used to get the format */
    int data_index;
    int flags; /**< copied to the AVPacket flags */
    int total_size;
    int pes_header_size;
    int extended_stream_id;
    int64_t pts, dts;
    int64_t ts_packet_pos; /**< position of first TS packet of this PES packet */
    uint8_t header[MAX_PES_HEADER_SIZE];
    uint8_t *buffer;
    unsigned int buffer_bus;
} PESContext;


/**
 *  Assemble PES packets out of TS packets, and then call the "section_cb"
 *  function when they are complete.
 */
static void write_section_data(AVFormatContext *s, MpegTSFilter *tss1,
                               const uint8_t *buf, int buf_size, int is_start)
{
    MpegTSContext *ts = s->priv_data;
    MpegTSSectionFilter *tss = &tss1->u.section_filter;
    int len;

    if (is_start) {
        memcpy(tss->section_buf, buf, buf_size);
        tss->section_index = buf_size;
        tss->section_h_size = -1;
        tss->end_of_section_reached = 0;
    } else {
        if (tss->end_of_section_reached)
            return;
        len = 4096 - tss->section_index;
        if (buf_size < len)
            len = buf_size;
        memcpy(tss->section_buf + tss->section_index, buf, len);
        tss->section_index += len;
    }

    /* compute section length if possible */
    if (tss->section_h_size == -1 && tss->section_index >= 3) {
        len = (AV_RB16(tss->section_buf + 1) & 0xfff) + 3;
        if (len > 4096)
            return;
        tss->section_h_size = len;
    }
    
    if (tss->section_h_size != -1 && tss->section_index >= tss->section_h_size) {
        tss->end_of_section_reached = 1;
        #if 1
        if (!tss->check_crc ||
            av_crc(av_crc_get_table(AV_CRC_32_IEEE), -1,
                   tss->section_buf, tss->section_h_size) == 0)
            tss->section_cb(tss1, tss->section_buf, tss->section_h_size);
        #else
        tss->section_cb(tss1, tss->section_buf, tss->section_h_size);
        #endif
    }

}

static MpegTSFilter *mpegts_open_section_filter(MpegTSContext *ts, unsigned int pid,
                                         SectionCallback *section_cb, void *opaque,
                                         int check_crc)

{
    MpegTSFilter *filter;
    MpegTSSectionFilter *sec;

    av_dlog(ts->stream, "Filter: pid=0x%x\n", pid);

    if (pid >= NB_PID_MAX || ts->pids[pid])
        return NULL;
    filter = (void *)av_mallocz(sizeof(MpegTSFilter));
    if (!filter)
        return NULL;
    ts->pids[pid] = filter;
    filter->type = MPEGTS_SECTION;
    filter->pid = pid;
    filter->es_id = -1;
    filter->last_cc = -1;
    sec = &filter->u.section_filter;
    sec->section_cb = section_cb;
    sec->opaque = opaque;
    sec->section_buf = av_malloc(MAX_SECTION_SIZE);
    sec->check_crc = check_crc;
    if (!sec->section_buf) {
        av_free(filter);
        return NULL;
    }
    return filter;
}

static MpegTSFilter *mpegts_open_pes_filter(MpegTSContext *ts, unsigned int pid,
                                     PESCallback *pes_cb,
                                     void *opaque)
{
    MpegTSFilter *filter;
    MpegTSPESFilter *pes;

    if (pid >= NB_PID_MAX || ts->pids[pid])
        return NULL;
    filter = av_mallocz(sizeof(MpegTSFilter));
    if (!filter)
        return NULL;
    ts->pids[pid] = filter;
    filter->type = MPEGTS_PES;
    filter->pid = pid;
    filter->es_id = -1;
    filter->last_cc = -1;
    pes = &filter->u.pes_filter;
    pes->pes_cb = pes_cb;
    pes->opaque = opaque;
    return filter;
}

static void mpegts_close_filter(MpegTSContext *ts, MpegTSFilter *filter)
{
    int pid;

    pid = filter->pid;
    if (filter->type == MPEGTS_SECTION){
        av_freep(&filter->u.section_filter.section_buf);
    }else if (filter->type == MPEGTS_PES) {
        PESContext *pes = filter->u.pes_filter.opaque;
        packet_freep(&pes->buffer);
        av_freep(&filter->u.pes_filter.opaque);
    }

    av_free(filter);
    ts->pids[pid] = NULL;
}


static int analyze(const uint8_t *buf, int size, int packet_size, int *index){
    int stat[TS_MAX_PACKET_SIZE];
    int i;
    int x=0;
    int best_score=0;

    memset(stat, 0, packet_size*sizeof(int));

    for(x=i=0; i<size-3; i++){
        if(buf[i] == 0x47 && !(buf[i+1] & 0x80) && buf[i+3] != 0x47){
            stat[x]++;
            if(stat[x] > best_score){
                best_score= stat[x];
                if(index) *index= x;
            }
        }

        x++;
        if(x == packet_size) x= 0;
    }

    return best_score;
}

/* autodetect fec presence. Must have at least 1024 bytes  */
static int get_packet_size(const uint8_t *buf, int size)
{
    int score, fec_score, dvhs_score;

    if (size < (TS_FEC_PACKET_SIZE * 5 + 1))
        return -1;

    score    = analyze(buf, size, TS_PACKET_SIZE, NULL);
    dvhs_score    = analyze(buf, size, TS_DVHS_PACKET_SIZE, NULL);
    fec_score= analyze(buf, size, TS_FEC_PACKET_SIZE, NULL);
    av_dlog(NULL, "score: %d, dvhs_score: %d, fec_score: %d \n",
            score, dvhs_score, fec_score);

    if     (score > fec_score && score > dvhs_score) return TS_PACKET_SIZE;
    else if(dvhs_score > score && dvhs_score > fec_score) return TS_DVHS_PACKET_SIZE;
    else if(score < fec_score && dvhs_score < fec_score) return TS_FEC_PACKET_SIZE;
    else                       return -1;
}

typedef struct SectionHeader {
    uint8_t tid;
    uint16_t id;
    uint8_t version;
    uint8_t sec_num;
    uint8_t last_sec_num;
} SectionHeader;

static inline int get8(const uint8_t **pp, const uint8_t *p_end)
{
    const uint8_t *p;
    int c;

    p = *pp;
    if (p >= p_end)
        return -1;
    c = *p++;
    *pp = p;
    return c;
}

static inline int get16(const uint8_t **pp, const uint8_t *p_end)
{
    const uint8_t *p;
    int c;

    p = *pp;
    if ((p + 1) >= p_end)
        return -1;
    c = AV_RB16(p);
    p += 2;
    *pp = p;
    return c;
}


static int parse_section_header(SectionHeader *h,
                                const uint8_t **pp, const uint8_t *p_end)
{
    int val;

    val = get8(pp, p_end);
    if (val < 0)
        return -1;
    h->tid = val;
    *pp += 2;
    val = get16(pp, p_end);
    if (val < 0)
        return -1;
    h->id = val;
    val = get8(pp, p_end);
    if (val < 0)
        return -1;
    h->version = (val >> 1) & 0x1f;
    val = get8(pp, p_end);
    if (val < 0)
        return -1;
    h->sec_num = val;
    val = get8(pp, p_end);
    if (val < 0)
        return -1;
    h->last_sec_num = val;
    return 0;
}

typedef struct {
    uint32_t stream_type;
    enum CodecType codec_type;
    enum CodecID codec_id;
} StreamType;

static const StreamType ISO_types[] = {
    { 0x01, AVMEDIA_TYPE_VIDEO, AV_CODEC_ID_MPEG2VIDEO },
    { 0x02, AVMEDIA_TYPE_VIDEO, AV_CODEC_ID_MPEG2VIDEO },
    { 0x03, AVMEDIA_TYPE_AUDIO,        AV_CODEC_ID_MP3 },
    { 0x04, AVMEDIA_TYPE_AUDIO,        AV_CODEC_ID_MP3 },
    { 0x0f, AVMEDIA_TYPE_AUDIO,        AV_CODEC_ID_AAC },
    { 0x10, AVMEDIA_TYPE_VIDEO,      AV_CODEC_ID_MPEG4 },
    /* Makito encoder sets stream type 0x11 for AAC,
     * so auto-detect LOAS/LATM instead of hardcoding it. */
//  { 0x11, AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_AAC_LATM }, /* LATM syntax */
    { 0x1b, AVMEDIA_TYPE_VIDEO,       AV_CODEC_ID_H264 },
    { 0xd1, AVMEDIA_TYPE_VIDEO,      AV_CODEC_ID_DIRAC },
    { 0xea, AVMEDIA_TYPE_VIDEO,        AV_CODEC_ID_VC1 },
    { 0 },
};

static const StreamType HDMV_types[] = {
    { 0x80, AVMEDIA_TYPE_AUDIO, AV_CODEC_ID_PCM_BLURAY },
    { 0x81, AVMEDIA_TYPE_AUDIO, AV_CODEC_ID_AC3 },
    { 0x82, AVMEDIA_TYPE_AUDIO, AV_CODEC_ID_DTS },
    { 0x83, AVMEDIA_TYPE_AUDIO, AV_CODEC_ID_TRUEHD },
    { 0x84, AVMEDIA_TYPE_AUDIO, AV_CODEC_ID_EAC3 },
    { 0x85, AVMEDIA_TYPE_AUDIO, AV_CODEC_ID_DTS }, /* DTS HD */
    { 0x86, AVMEDIA_TYPE_AUDIO, AV_CODEC_ID_DTS }, /* DTS HD MASTER*/
    { 0xa1, AVMEDIA_TYPE_AUDIO, AV_CODEC_ID_EAC3 }, /* E-AC3 Secondary Audio */
    { 0xa2, AVMEDIA_TYPE_AUDIO, AV_CODEC_ID_DTS },  /* DTS Express Secondary Audio */
    { 0x90, AVMEDIA_TYPE_SUBTITLE, AV_CODEC_ID_HDMV_PGS_SUBTITLE },
    { 0 },
};

/* ATSC ? */
static const StreamType MISC_types[] = {
    { 0x81, AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_AC3 },
    { 0x83, AVMEDIA_TYPE_AUDIO, AV_CODEC_ID_PCM_BLURAY },//now just use AV_CODEC_ID_PCM_BLURAY
    { 0x8a, AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_DTS },
    { 0 },
};

static const StreamType REGD_types[] = {
    { MKTAG('d','r','a','c'), AVMEDIA_TYPE_VIDEO, AV_CODEC_ID_DIRAC },
    { MKTAG('A','C','-','3'), AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_AC3 },
    { MKTAG('B','S','S','D'), AVMEDIA_TYPE_AUDIO, AV_CODEC_ID_S302M },
    { MKTAG('D','T','S','1'), AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_DTS },
    { MKTAG('D','T','S','2'), AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_DTS },
    { MKTAG('D','T','S','3'), AVMEDIA_TYPE_AUDIO,   AV_CODEC_ID_DTS },
    { MKTAG('V','C','-','1'), AVMEDIA_TYPE_VIDEO,   AV_CODEC_ID_VC1 },
    { 0 },
};

/* descriptor present */
static const StreamType DESC_types[] = {
    { 0x6a, AVMEDIA_TYPE_AUDIO,             AV_CODEC_ID_AC3 }, /* AC-3 descriptor */
    { 0x7a, AVMEDIA_TYPE_AUDIO,            AV_CODEC_ID_EAC3 }, /* E-AC-3 descriptor */
    { 0x7b, AVMEDIA_TYPE_AUDIO,             AV_CODEC_ID_DTS },
    { 0x56, AVMEDIA_TYPE_SUBTITLE, AV_CODEC_ID_DVB_TELETEXT },
    { 0x59, AVMEDIA_TYPE_SUBTITLE, AV_CODEC_ID_DVB_SUBTITLE }, /* subtitling descriptor */
    { 0 },
};

static void mpegts_find_stream_type(AVStream *st,
                                    uint32_t stream_type, const StreamType *types)
{
    for (; types->stream_type; types++) {
        if (stream_type == types->stream_type) {
            st->codec->codec_type = types->codec_type;
            st->codec->codec_id   = types->codec_id;
            st->request_probe     = 0;
            return;
        }
    }
}

static int mpegts_set_stream_info(AVStream *st, PESContext *pes,
                                  uint32_t stream_type, uint32_t prog_reg_desc)
{
    int old_codec_type= st->codec->codec_type;
    int old_codec_id  = st->codec->codec_id;
    st->priv_data = pes;
    st->codec->codec_type = AVMEDIA_TYPE_DATA;
    st->codec->codec_id   = AV_CODEC_ID_NONE;
    pes->st = st;
    pes->stream_type = stream_type;

    av_log(pes->stream, AV_LOG_DEBUG,
           "stream=%d stream_type=%x pid=%x prog_reg_desc=%.4s\n",
           st->index, pes->stream_type, pes->pid, (char*)&prog_reg_desc);

    st->codec->codec_tag = pes->stream_type;

    mpegts_find_stream_type(st, pes->stream_type, ISO_types);
    if ((prog_reg_desc == AV_RL32("HDMV") ||
         prog_reg_desc == AV_RL32("HDPR")) &&
        st->codec->codec_id == AV_CODEC_ID_NONE) {
        mpegts_find_stream_type(st, pes->stream_type, HDMV_types);
        if (pes->stream_type == 0x83) ez_avlog("PES stream type is 0x83, FIXME\n");
    }
    if (st->codec->codec_id == AV_CODEC_ID_NONE)
        mpegts_find_stream_type(st, pes->stream_type, MISC_types);
    if (st->codec->codec_id == AV_CODEC_ID_NONE){
        st->codec->codec_id  = old_codec_id;
        st->codec->codec_type= old_codec_type;
    }

    return 0;
}

/**
 * Parse MPEG-PES five-byte timestamp
 */
static av_i64 ff_parse_pes_pts(const av_u8 *buf) {
    return (av_i64)(*buf & 0x0e) << 29 |
            (AV_RB16(buf+1) >> 1) << 15 |
             AV_RB16(buf+3) >> 1;
}


static void new_pes_packet(PESContext *pes, AVPacket *pkt)
{

    MpegTSContext *ts = pes->ts;

    if(!pes->buffer){
        ez_avlog("pes buffer is null!\n");
        return ;
    }
    av_init_packet(pkt);

    pkt->destruct = av_destruct_packet;
    pkt->data = pes->buffer;
    pkt->bus_addr = pes->buffer_bus;
    pkt->size = pes->data_index;

    if(pes->total_size != MAX_PES_PAYLOAD &&
       pes->pes_header_size + pes->data_index != pes->total_size + PES_START_SIZE) {
        av_dlog(NULL, "PES packet size mismatch, totalsize=%d, data_index=%d\n", pes->total_size, pes->data_index);
    }
    memset(pkt->data+pkt->size, 0, FF_INPUT_BUFFER_PADDING_SIZE);

    // Separate out the AC3 substream from an HDMV combined TrueHD/AC3 PID
    if (pes->sub_st && pes->stream_type == 0x83 && pes->extended_stream_id == 0x76)
        pkt->stream_index = pes->sub_st->index;
    else
        pkt->stream_index = pes->st->index;

    if(ts->start_pts < 0){
        if(pes->dts >= 0){
            ts->start_pts = pes->dts;
        }else if(pes->pts >= 0){
            ts->start_pts = pes->pts;
        }
        av_dlog(0, "start pts = %lld\n", ts->start_pts);
    }
    
    pkt->pts = (pes->pts - ts->start_pts)/90;
    pkt->dts = AV_NOPTS_VALUE;
    /* store position of first TS packet of this PES packet */
    pkt->pos = pes->ts_packet_pos;
    pkt->flags = pes->flags;

    /* reset pts values */
    pes->pts = AV_NOPTS_VALUE;
    pes->dts = AV_NOPTS_VALUE;
    pes->buffer = NULL;
    pes->buffer_bus = 0;
    pes->data_index = 0;
    pes->flags = 0;
}


/* return non zero if a packet could be constructed */
static int mpegts_push_data(MpegTSFilter *filter,
                            const uint8_t *buf, int buf_size, int is_start,
                            int64_t pos)
{
    PESContext *pes = filter->u.pes_filter.opaque;
    MpegTSContext *ts = pes->ts;
    const uint8_t *p;
    int len, code;

    if(!ts->pkt)
        return 0;

    if (is_start) {
        if (pes->state == MPEGTS_PAYLOAD && pes->data_index > 0) {
            new_pes_packet(pes, ts->pkt);
            ts->stop_parse = 1;
        }
        pes->state = MPEGTS_HEADER;
        pes->data_index = 0;
        pes->ts_packet_pos = pos;
    }
    p = buf;
    while (buf_size > 0) {
        switch(pes->state) {
        case MPEGTS_HEADER:
            len = PES_START_SIZE - pes->data_index;
            if (len > buf_size)
                len = buf_size;
            memcpy(pes->header + pes->data_index, p, len);
            pes->data_index += len;
            p += len;
            buf_size -= len;
            if (pes->data_index == PES_START_SIZE) {
                /* we got all the PES or section header. We can now
                   decide */
                if (pes->header[0] == 0x00 && pes->header[1] == 0x00 &&
                    pes->header[2] == 0x01) {
                    /* it must be an mpeg2 PES stream */
                    code = pes->header[3] | 0x100;
                    av_dlog(pes->stream, "pid=%x pes_code=%#x\n", pes->pid, code);

                    if(code == 0x1be) /* padding_stream */
                        goto skip;
                    /* stream not present in PMT */
                    if (!pes->st) {
                        pes->st = (AVStream *)avformat_new_stream(ts->stream, pes->pid);
                        if (!pes->st)
                            return AVERROR(ENOMEM);
                        mpegts_set_stream_info(pes->st, pes, 0, 0);
                    }

                    pes->total_size = AV_RB16(pes->header + 4);
                    /* NOTE: a zero total size means the PES size is
                       unbounded */
                    if (!pes->total_size)
                        pes->total_size = MAX_PES_PAYLOAD;

                    /* allocate pes buffer */
                    if(pes->buffer){
                        ez_avlog("pes buffer is not null, FIXME\n");
                    }
                    pes->buffer = packet_malloc(pes->total_size+FF_INPUT_BUFFER_PADDING_SIZE, &pes->buffer_bus);
                    if (!pes->buffer)
                        return AVERROR(ENOMEM);

                    if (code != 0x1bc && code != 0x1bf && /* program_stream_map, private_stream_2 */
                        code != 0x1f0 && code != 0x1f1 && /* ECM, EMM */
                        code != 0x1ff && code != 0x1f2 && /* program_stream_directory, DSMCC_stream */
                        code != 0x1f8) {                  /* ITU-T Rec. H.222.1 type E stream */
                        pes->state = MPEGTS_PESHEADER;
                        if (pes->st->codec->codec_id == AV_CODEC_ID_NONE && !pes->st->request_probe) {
                            av_dlog(pes->stream, "pid=%x stream_type=%x probing\n",
                                    pes->pid, pes->stream_type);
                            pes->st->request_probe= 1;
                        }
                    } else {
                        pes->state = MPEGTS_PAYLOAD;
                        pes->data_index = 0;
                    }
                } else {
                    /* otherwise, it should be a table */
                    /* skip packet */
                skip:
                    pes->state = MPEGTS_SKIP;
                    continue;
                }
            }
            break;
            /**********************************************/
            /* PES packing parsing */
        case MPEGTS_PESHEADER:
            len = PES_HEADER_SIZE - pes->data_index;
            if (len < 0)
                return -1;
            if (len > buf_size)
                len = buf_size;
            memcpy(pes->header + pes->data_index, p, len);
            pes->data_index += len;
            p += len;
            buf_size -= len;
            if (pes->data_index == PES_HEADER_SIZE) {
                pes->pes_header_size = pes->header[8] + 9;
                pes->state = MPEGTS_PESHEADER_FILL;
            }
            break;
        case MPEGTS_PESHEADER_FILL:
            len = pes->pes_header_size - pes->data_index;
            if (len < 0)
                return -1;
            if (len > buf_size)
                len = buf_size;
            memcpy(pes->header + pes->data_index, p, len);
            pes->data_index += len;
            p += len;
            buf_size -= len;
            if (pes->data_index == pes->pes_header_size) {
                const uint8_t *r;
                unsigned int flags, pes_ext, skip;

                flags = pes->header[7];
                r = pes->header + 9;
                pes->pts = AV_NOPTS_VALUE;
                pes->dts = AV_NOPTS_VALUE;
                if ((flags & 0xc0) == 0x80) {
                    pes->dts = pes->pts = ff_parse_pes_pts(r);
                    r += 5;
                } else if ((flags & 0xc0) == 0xc0) {
                    pes->pts = ff_parse_pes_pts(r);
                    r += 5;
                    pes->dts = ff_parse_pes_pts(r);
                    r += 5;
                }
                pes->extended_stream_id = -1;
                if (flags & 0x01) { /* PES extension */
                    pes_ext = *r++;
                    /* Skip PES private data, program packet sequence counter and P-STD buffer */
                    skip = (pes_ext >> 4) & 0xb;
                    skip += skip & 0x9;
                    r += skip;
                    if ((pes_ext & 0x41) == 0x01 &&
                        (r + 2) <= (pes->header + pes->pes_header_size)) {
                        /* PES extension 2 */
                        if ((r[0] & 0x7f) > 0 && (r[1] & 0x80) == 0)
                            pes->extended_stream_id = r[1];
                    }
                }

                /* we got the full header. We parse it and get the payload */
                pes->state = MPEGTS_PAYLOAD;
                pes->data_index = 0;
                if (pes->stream_type == 0x12 && buf_size > 0) {
                    ez_avlog("stream type =0x12, FIXME\n");
                }
            }
            break;
        case MPEGTS_PAYLOAD:
            if (buf_size > 0 && pes->buffer) {
                if (pes->data_index > 0 && pes->data_index+buf_size > pes->total_size) {
                    new_pes_packet(pes, ts->pkt);
                    pes->total_size = MAX_PES_PAYLOAD;
                    pes->buffer = packet_malloc(pes->total_size+FF_INPUT_BUFFER_PADDING_SIZE, &pes->buffer_bus);
                    if (!pes->buffer)
                        return AVERROR(ENOMEM);
                    ts->stop_parse = 1;
                } else if (pes->data_index == 0 && buf_size > pes->total_size) {
                    // pes packet size is < ts size packet and pes data is padded with 0xff
                    // not sure if this is legal in ts but see issue #2392
                    buf_size = pes->total_size;
                }
                memcpy(pes->buffer+pes->data_index, p, buf_size);
                pes->data_index += buf_size;
            }
            buf_size = 0;
            /* emit complete packets with known packet size
             * decreases demuxer delay for infrequent packets like subtitles from
             * a couple of seconds to milliseconds for properly muxed files.
             * total_size is the number of bytes following pes_packet_length
             * in the pes header, i.e. not counting the first PES_START_SIZE bytes */
            if (!ts->stop_parse && pes->total_size < MAX_PES_PAYLOAD &&
                pes->pes_header_size + pes->data_index == pes->total_size + PES_START_SIZE) {
                ts->stop_parse = 1;
                new_pes_packet(pes, ts->pkt);
            }
            break;
        case MPEGTS_SKIP:
            buf_size = 0;
            break;
        }
    }

    return 0;
}

static PESContext *add_pes_stream(MpegTSContext *ts, int pid, int pcr_pid)
{
    MpegTSFilter *tss;
    PESContext *pes;

    /* if no pid found, then add a pid context */
    pes = av_mallocz(sizeof(PESContext));
    if (!pes)
        return 0;
    pes->ts = ts;
    pes->stream = ts->stream;
    pes->pid = pid;
    pes->pcr_pid = pcr_pid;
    pes->state = MPEGTS_SKIP;
    pes->pts = AV_NOPTS_VALUE;
    pes->dts = AV_NOPTS_VALUE;
    tss = mpegts_open_pes_filter(ts, pid, mpegts_push_data, pes);
    if (!tss) {
        av_free(pes);
        return 0;
    }
    return pes;
}

static void pmt_cb(MpegTSFilter *filter, const uint8_t *section, int section_len)
{
    MpegTSContext *ts = filter->u.section_filter.opaque;
    SectionHeader h1, *h = &h1;
    PESContext *pes;
    AVStream *st;
    const uint8_t *p, *p_end, *desc_list_end;
    int program_info_length, pcr_pid, pid, stream_type;
    int desc_list_len;
    uint32_t prog_reg_desc = 0; /* registration descriptor */

    int i;
    
    p_end = section + section_len - 4;
    p = section;
    if (parse_section_header(h, &p, p_end) < 0)
        return;

    av_dlog(ts->stream, "sid=0x%x sec_num=%d/%d\n",
           h->id, h->sec_num, h->last_sec_num);

    if (h->tid != PMT_TID)
        return;

    pcr_pid = get16(&p, p_end);
    if (pcr_pid < 0)
        return;
    pcr_pid &= 0x1fff;

    av_dlog(ts->stream, "pcr_pid=0x%x\n", pcr_pid);

    program_info_length = get16(&p, p_end)& 0xfff;
    if (program_info_length < 0)
        return;
    p += program_info_length;
    if (p >= p_end)
        goto out;
    // stop parsing after pmt, we found header
    if (!ts->stream->nb_streams)
        ts->stop_parse = 2;

    for(;;) {
        st = 0;
        pes = NULL;
        stream_type = get8(&p, p_end);
        if (stream_type < 0)
            break;
        pid = get16(&p, p_end);
        if (pid < 0)
            break;
        pid &= 0x1fff;
        /* now create stream */
        if (ts->pids[pid] && ts->pids[pid]->type == MPEGTS_PES) {
            pes = ts->pids[pid]->u.pes_filter.opaque;
            if (!pes->st) {
                pes->st = (AVStream *)avformat_new_stream(pes->stream, pid);
            }
            st = pes->st;
        } else if (stream_type != 0x13) {
            if (ts->pids[pid]) mpegts_close_filter(ts, ts->pids[pid]); //wrongly added sdt filter probably
            pes = add_pes_stream(ts, pid, pcr_pid);
            if (pes) {
                st = (AVStream *)avformat_new_stream(pes->stream, pid);
            }
        } else {
            int idx = ff_find_stream_index(ts->stream, pid);
            if (idx >= 0) {
                st = ts->stream->streams[idx];
            } else {
                st = (AVStream *)avformat_new_stream(ts->stream, pid);
                st->codec->codec_type = AVMEDIA_TYPE_DATA;
            }
        }

        if (!st)
            goto out;

        st->pmt_id = filter->pid;
        if (pes && !pes->stream_type)
            mpegts_set_stream_info(st, pes, stream_type, prog_reg_desc);

        desc_list_len = get16(&p, p_end);
        if (desc_list_len < 0)
            break;
        desc_list_len &= 0xfff;
        desc_list_end = p + desc_list_len;
        if (desc_list_end > p_end)
            break;
        p = desc_list_end;
    }

 out:
    return ;
}

static void pat_cb(MpegTSFilter *filter, const uint8_t *section, int section_len)
{
    MpegTSContext *ts = filter->u.section_filter.opaque;
    SectionHeader h1, *h = &h1;
    const uint8_t *p, *p_end;
    int sid, pmt_pid;

    av_dlog(ts->stream, "PAT:\n");

    p_end = section + section_len - 4;
    p = section;
    if (parse_section_header(h, &p, p_end) < 0)
        return;
    if (h->tid != PAT_TID)
        return;

    for(;;) {
        sid = get16(&p, p_end);
        if (sid < 0)
            break;
        pmt_pid = get16(&p, p_end);
        if (pmt_pid < 0)
            break;
        pmt_pid &= 0x1fff;

        av_dlog(ts->stream, "sid=0x%x pid=0x%x\n", sid, pmt_pid);

        if (sid == 0x0000) {
            /* NIT info */
        } else {
            if (ts->pids[pmt_pid])
                mpegts_close_filter(ts, ts->pids[pmt_pid]);
            mpegts_open_section_filter(ts, pmt_pid, pmt_cb, ts, 1);
        }
    }
}



/* handle one TS packet */
static int handle_packet(MpegTSContext *ts, const uint8_t *packet)
{
    AVFormatContext *s = ts->stream;
    MpegTSFilter *tss;
    int len, pid, cc, expected_cc, cc_ok, afc, is_start, is_discontinuity,
        has_adaptation, has_payload;
    const uint8_t *p, *p_end;
    int64_t pos;

    pid = AV_RB16(packet + 1) & 0x1fff;

    is_start = packet[1] & 0x40;
    tss = ts->pids[pid];
    if (ts->auto_guess && tss == NULL && is_start) {
        add_pes_stream(ts, pid, -1);
        tss = ts->pids[pid];
    }
    if (!tss)
        return 0;

    afc = (packet[3] >> 4) & 3;
    if (afc == 0) /* reserved value */
        return 0;
    has_adaptation = afc & 2;
    has_payload = afc & 1;
    is_discontinuity = has_adaptation
                && packet[4] != 0 /* with length > 0 */
                && (packet[5] & 0x80); /* and discontinuity indicated */

    /* continuity check (currently not used) */
    cc = (packet[3] & 0xf);
    expected_cc = has_payload ? (tss->last_cc + 1) & 0x0f : tss->last_cc;
    cc_ok = pid == 0x1FFF // null packet PID
            || is_discontinuity
            || tss->last_cc < 0
            || expected_cc == cc;

    tss->last_cc = cc;
    if (!cc_ok) {
        av_log(ts->stream, AV_LOG_DEBUG,
               "Continuity check failed for pid %d expected %d got %d\n",
               pid, expected_cc, cc);
    }

    if (!has_payload)
        return 0;
    p = packet + 4;
    if (has_adaptation) {
        /* skip adaptation field */
        p += p[0] + 1;
    }
    /* if past the end of packet, ignore */
    p_end = packet + TS_PACKET_SIZE;
    if (p >= p_end)
        return 0;

    pos = avio_tell(ts->stream->pb);
    ts->pos47= pos % ts->raw_packet_size;

    if (tss->type == MPEGTS_SECTION) {
        if (is_start) {
            /* pointer field present */
            len = *p++;
            if (p + len > p_end)
                return 0;
            if (len && cc_ok) {
                /* write remaining section bytes */
                write_section_data(s, tss,
                                   p, len, 0);
                /* check whether filter has been closed */
                if (!ts->pids[pid])
                    return 0;
            }
            p += len;
            if (p < p_end) {
                write_section_data(s, tss,
                                   p, p_end - p, 1);
            }
        } else {
            if (cc_ok) {
                write_section_data(s, tss,
                                   p, p_end - p, 0);
            }
        }
    } else {
        int ret;
        // Note: The position here points actually behind the current packet.
        if ((ret = tss->u.pes_filter.pes_cb(tss, p, p_end - p, is_start,
                                            pos - ts->raw_packet_size)) < 0)
            return ret;
    }

    return 0;
}

/* XXX: try to find a better synchro over several packets (use
   get_packet_size() ?) */
static int mpegts_resync(AVFormatContext *s)
{
    ByteIOContext *pb = s->pb;
    int c, i;

    for(i = 0;i < MAX_RESYNC_SIZE; i++) {
        c = avio_r8(pb);
        if (avio_eof(pb))
            return -1;
        if (c == 0x47) {
            avio_seek(pb, -1, SEEK_CUR);
            return 0;
        }
    }
    av_log(s, AV_LOG_ERROR, "max resync size reached, could not find sync byte\n");
    /* no sync found */
    return -1;
}

/* return -1 if error or EOF. Return 0 if OK. */
static int read_packet(AVFormatContext *s, uint8_t **buf, int raw_packet_size)
{
    ByteIOContext *pb = s->pb;
    int skip, len;
    uint8_t *p;

    for(;;) {
        len = avio_read_ptr(pb, buf, TS_PACKET_SIZE);
        if (len != TS_PACKET_SIZE)
            return len < 0 ? len : AVERROR_EOF;
        /* check packet sync byte */
        p = *buf;
        if (p[0] != 0x47) {
            /* find a new packet start */
            avio_seek(pb, -TS_PACKET_SIZE, SEEK_CUR);
            if (mpegts_resync(s) < 0)
                return AVERROR(EAGAIN);
            else
                continue;
        } else {
            skip = raw_packet_size - TS_PACKET_SIZE;
            if (skip > 0)
                avio_skip(pb, skip);
            break;
        }
    }
    return 0;
}

static int handle_packets(MpegTSContext *ts, const int nb_packets)
{
    AVFormatContext *s = ts->stream;
    int packet_num, ret = 0;
    uint8_t *packet = NULL;

    if (avio_tell(s->pb) != ts->last_pos) {
        int i;
        av_dlog(ts->stream, "Skipping after seek\n");
        /* seek detected, flush pes buffer */
        for (i = 0; i < NB_PID_MAX; i++) {
            if (ts->pids[i]) {
                if (ts->pids[i]->type == MPEGTS_PES) {
                   PESContext *pes = ts->pids[i]->u.pes_filter.opaque;
                   packet_freep(&pes->buffer);
                   pes->data_index = 0;
                   pes->state = MPEGTS_SKIP; /* skip until pes header */
                }
                ts->pids[i]->last_cc = -1;
            }
        }
    }

    ts->stop_parse = 0;
    packet_num = 0;
    for(;;) {
        packet_num++;
        if ((nb_packets > 0 && packet_num >= nb_packets) ||
            ts->stop_parse > 1) {
            ret = AVERROR(EAGAIN);
            break;
        }
        if (ts->stop_parse > 0)
            break;

        ret = read_packet(s, &packet, ts->raw_packet_size);
        if (ret != 0)
            break;
        ret = handle_packet(ts, packet);
        if (ret != 0)
            break;
    }
    ts->last_pos = avio_tell(s->pb);
    return ret;
}

int mpegts_read_header(AVFormatContext **av, char * const name, av_file_iocontext_s *fio)
{
    MpegTSContext *ts;
    ByteIOContext *pb;
    AVFormatContext * s = NULL;
    uint8_t *buf = NULL;
    int ret = -1;
    int len;
    int64_t pos;

#define TS_PROBE_SIZE 0x7d00
    *av = s 
        = av_mallocz(sizeof(AVFormatContext));
    if(*av == NULL){
        goto fail;
    }
    s->audio_stream_index = -1;
    s->video_stream_index = -1;
    s->probesize = TS_PROBE_SIZE*TS_PACKET_SIZE;
    s->priv_data = ts 
        = av_mallocz(sizeof(MpegTSContext));

    if(ts == NULL){
        goto fail;
    }
    ts->start_pts = AV_NOPTS_VALUE;
    ret = avio_open(&s->pb, name, fio);
    if(ret < 0){
        goto fail;
    }
    pb = s->pb;

    /* read the first 8192 bytes to get packet size */
    pos = avio_tell(pb);
    len = avio_read_ptr(pb, &buf, 8*1024);
    ts->raw_packet_size = get_packet_size(buf, len);
    if (ts->raw_packet_size <= 0) {
        ez_avlog("Could not detect TS packet size,  FIXME\n");
        ret = AVERROR_NOFMT;
        goto fail;
    }
    ts->stream = s;
    ts->auto_guess = 0;

    /* normal demux */

    /* first do a scan to get all the services */
    /* NOTE: We attempt to seek on non-seekable files as well, as the
     * probe buffer usually is big enough. Only warn if the seek failed
     * on files where the seek should work. */
    if (avio_seek(pb, pos, SEEK_SET) < 0)
        av_log(s, pb->seekable ? AV_LOG_ERROR : AV_LOG_INFO, "Unable to seek back to the start\n");

    mpegts_open_section_filter(ts, PAT_PID, pat_cb, ts, 1);
    handle_packets(ts, s->probesize / ts->raw_packet_size);
    /* if could not find service, enable auto_guess */

    ts->auto_guess = 1;

    avio_seek(pb, pos, SEEK_SET);
    ts->last_pos = pos;
    av_dlog(ts->stream, "tuning done\n");
    
    return 0;
    
 fail:
    if(s){
        if(s->pb){
            avio_close(s->pb);
            s->pb = NULL;
        }

        if(ts){
            av_free(ts);
        }

        av_free(s);
    }
    
    return ret;
}

int mpegts_read_packet(AVFormatContext *s,
                              AVPacket *pkt)
{
    MpegTSContext *ts = s->priv_data;
    int ret, i;

    pkt->size = -1;
    ts->pkt = pkt;
    ret = handle_packets(ts, 0);
    if (ret < 0) {
        av_free_packet(ts->pkt);
        /* flush pes data left */
        for (i = 0; i < NB_PID_MAX; i++) {
            if (ts->pids[i] && ts->pids[i]->type == MPEGTS_PES) {
                PESContext *pes = ts->pids[i]->u.pes_filter.opaque;
                if (pes->state == MPEGTS_PAYLOAD && pes->data_index > 0) {
                    new_pes_packet(pes, pkt);
                    pes->state = MPEGTS_SKIP;
                    ret = 0;
                    break;
                }
            }
        }
    }

    if (!ret && pkt->size < 0)
        ret = AVERROR(EINTR);
    return ret;
}

int mpegts_read_close(AVFormatContext *s)
{
    MpegTSContext *ts;
    int i;
    
    if(s){
        ts = s->priv_data;
        for(i=0;i<NB_PID_MAX;i++)
            if (ts->pids[i]) mpegts_close_filter(ts, ts->pids[i]);
        
        if(s->pb){
            avio_close(s->pb);
            s->pb = NULL;
        }

        if(ts){
            av_free(ts);
        }

        for(i=0; i<s->nb_streams; i++){
            av_free(s->streams[i]->codec);
            av_free(s->streams[i]);
        }
        
        av_free(s);
    }
    

    return 0;
}

