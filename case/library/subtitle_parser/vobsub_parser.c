#include "vobsub_parser.h"
#include "vobsub_spudec.h"
#include "subtitle_common.h"

int vobsub_id=0;
static int file_idx=0;
//#define  VOBSUB_PRINT_PACK 0
///function definition////

#define DUMP_BUFFER_DEST_DIR "/mnt/udisk"

static FILE *test_fp=NULL;

static char test_open(const char* filename)
{	
#ifdef VOBSUB_PRINT_PACK
	test_fp = fopen(filename,"wb");
	if(test_fp==NULL){
		printf("test txt open failed!\n");
	}
	return 0;
#else 
	return 0;
#endif
}

static int test_write(unsigned char *buf,unsigned int size)
{
#ifdef VOBSUB_PRINT_PACK
	if(test_fp!=NULL)
		return fwrite(buf,1,size,test_fp);
	else
		return 0;
#else
	return 0;
#endif
}

static char test_close()
{
#ifdef VOBSUB_PRINT_PACK
	if(test_fp!=NULL){
		fclose(test_fp);
		test_fp = NULL;
	}
	return 0;
#else
	return 0;
#endif
}

/**********************************************************************
 * Packet queue
 **********************************************************************/


static void packet_queue_construct(packet_queue_t *queue)
{
    queue->id = NULL;
    queue->packets = NULL;
    queue->packets_reserve = 0;
    queue->packets_size = 0;
    queue->current_index = 0;
}

static void packet_construct(raw_packet_t *pkt)
{
    pkt->pts100 = 0;
	pkt->ptsend = 0;
    pkt->filepos = 0;
    pkt->size = 0;
    pkt->data = NULL;
}


/* Make sure there is enough room for needed_size packets in the
packet queue. */
static int packet_queue_ensure(packet_queue_t *queue, unsigned int needed_size,parser_info_t *parser_info)
{
    if (queue->packets_reserve < (int)needed_size) {
		if (queue->packets) {
			raw_packet_t *tmp = parser_info->fops.realloc(queue->packets, 2 * queue->packets_reserve * sizeof(raw_packet_t));
			if (tmp == NULL) {
				printf("realloc failure");
				return -1;
			}
			queue->packets = tmp;
			queue->packets_reserve *= 2;
		}
		else {
			queue->packets = parser_info->fops.malloc(sizeof(raw_packet_t));
			if (queue->packets == NULL) {
				printf("malloc failure");
				return -1;
			}
			queue->packets_reserve = 1;
		}
    }
    return 0;
}


/* add one more packet */
static int packet_queue_grow(packet_queue_t *queue,parser_info_t *parser_info)
{
    if (packet_queue_ensure(queue, queue->packets_size + 1,parser_info) < 0) 
		return -1;
    packet_construct(queue->packets + queue->packets_size);
    ++queue->packets_size;
    return 0;
}


/* insert a new packet, duplicating pts from the current one */
static int packet_queue_insert(packet_queue_t *queue,parser_info_t *parser_info)
{
	raw_packet_t *pkts;
	if (packet_queue_ensure(queue, queue->packets_size + 1,parser_info) < 0)
		return -1;
	/* XXX packet_size does not reflect the real thing here, it will be updated a bit later */
	memmove(queue->packets + queue->current_index + 2,
		queue->packets + queue->current_index + 1,
		sizeof(raw_packet_t) * (queue->packets_size - queue->current_index - 1));
	pkts = queue->packets + queue->current_index;
	++queue->packets_size;
	++queue->current_index;
	packet_construct(pkts + 1);
	pkts[1].pts100 = pkts[0].pts100;
	pkts[1].ptsend = pkts[0].ptsend;
	pkts[1].filepos = pkts[0].filepos;
	return 0;
}

static void packet_destroy(raw_packet_t *pkt,parser_info_t *parser_info)
{
    if (pkt->data)
		parser_info->fops.free(pkt->data);
}


static void packet_queue_destroy(packet_queue_t *queue,parser_info_t *parser_info)
{
    if (queue->packets) {
		while (queue->packets_size--)
			packet_destroy(queue->packets + queue->packets_size,parser_info);
		parser_info->fops.free(queue->packets);
    }
    return;
}
///////////////////////////////////////////////////////////


static int vobsub_ensure_spu_stream(vobsub_t *vob, unsigned int index,parser_info_t *parser_info);
static int vobsub_add_id(vobsub_t *vob, const char *id, size_t idlen, const unsigned int index,parser_info_t *parser_info);

/* Make sure that the spu stream idx exists. */
//为新的数据流分配节点，并初始化新的节点
//节点为一个数组，数组头存在spu_streams中，大小存放在spu_streams_size
static int vobsub_ensure_spu_stream(vobsub_t *vob, unsigned int index,parser_info_t *parser_info)
{
    if (index >= vob->spu_streams_size) {
		/* This is a new stream */
		if (vob->spu_streams) {
			packet_queue_t *tmp = parser_info->fops.realloc(vob->spu_streams, (index + 1) * sizeof(packet_queue_t));
			if (tmp == NULL) {
				printf("vobsub_ensure_spu_stream: realloc failure");
				return -1;
			}
			vob->spu_streams = tmp;
		}
		else {
			vob->spu_streams = parser_info->fops.malloc((index + 1) * sizeof(packet_queue_t));
			if (vob->spu_streams == NULL) {
				printf("vobsub_ensure_spu_stream: malloc failure");
				return -1;
			}
		}
		while (vob->spu_streams_size <= index) {
			packet_queue_construct(vob->spu_streams + vob->spu_streams_size);
			++vob->spu_streams_size;
		}
    }
    return 0;
}

//将语言节点信息保存起来，
static int vobsub_add_id(vobsub_t *vob, const char *id, size_t idlen, const unsigned int index,parser_info_t *parser_info)
{
    if (vobsub_ensure_spu_stream(vob, index,parser_info) < 0)
		return -1;
    if (id && idlen) {
		if (vob->spu_streams[index].id)
			parser_info->fops.free(vob->spu_streams[index].id);
		vob->spu_streams[index].id = parser_info->fops.malloc(idlen + 1);
		if (vob->spu_streams[index].id == NULL) {
			printf("vobsub_add_id: malloc failure");
			return -1;
		}
		vob->spu_streams[index].id[idlen] = 0;
		memcpy(vob->spu_streams[index].id, id, idlen);
    }
    vob->spu_streams_current = index;
    printf("ID_VOBSUB_ID=%d\n", index);
    if (id && idlen)
		printf("ID_VSID_%d_LANG=%s\n", index, vob->spu_streams[index].id);
    printf("[vobsub] subtitle (vobsubid): %d language %s\n",index, vob->spu_streams[index].id);
    return 0;
}



static int vobsub_add_timestamp(vobsub_t *vob, long filepos, int ms,parser_info_t *parser_info)
{
    packet_queue_t *queue;
    raw_packet_t *pkt;
	raw_packet_t *pkt_prev;
    if (vob->spu_streams == 0) {
		printf("[vobsub] warning, binning some index entries.  Check your index file\n");
		return -1;
    }
    queue = vob->spu_streams + vob->spu_streams_current;
    if (packet_queue_grow(queue,parser_info) >= 0) {
		pkt = queue->packets + (queue->packets_size - 1);
		pkt->filepos = filepos;
		pkt->pts100 = ms < 0 ? 0xffffffff : (unsigned int)ms*90;
		pkt->ptsend = pkt->pts100+90000;//assume the end of the current line is 1 second later	
		if(queue->packets_size>1){
			pkt_prev =  queue->packets + (queue->packets_size - 2);
			pkt_prev->ptsend = pkt->pts100-1;//the real ending time of the prev line 
		}	
		return 0;
    }
    return -1;
}

///////////////////////////used for parser idx file///////////////////////////////////////
static int vobsub_set_lang(const char *line)
{
    if (vobsub_id == -1)
        vobsub_id = atoi(line + 8);
    return 0;
}

static int vobsub_parse_delay(vobsub_t *vob, const char *line)
{
    int h, m, s, ms;
    int forward = 1;
    if (*(line + 7) == '+'){
		forward = 1;
		line++;
    }
    else if (*(line + 7) == '-'){
		forward = -1;
		line++;
    }
    printf("forward=%d", forward);
    h = atoi(line + 7);
    printf("h=%d," ,h);
    m = atoi(line + 10);
    printf("m=%d,", m);
    s = atoi(line + 13);
    printf("s=%d,", s);
    ms = atoi(line + 16);
    printf("ms=%d", ms);
    vob->delay = (ms + 1000 * (s + 60 * (m + 60 * h))) * forward;
    return 0;
}

static int vobsub_parse_id(vobsub_t *vob, const char *line,parser_info_t *parser_info)
{
    // id: xx, index: n
    size_t idlen;
    const char *p, *q;
    p  = line;
    while (isspace(*p))
		++p;
    q = p;
    while (isalpha(*q)||((*q)=='-'))
		++q;
    idlen = q - p;
    if (idlen == 0)
	return -1;
    ++q;
    while (isspace(*q))
		++q;
    if (strncmp("index:", q, 6))
		return -1;
    q += 6;
    while (isspace(*q))
		++q;
    if (!isdigit(*q))
		return -1;
    return vobsub_add_id(vob, p, idlen, atoi(q),parser_info);
}


static int vobsub_parse_palette(vobsub_t *vob, const char *line)
{
    // palette: XXXXXX, XXXXXX, XXXXXX, XXXXXX, XXXXXX, XXXXXX, XXXXXX, XXXXXX, XXXXXX, XXXXXX, XXXXXX, XXXXXX, XXXXXX, XXXXXX, XXXXXX, XXXXXX
    unsigned int n;
    n = 0;
    while (1) {
		const char *p;
		int r, g, b, y, u, v, tmp;
		while (isspace(*line))
			++line;
		p = line;
		while (isxdigit(*p))
			++p;
		if (p - line != 6)
			return -1;
		tmp = strtoul(line, NULL, 16);
		r = tmp >> 16 & 0xff;
		g = tmp >> 8 & 0xff;
		b = tmp & 0xff;
		y = (int)av_clip_uint8( 0.1494  * r + 0.6061 * g + 0.2445 * b);
		u = (int)av_clip_uint8( 0.6066  * r - 0.4322 * g - 0.1744 * b + 128);
		v = (int)av_clip_uint8(-0.08435 * r - 0.3422 * g + 0.4266 * b + 128);
		vob->palette[n++] = y << 16 | u << 8 | v;
		if (n == 16)
			break;
		if (*p == ',')
			++p;
		line = p;
    }
    vob->have_palette = 1;
    return 0;
}


static int vobsub_parse_size(vobsub_t *vob, const char *line)
{
    // size: WWWxHHH
    char *p;
    while (isspace(*line))
		++line;
    if (!isdigit(*line))
		return -1;
    vob->orig_frame_width = strtoul(line, &p, 10);
    if (*p != 'x')
		return -1;
    ++p;
    vob->orig_frame_height = strtoul(p, NULL, 10);
    return 0;
}


static int vobsub_parse_origin(vobsub_t *vob, const char *line)
{
    // org: X,Y
    char *p;
    while (isspace(*line))
		++line;
    if (!isdigit(*line))
		return -1;
    vob->origin_x = strtoul(line, &p, 10);
    if (*p != ',')
		return -1;
    ++p;
    vob->origin_y = strtoul(p, NULL, 10);
    return 0;
}


static int vobsub_parse_timestamp(vobsub_t *vob, const char *line,parser_info_t *parser_info)
{
    // timestamp: HH:MM:SS.mmm, filepos: 0nnnnnnnnn
    const char *p;
    int h, m, s, ms;
    long filepos;
    while (isspace(*line))
		++line;
    p = line;
    while (isdigit(*p))
		++p;
    if (p - line != 2)
		return -1;
    h = atoi(line);
    if (*p != ':')
		return -1;
    line = ++p;
    while (isdigit(*p))
		++p;
    if (p - line != 2)
		return -1;
    m = atoi(line);
    if (*p != ':')
		return -1;
    line = ++p;
    while (isdigit(*p))
		++p;
    if (p - line != 2)
		return -1;
    s = atoi(line);
    if (*p != ':')
		return -1;
    line = ++p;
    while (isdigit(*p))
		++p;
    if (p - line != 3)
		return -1;
    ms = atoi(line);
    if (*p != ',')
		return -1;
    line = p + 1;
    while (isspace(*line))
		++line;
    if (strncmp("filepos:", line, 8))
		return -1;
    line += 8;
    while (isspace(*line))
		++line;
    if (! isxdigit(*line))
		return -1;
    filepos = strtol(line, NULL, 16);
    return vobsub_add_timestamp(vob, filepos, vob->delay + ms + 1000 * (s + 60 * (m + 60 * h)),parser_info);
}


static int vobsub_parse_cuspal(vobsub_t *vob, const char *line)
{
    //colors: XXXXXX, XXXXXX, XXXXXX, XXXXXX
    unsigned int n;
    n = 0;
    line += 40;
    while(1){
		const char *p;
		while (isspace(*line))
			++line;
		p=line;
		while (isxdigit(*p))
			++p;
		if (p - line !=6)
			return -1;
		vob->cuspal[n++] = strtoul(line, NULL,16);
		if (n==4)
			break;
		if(*p == ',')
			++p;
		line = p;
    }
    return 0;
}


/* don't know how to use tridx */
static int vobsub_parse_tridx(const char *line)
{
    //tridx: XXXX
    int tridx;
    tridx = strtoul((line + 26), NULL, 16);
    tridx = ((tridx&0x1000)>>12) | ((tridx&0x100)>>7) | ((tridx&0x10)>>2) | ((tridx&1)<<3);
    return tridx;
}

static int vobsub_parse_custom(vobsub_t *vob, const char *line)
{
    //custom colors: OFF/ON(0/1)
    if ((strncmp("ON", line + 15, 2) == 0)||strncmp("1", line + 15, 1) == 0)
        vob->custom=1;
    else if ((strncmp("OFF", line + 15, 3) == 0)||strncmp("0", line + 15, 1) == 0)
        vob->custom=0;
    else
        return -1;
    return 0;
}


static int vobsub_parse_forced_subs(vobsub_t *vob, const char *line)
{
    const char *p;
	
    p  = line;
    while (isspace(*p))
		++p;
	
    if (strncasecmp("on",p,2) == 0){
		vob->forced_subs=~0;
		return 0;
    } else if (strncasecmp("off",p,3) == 0){
		vob->forced_subs=0;
		return 0;
    }
	
    return -1;
}

static mpeg_t * mpeg_open(parser_info_t *parser_info)
{

    mpeg_t *res = parser_info->fops.malloc(sizeof(mpeg_t));
	int err = res == NULL;
    if (!err) {
		res->pts = 0;
		res->aid = -1;
		res->packet = NULL;
		res->packet_size = 0;
		res->packet_reserve = 0;
		res->fp = parser_info->fp1;
		if (res->fp==NULL){
			parser_info->fops.free(res);
			//printf("NULL res@@@@@@@@@@@@@@@@@\n");
			return NULL;
		}
    }
    return err ? NULL : res;
}


static void mpeg_free(mpeg_t *mpeg,parser_info_t *parser_info)
{
    if (mpeg->packet)
		parser_info->fops.free(mpeg->packet);
    if(mpeg)
		parser_info->fops.free(mpeg);
}

static int mpeg_run(mpeg_t *mpeg,parser_info_t *parser_info)
{
	char testfile[30]="";
    unsigned int len, idx, version;
	int readnum=0;
    char c;
    /* Goto start of a packet, it starts with 0x000001?? */
    const unsigned char wanted[] = { 0, 0, 1 };
    unsigned char buf[5];
	int nbyte=0;
    mpeg->aid = -1;
    mpeg->packet_size = 0;
	if((readnum=parser_info->fops.read(mpeg->fp,buf,4))!=4){
		printf("ReadNum=%d,Pos=0x%x\n",readnum,parser_info->fops.tell(mpeg->fp));
		return -1;
	}
    while (memcmp(buf, wanted, sizeof(wanted)) != 0) {
		nbyte = get_char(parser_info,mpeg->fp,&c);
		if (nbyte == 0)
			return -1;
		memmove(buf, buf + 1, 3);
		buf[3] = c;
    }
    switch (buf[3]) {
		case 0xB9:			/* System End Code */
			break;
		case 0xBA:			/* Packet start code */
			nbyte = get_char(parser_info,mpeg->fp,&c);
			if (nbyte== 0)
				return -1;
			if ((c & 0xc0) == 0x40)
				version = 4;
			else if ((c & 0xf0) == 0x20)
				version = 2;
			else {
				printf("VobSub: Unsupported MPEG version: 0x%02x\n", c);
				return -1;
			}
			if (version == 4) {
				if (parser_info->fops.seek_cur(mpeg->fp, 9))
					return -1;
			}
			else if (version == 2) {
				if (parser_info->fops.seek_cur(mpeg->fp, 7))
					return -1;
			}
			else{
				printf("%s,%d:Error Version Type\n",__FILE__,__LINE__);
				return -1;
			}
			break;
		case 0xBD:			/* packet */
			if (parser_info->fops.read(mpeg->fp,buf,2) != 2)
				return -1;
			len = buf[0] << 8 | buf[1];
			idx = parser_info->fops.tell(mpeg->fp);
			get_char(parser_info,mpeg->fp,&c);
			if ((c & 0xC0) == 0x40) { /* skip STD scale & size */
				if (get_char(parser_info,mpeg->fp,&c) ==0)
					return -1;
				nbyte = get_char(parser_info,mpeg->fp,&c);
				if (nbyte == 0)
					return -1;
			}
			if ((c & 0xf0) == 0x20) { /* System-1 stream timestamp */
				/* Do we need this? */
				printf("%s,%d:System-1 stream timestamp,no support yet\n",__FILE__,__LINE__);
				return -1;
			}
			else if ((c & 0xf0) == 0x30) {
				/* Do we need this? */
				printf("%s,%d:Do we need this\n",__FILE__,__LINE__);
				return -1;
			}
			else if ((c & 0xC0) == 0x80) { /* System-2 (.VOB) stream */
				unsigned int pts_flags, hdrlen, dataidx;
				get_char(parser_info,mpeg->fp,&c);
				pts_flags = c;
				get_char(parser_info,mpeg->fp,&c);
				hdrlen = c;
				dataidx = parser_info->fops.tell(mpeg->fp) + hdrlen;
				//printf("pts_flag=0x%x,hdrlen=0x%x,Pos=0x%x,dataidx=0x%x\n",pts_flags,hdrlen,parser_info->fops.tell(mpeg->fp),dataidx);
				if (dataidx > idx + len) {
					printf("Invalid header length: %d (total length: %d, idx: %d, dataidx: %d)\n",
						hdrlen, len, idx, dataidx);
					return -1;
				}
				if ((pts_flags & 0xC0) == 0x80) {
					if(parser_info->fops.read(mpeg->fp,buf,5)!=5)
						return -1;
					if (!(((buf[0] & 0xF0) == 0x20) && (buf[0] & 1) && (buf[2] & 1) &&  (buf[4] & 1))) {
						printf("vobsub PTS error: 0x%02x %02x%02x %02x%02x \n",
							buf[0], buf[1], buf[2], buf[3], buf[4]);
						mpeg->pts = 0;
					}
					else{
						mpeg->pts = ((buf[0] & 0x0e) << 29 | buf[1] << 22 | (buf[2] & 0xfe) << 14
						| buf[3] << 7 | (buf[4] >> 1));
						//printf("vobsub PTS : 0x%02x %02x%02x %02x%02x:pts=0x%x\n",buf[0], buf[1], buf[2], buf[3], buf[4],mpeg->pts);
					}
				}
				else /* if ((pts_flags & 0xc0) == 0xc0) */ {
				/* what's this? */
				/* abort(); */
				}
				parser_info->fops.seek_set(mpeg->fp,dataidx);
				//printf("Tell Pos=0x%x  ",parser_info->fops.tell(mpeg->fp));
				get_char(parser_info,mpeg->fp,&c);
				mpeg->aid = c;
				//printf("Aid=0x%x\n",mpeg->aid);
				if (mpeg->aid < 0) {
					printf("Bogus aid %d\n", mpeg->aid);
					return -1;
				}
				//printf("Tell Pos=0x%x,idx=0x%x,len=0x%x\n",parser_info->fops.tell(mpeg->fp),idx,len);
				mpeg->packet_size = len - ((unsigned int) parser_info->fops.tell(mpeg->fp) - idx);
				if (mpeg->packet_reserve < mpeg->packet_size) {
					if (mpeg->packet)
						parser_info->fops.free(mpeg->packet);
					mpeg->packet = (unsigned char*)parser_info->fops.malloc(mpeg->packet_size);
					if (mpeg->packet)
						mpeg->packet_reserve = mpeg->packet_size;
				}
				if (mpeg->packet == NULL) {
					printf("%s,%d:malloc failure",__FILE__,__LINE__);
					mpeg->packet_reserve = 0;
					mpeg->packet_size = 0;
					return -1;
				}
				if(parser_info->fops.read(mpeg->fp,mpeg->packet,mpeg->packet_size)!=(int)mpeg->packet_size){
					printf("%s,%d:fread failure",__FILE__,__LINE__);
					mpeg->packet_size = 0;
					return -1;
				}
				{

					test_open(testfile);
					test_write(mpeg->packet,mpeg->packet_size);
					test_close();
				}
			
				idx = len;
			}
			break;
		case 0xBE:			/* Padding */
			if(parser_info->fops.read(mpeg->fp,buf,2)!=2)
				return -1;
			len = buf[0] << 8 | buf[1];
			if (len > 0 && parser_info->fops.seek_cur(mpeg->fp,len))
				return -1;
			break;
		default:
			if (0xC0 <= buf[3] && buf[3] < 0xf0) {
				/* MPEG audio or video */
				if(parser_info->fops.read(mpeg->fp,buf,2)!=2)
					return -1;
				len = buf[0] << 8 | buf[1];
				if (len > 0 && parser_info->fops.seek_cur(mpeg->fp,len))
					return -1;
				
			}
			else {
				printf("unknown header 0x%02X%02X%02X%02X\n",
					buf[0], buf[1], buf[2], buf[3]);
				return -1;
			}
    }
    return 0;
}


///////////////////////////////////////////////////
static size_t vobsub_getline(char **lineptr, size_t *n,parser_info_t *parser_info)
{
	size_t res = 0;
	int nbyte=0;
	char c;
	if (*lineptr == NULL) {
		*lineptr = parser_info->fops.malloc(4096);
		if (*lineptr)
			*n = 4096;
	}
	else if (*n == 0) {
		char *tmp = parser_info->fops.realloc(*lineptr, 4096);
		if (tmp) {
			*lineptr = tmp;
			*n = 4096;
		}
	}
	if (*lineptr == NULL || *n == 0)
		return -1;
	nbyte= get_char(parser_info,parser_info->fp,&c);
	for (;nbyte!=0; nbyte =get_char(parser_info,parser_info->fp,&c)) {
		if (res + 1 >= *n) {
			char *tmp = parser_info->fops.realloc(*lineptr, *n * 2);
			if (tmp == NULL)
				return -1;
			*lineptr = tmp;
			*n *= 2;
		}
		(*lineptr)[res++] = c;
		if (c == 0x0A && (*lineptr)[res-2]==0x0D ){
			(*lineptr)[res] = 0;
			return res;
		}
	}
	if (res == 0)
		return -1;
	(*lineptr)[res] = 0;
	return res;
}

int vobsub_init_fp(char *file,parser_info_t *parser_info)
{
	char *namebuf= parser_info->fops.malloc(256);
	char *buf=NULL;
	int filenamelen=0;
	if(namebuf==NULL){
		printf("%s,%d:malloc failed!\n",__FILE__,__LINE__);
		return -1;
	}
	memset(namebuf,0,256);
	filenamelen=get_filename(file,namebuf,256);
	if(filenamelen==-1){
		parser_info->fops.free(namebuf);
		return -1;
	}
	buf = (char*)parser_info->fops.malloc(filenamelen+5);
	memset(buf,0,filenamelen+5);
	if(buf==NULL){
		parser_info->fops.free(namebuf);
		return -1;
	}
	
	strcpy(buf,namebuf);
	strcat(buf,".idx");
	//printf("idx_name====================%s\n",buf);
	parser_info->fp = parser_info->fops.open(buf,"rb");
	if(parser_info->fp==NULL){
		printf("%s,%d:Open Idx File error!\n",__FILE__,__LINE__);
		parser_info->fops.free(buf);
		parser_info->fops.free(namebuf);
		return -1;
	}

	strcpy(buf,namebuf);
	strcat(buf,".sub");
	
	parser_info->fp1 = parser_info->fops.open(buf,"rb");
	if(parser_info->fp1==NULL){
		printf("%s,%d:Open Sub File error!\n",__FILE__,__LINE__);
		parser_info->fops.free(buf);
		parser_info->fops.free(namebuf);
		return -1;
	}
	if(buf)parser_info->fops.free(buf);
	if(namebuf)parser_info->fops.free(namebuf);
	return 0;
}

static int vobsub_parse_one_line(vobsub_t *vob,parser_info_t *parser_info)
{
    int line_size;
    int res = -1;
	size_t line_reserve = 0;
	char *line = NULL;
    do {
		//printf("line_sie@@@@@@@\n");
		line_size = vobsub_getline(&line, &line_reserve,parser_info);
		//printf("line_size===============%d\n",line);
		if (line_size < 0) {
			break;
		}
		if (*line == 0 || *line ==0x0D || (*line==0x0D && *(line+1)==0x0A) || *line == '#')
			continue;
		else if (strncmp("langidx:", line, 8) == 0)
			res = vobsub_set_lang(line);
		else if (strncmp("delay:", line, 6) == 0)
			res = vobsub_parse_delay(vob, line);
		else if (strncmp("id:", line, 3) == 0)
			res = vobsub_parse_id(vob, line + 3,parser_info);
		else if (strncmp("palette:", line, 8) == 0)
			res = vobsub_parse_palette(vob, line + 8);
		else if (strncmp("size:", line, 5) == 0)
			res = vobsub_parse_size(vob, line + 5);
		else if (strncmp("org:", line, 4) == 0)
			res = vobsub_parse_origin(vob, line + 4);
		else if (strncmp("timestamp:", line, 10) == 0)
			res = vobsub_parse_timestamp(vob, line + 10,parser_info);
		else if (strncmp("custom colors:", line, 14) == 0)
			//custom colors: ON/OFF, tridx: XXXX, colors: XXXXXX, XXXXXX, XXXXXX,XXXXXX
			res = vobsub_parse_cuspal(vob, line) + vobsub_parse_tridx(line) + vobsub_parse_custom(vob, line);
		else if (strncmp("forced subs:", line, 12) == 0)
			res = vobsub_parse_forced_subs(vob, line + 12);
		else {
			printf("vobsub: ignoring %s\n", line);
			continue;
		}
		if (res < 0){
			printf("ERROR in %s", line);
			break;
		}
    } while (1);
    if (line)
		free(line);
    return res;
}



void * vobsub_open(const int force,parser_info_t *parser_info)
{
	vobsub_info_t *vobsub_info = (vobsub_info_t *)parser_info->finfo;
	vobsub_t *vob = (vobsub_t*)parser_info->fops.malloc(sizeof(vobsub_t));
	spudec_handle_t ** spu = &vobsub_info->spudec;
	if(spu)
		*spu = NULL;
	if (vob) {
		mpeg_t *mpg;
		vob->custom = 0;
		vob->have_palette = 0;
		vob->orig_frame_width = 0;
		vob->orig_frame_height = 0;
		vob->spu_streams = NULL;
		vob->spu_streams_size = 0;
		vob->spu_streams_current = 0;
		vob->delay = 0;
		vob->forced_subs=0;
		
		/* read in the index */
		while (vobsub_parse_one_line(vob,parser_info) >= 0);
		
		/* if no palette in .idx then use custom colors */
		if ((vob->custom == 0)&&(vob->have_palette!=1))
			vob->custom = 1;
		if (spu && vob->orig_frame_width && vob->orig_frame_height)
			*spu = spudec_new_scaled_vobsub(vob->palette, vob->cuspal, vob->custom, vob->orig_frame_width, vob->orig_frame_height,parser_info);

		/* read the indexed mpeg_stream */
		
		mpg = mpeg_open(parser_info);
		
		if (mpg == NULL) {
			printf("VobSub: Can't open SUB file\n");
			parser_info->fops.free(vob);
			return NULL;
		} 
		else {
			
			long last_pts_diff = 0;
			while(!parser_info->fops.osfeof(mpg->fp)) {
				
				long pos = parser_info->fops.tell(mpg->fp);
				if (mpeg_run(mpg,parser_info) < 0) {
					if (!parser_info->fops.osfeof(mpg->fp))
						printf("VobSub: mpeg_run error\n");
					printf("MPEG RUN COMPLETE!\n");
					break;
				}
				
				if (mpg->packet_size) {
					if ((mpg->aid & 0xe0) == 0x20) {
						unsigned int sid = mpg->aid & 0x1f;
						
						if (vobsub_ensure_spu_stream(vob, sid,parser_info) >= 0)  {
							
							packet_queue_t *queue = vob->spu_streams + sid;
							/* get the packet to fill */
							//printf("queue=0x%x,spu_streams=0x%x,sid=%d\n",queue,vob->spu_streams,sid);
							if (queue->packets_size == 0 && packet_queue_grow(queue,parser_info)<0){
								printf("%s,%d:Error queue grow!\n",__FILE__,__LINE__);
								return NULL;
							}
							
							while (queue->current_index + 1 < queue->packets_size && queue->packets[queue->current_index + 1].filepos <= pos){
								++queue->current_index;
							}
							if (queue->current_index < queue->packets_size) {
								raw_packet_t *pkt;
								if (queue->packets[queue->current_index].data) {
									/* insert a new packet and fix the PTS ! */
									packet_queue_insert(queue,parser_info);
									
									queue->packets[queue->current_index].pts100 = mpg->pts + last_pts_diff;
								}
								pkt = queue->packets + queue->current_index;
								if (pkt->pts100 != 0xFFFFFFFF) {
									if (queue->packets_size > 1)
										last_pts_diff = pkt->pts100 - mpg->pts;
									else
										pkt->pts100 = mpg->pts;
									/* FIXME: should not use mpg_sub internal informations, make a copy */
									pkt->data = mpg->packet;
									pkt->size = mpg->packet_size;
									//printf("Queue=0x%x,Pkt Index=%d,data=0x%x,size=0x%x\n",queue->packets ,queue->current_index,pkt->data,pkt->size);
									mpg->packet = NULL;
									mpg->packet_reserve = 0;
									mpg->packet_size = 0;
								}
							}
						}
						else
							printf("don't know what to do with subtitle #%u\n", sid);
					}
				}
			}
			vob->spu_streams_current = vob->spu_streams_size;
			while (vob->spu_streams_current-- > 0)
				vob->spu_streams[vob->spu_streams_current].current_index = 0;
			mpeg_free(mpg,parser_info);
		}
    	}
	
	vobsub_info->vob = vob;
	return vob;
}


int vobsub_get_packet(void *vobsub_info, void** data, unsigned int* timestamp,unsigned int *timeend)
{
	raw_packet_t *pkt=NULL;
	vobsub_t *vob = ((vobsub_info_t *)vobsub_info)->vob;
	if (vob->spu_streams && 0 <= vobsub_id && (unsigned) vobsub_id < vob->spu_streams_size) {
		packet_queue_t *queue = vob->spu_streams + vobsub_id;
		if(queue->current_index>=queue->packets_size)//larger than the max, return -2
			return -2;
		else if(queue->current_index<0)//less than 0 return -3
			return -3;
		pkt = queue->packets + queue->current_index;
		*data = pkt->data;
		*timestamp = pkt->pts100;
		*timeend = pkt->ptsend;
		//printf("{Curidx[%d],timeS=%d,timeE=%d}",queue->current_index,pkt->pts100,pkt->ptsend);
		return pkt->size;
	}
	return -1;
}

int vobsub_move_packet(void *vobsub_info,int dir)
{
	vobsub_t *vob = ((vobsub_info_t *)vobsub_info)->vob;
	if (vob->spu_streams && 0 <= vobsub_id && (unsigned) vobsub_id < vob->spu_streams_size) {
		packet_queue_t *queue = vob->spu_streams + vobsub_id;
		 if(dir==GET_PACKET_NEXT){
			queue->current_index++;
			if(queue->current_index<0)
				queue->current_index=0;
			if(queue->current_index>=queue->packets_size){
				queue->current_index = queue->packets_size;
				return -1;
			}
		}
		else if(dir==GET_PACKET_PREV){
			queue->current_index--;
			if(queue->current_index>=queue->packets_size)
				queue->current_index=queue->packets_size-1;
			if(queue->current_index<0){
				queue->current_index =-1;
				return -1;
			}
		}
		else
			printf("%s,%d:Do nothing!\n",__FILE__,__LINE__);
		//printf("queue->current_index===================%d\n",queue->current_index);
		return queue->current_index;
	}
	return -1;
}

int vobsub_set_packetidx(void *vobsub_info,int idx)
{
	vobsub_t *vob = ((vobsub_info_t *)vobsub_info)->vob;
	if (vob->spu_streams && 0 <= vobsub_id && (unsigned) vobsub_id < vob->spu_streams_size) {
		packet_queue_t *queue = vob->spu_streams + vobsub_id;
		queue->current_index = idx;
		return 0;
	}
	return -1;
}


int vobsub_get_next_packet(void *vobsub_info, void** data,unsigned int* timestamp)
{
	int size;	
	int cur_idx=0;
	unsigned int timeend=0;
/*	vobsub_t *vob = ((vobsub_info_t *)vobsub_info)->vob;
	if (vob->spu_streams && 0 <= vobsub_id && (unsigned) vobsub_id < vob->spu_streams_size) {
		packet_queue_t *queue = vob->spu_streams + vobsub_id;
		if(queue->current_index<0)
			queue->current_index = 0;
		if (queue->current_index < queue->packets_size) {
			raw_packet_t *pkt = queue->packets + queue->current_index;
			++queue->current_index;
			*data = pkt->data;
			*timestamp = pkt->pts100;
			return pkt->size;
		}
	}
	return -1;
	*/
	size = vobsub_get_packet(vobsub_info,data,timestamp,&timeend);
	//printf("size===================%d\n",size);
	return size;
}

int __vobsub_packet_search_prev(vobsub_info_t *vobsub_info,unsigned int ms,unsigned int *timestamp,unsigned int *timeend)
{
	int cur_idx=0,packet_len,is_find=0;
	unsigned int fixed_time;
	void *packet;
	while(1){
		cur_idx = vobsub_move_packet(vobsub_info,GET_PACKET_PREV);
		//printf("cur_idx0000000============%d\n",cur_idx);
		if(cur_idx==-1)
			break;
		else{
			packet_len = vobsub_get_packet(vobsub_info,&packet,timestamp,timeend);
			
			//printf("packet_len0000000============%d\n",packet_len);
			if(packet_len<0){
				printf("%s,%d:Logic Err!\n",__FILE__,__LINE__);
				while(1);
			}
			else{
				//printf("ms====================%d\n",ms);
				//printf("timestamp====================%d\n",timestamp);
				//printf("timeend====================%d\n",timeend);
				if(ms>=*timestamp && ms<=*timeend){
					fixed_time = *timestamp;
					is_find=1;
					while(1){
					//the packet may have the same timestamp, should searching forward to find the beginning of the matched timestamp 
						cur_idx = vobsub_move_packet(vobsub_info,GET_PACKET_PREV);
					
						//printf("cur_idx111111============%d\n",cur_idx);
						if(cur_idx==-1)
							break;
						packet_len = vobsub_get_packet(vobsub_info,&packet,timestamp,timeend);
						//printf("packet_len111111============%d\n",packet_len);

						if(*timestamp==fixed_time)
							continue;
						else{
							vobsub_move_packet(vobsub_info,GET_PACKET_NEXT);
							break;
						}
					}
					break;
				}
				else
					continue;
			}
		}
	}
	return is_find;
}

int __vobsub_packet_search_next(vobsub_info_t *vobsub_info,unsigned int ms,unsigned int *timestamp,unsigned int *timeend)
{
	int cur_idx=0,packet_len,is_find=0;
	unsigned int fixed_time;
	void *packet;
	while(1){
		cur_idx = vobsub_move_packet(vobsub_info,GET_PACKET_NEXT);
		if(cur_idx==-1)
			break;
		else{
			packet_len = vobsub_get_packet(vobsub_info,&packet,timestamp,timeend);
			if(packet_len<0){
				printf("%s,%d:Logic Err!\n",__FILE__,__LINE__);
				while(1);
			}
			else{
				if(ms<=*timeend && ms>=*timestamp){
					fixed_time = *timestamp;
					is_find=1;
					break;
				}
				else
					continue;
			}
		}
	}
	return is_find;
}



int vobsub_query_line_info(parser_info_t *parser_info,struct time_line *time_query, line_info_t *line_info)
{
	int packet_len=0;
	int cur_idx=0,is_find=0,fix_idx=0;
	void *packet;
	unsigned int timestamp;
	unsigned int timeend;
	vobsub_info_t *vobsub_info = (vobsub_info_t *)parser_info->finfo;
	unsigned int ms;
	unsigned long time_match=time2hunsec(time_query);
	ms = time_match*10*90;
	memset(line_info,0,sizeof(line_info_t));
	//printf("QUERY TIME=%d,h=%d,m=%d,s=%d\n",ms,time_query->hour,time_query->min,time_query->sec);
	packet_len = vobsub_get_packet(vobsub_info,&packet,&timestamp,&timeend);
	//printf("packet_len===========================%d\n",packet_len);
	//printf("ms=================%d\n",ms);
	//printf("timestamp=================%d\n",timestamp);
	if(packet_len==-2){
		is_find = __vobsub_packet_search_prev(vobsub_info,ms,&timestamp,&timeend);
	}
	else if(packet_len==-3)
		is_find = __vobsub_packet_search_next(vobsub_info,ms,&timestamp,&timeend);
	else{
		if(ms>timeend){
			is_find = __vobsub_packet_search_next(vobsub_info,ms,&timestamp,&timeend);
		}
		else if(ms<timestamp){
			
			is_find = __vobsub_packet_search_prev(vobsub_info,ms,&timestamp,&timeend);
		}
		else{
			is_find=1;//the current packet matched the condition
		}
	}
	
	//printf("Is Find00000==%d\n",is_find);
	if(is_find){
		if(vobsub_info->cur_time!=timestamp){
			vobsub_info->cur_time=timestamp;
			//printf("packet_len====================%d\n",packet_len);
			while ((packet_len=vobsub_get_next_packet(parser_info->finfo, &packet, &timestamp)) >= 0) {
				if(vobsub_show_image(packet,packet_len,timestamp,parser_info,line_info)==1)
					break;
				else
					cur_idx = vobsub_move_packet(vobsub_info,GET_PACKET_NEXT);
					
			}
			hunsec2time(&line_info->t_start,timestamp/900);
			hunsec2time(&line_info->t_end,timeend/900);
			line_info->len=line_info->img_w*line_info->img_h*2;
		}
		else{
			//printf("Image is not Change!\n");
		}
	}
	//printf("Is Find11111==%d\n",is_find);
	return is_find;
}


void *vobsub_get_info(parser_info_t*parser_info)
{
	vobsub_info_t *vobsub_info = (vobsub_info_t*)parser_info->fops.malloc(sizeof(vobsub_info_t));
	if(vobsub_info==NULL){
		printf("%s,%d:Sorry Malloc VOBSUB info Failed!\n",__FILE__,__LINE__);
		return NULL;
	}
	vobsub_info->vob = NULL;
	vobsub_info->spudec=NULL;
	vobsub_info->cur_time=0;
	return vobsub_info;
}

int vobsub_depose_res(parser_info_t *parser_info)
{
	vobsub_info_t *vobsub_info = (vobsub_info_t *)parser_info->finfo;
	vobsub_t *vob =vobsub_info->vob;
	spudec_handle_t *spu = vobsub_info->spudec;
	if(vobsub_info==NULL)
		return 0;
	if(vob!=NULL){
		if (vob->spu_streams) {
			while (vob->spu_streams_size--)
				packet_queue_destroy(vob->spu_streams + vob->spu_streams_size,parser_info);
			parser_info->fops.free(vob->spu_streams);
		}
		parser_info->fops.free(vob);
		vobsub_info->vob = NULL;
	}
	if(spu!=NULL){
		if(spu->output_buf!=NULL)
			parser_info->fops.free(spu->output_buf);
		parser_info->fops.free(spu);
		vobsub_info->spudec = NULL;
	}
	parser_info->fops.free(vobsub_info);
	parser_info->finfo = NULL;
	return 0;
}


static void output_pgm(FILE *f, int w, int h, unsigned char *src, unsigned char *srca, int stride)
{
   	 int x, y;
	unsigned char c=0;
	printf("W=%d,H=%d\n",w,h);
  	 fprintf(f,
		"P5\n"
		"%d %d\n"
		"255\n",
		w, h);
    for (y = 0; y < h; ++y) {
		for (x = 0; x < w; ++x) {
			int res;
			if (srca[x])
				res = src[x] * (256 - srca[x]);
			else
				res = 0;
			res = res>>8;
			if(res!=0)
				c = 0xff;
			else
				c = res&0xff;
			putc(c,f);
			putc(0x80,f);
		}
		src += stride;
		srca += stride;
    }
  	putc('\n', f);
}

static void dump_output_buf(void*buf,int w, int h,int random,int output_format)
{
	FILE *f;
	char namebuf[128];
	unsigned int filelen=0;
	unsigned int realnum=0;
	sprintf(namebuf, "%s/subtitle_%d_%d_%d_%d.dat",DUMP_BUFFER_DEST_DIR,file_idx,w,h,random/90);
	

	if(output_format==OUTPUT_FORMAT_RGB565 || output_format==OUTPUT_FORMAT_YUV422)
		filelen=w*h*2;
	else if(output_format == OUTPUT_FORMAT_OSD2BIT){
		if(w*h%4==0)
			filelen= w*h/4;
		else
			filelen= w*h/4+1;
	}
	else{
		printf("%s,%d:Out Put Format is not support format==%d!",__FILE__,__LINE__,output_format);
		return;
	}
		
	printf("Dump Output Buf, filename=%s,w=%d,h=%d,format=%d,filelen=%d",namebuf,w,h,output_format,filelen);
	f = fopen(namebuf, "wb");
	if(f==NULL){
		printf("%s,%d:Open TMP File Failed!name=%s,filelen==%d\n",__FILE__,__LINE__,namebuf,filelen);
		while(1);
	}
	file_idx++;
	realnum = fwrite(buf, sizeof(unsigned char), filelen,f);
	if(realnum!=filelen)
		printf("%s,%d:Save Error!\n",__FILE__,__LINE__);
	fclose(f);
	
}
#define __get_byte(which_pix)	((which_pix)/4)
#define __get_bits(which_pix)	(((which_pix)%4)*2)





static int vobsub_draw_in_outputbuf(void*buf,int w, int h, unsigned char *src, unsigned char *srca, int stride,char output_format)
{
   	int x, y;
	int which_pix=0;
	int which_byte=0;
	unsigned int offset=0;
	int res;
	unsigned short int *output_buf=(unsigned short int *)buf;
	unsigned char *osd2bit_buf=(unsigned char*)buf;
	
	unsigned char *osd8bit_buf=(unsigned char*)buf;
	unsigned char pixvalue=0;
	unsigned short int c=0;
	//printf("%s,%d:w===%d,h====%d\n",__FILE__,__LINE__,w,h);

	switch(output_format){
		case OUTPUT_FORMAT_RGB565:
		case OUTPUT_FORMAT_YUV422:
			memset(buf,0,w*h*2);
			break;
		case OUTPUT_FORMAT_OSD2BIT:
			memset(buf,0,w*h/4);
			break;
		case OUTPUT_FORMAT_OSD8BIT:
			memset(buf,0,w*h);
			break;
	}
	
	for (y = 0; y < h; ++y) {
		for (x = 0; x < w; ++x) {
			if (srca[x])
				res = src[x] * (256 - srca[x]);
			else
				res = 0;
			res = res>>8;
			if(output_format==OUTPUT_FORMAT_RGB565){
				if(res!=0)
					c = 0xffff;
				else
					c = 0;
				*(output_buf+offset)=c;
				offset++;
			}
			else if(output_format==OUTPUT_FORMAT_YUV422){
				if(res!=0)
					c = 0x80ff;
				else
					c = 0x8000;
				*(output_buf+offset)=c;
				offset++;
			}
			else if(output_format == OUTPUT_FORMAT_OSD2BIT){
				if(res!=0)
					pixvalue = 1;
				else
					pixvalue = 0;
				which_byte = __get_byte(which_pix);
				*(osd2bit_buf+which_byte) = *(osd2bit_buf+which_byte)  | (pixvalue<<__get_bits(which_pix));
				which_pix ++;
			}
			else if(output_format==OUTPUT_FORMAT_OSD8BIT){
		
				if(res!=0)
					c = 1;
				else
					c = 0;
				*(osd8bit_buf+offset)=c;
				offset++;
			}
			else
				printf("%s,%d:Crazy:Out Put Format Error!\n");

		}
		src += stride;
		srca += stride;
	}
	return 1;
}

static int vobsub_draw_alpha(spudec_handle_t* spu)
{
	FILE *f;
	char buf[128];
	sprintf(buf, "%s/subtitle-%d-%d.pgm", DUMP_BUFFER_DEST_DIR,spu->start_pts / 90, spu->end_pts / 90);
	f = fopen(buf, "w");
	if(f==NULL){
		printf("%s,%d:Open TMP File Failed!\n",__FILE__,__LINE__,buf);
		while(1);
	}
	output_pgm(f,spu->width,spu->height,spu->image,spu->aimage,spu->stride);
	fclose(f);
	return 1;
}

static int vobsub_check_outputbuf(spudec_handle_t *spudec,parser_info_t *parser_info)
{
	unsigned int buflen_need,buflen_prev;
	char *tmp;
	buflen_need = spudec->width*spudec->height*2;
	buflen_prev = spudec->output_w*spudec->output_h*2;
	if(spudec->output_buf==NULL){
		spudec->output_buf=parser_info->fops.malloc(buflen_need);
		if(spudec->output_buf==NULL)
			return 0;
		spudec->output_w = spudec->width;
		spudec->output_h = spudec->height;
	}
	else{
		if(buflen_need>buflen_prev){
			tmp = parser_info->fops.realloc(spudec->output_buf,buflen_need);
			if(tmp==NULL){
				printf("%s,%d:Realloc Failed!\n",__FILE__,__LINE__);
				return 0;
			}
			spudec->output_buf = tmp;
		}
		spudec->output_w = spudec->width;
		spudec->output_h = spudec->height;	
	}
	return 1;
}

static int vobsub_draw_outputbuf(spudec_handle_t*spu)
{
	vobsub_draw_in_outputbuf(spu->output_buf,spu->width,spu->height,spu->image,spu->aimage,spu->stride,spu->output_format);
	//dump_output_buf(spu->output_buf, spu->width, spu->height,spu->start_pts,spu->output_format);
	return 1;
}

int _print_time(unsigned int ms)
{
	unsigned int tmp_ms = ms;
	int s,m,h;
	s = (ms/90)/1000;
	h = s/3600;
	s = s%3600;
	m = s/60;
	s = s%60;
	printf("h,m,s,ms==%d,%d,%d,%d\n",h,m,s,(ms/90)%1000);
	return 0;
}

int vobsub_show_image(unsigned char *packet,unsigned int packet_len,unsigned int pts100,parser_info_t *parser_info,line_info_t *line_info)
{
	vobsub_info_t *vobsub_info = (vobsub_info_t *)parser_info->finfo;
	spudec_handle_t *spudec=NULL;
	int is_draw=0;
	spudec = vobsub_info->spudec;
	spudec_assemble(spudec,packet, packet_len, pts100,parser_info);

	if (spudec->queue_head) {
		_print_time(pts100);
		spudec_heartbeat(spudec, spudec->queue_head->start_pts,parser_info);
		if (spudec_changed(spudec)){
			
			if(vobsub_check_outputbuf(spudec,parser_info)){
				is_draw=spudec_draw(spudec, vobsub_draw_outputbuf);
				spudec->queue_head=NULL;
				line_info->img_w = spudec->output_w;
				line_info->img_h = spudec->output_h;
				line_info->str_show = spudec->output_buf;
				printf("<<out Put Buf=0x%s,w=%d,h=%d>>",spudec->output_buf,spudec->output_w,spudec->output_h);
			}
		}
	}
	return is_draw;
}

///just for test
int _test_vobsub_query_line_info(parser_info_t *parser_info)
{

	struct time_line time_query;
	line_info_t line_info;
	time_query.hour =0;
	time_query.min =0;
	time_query.sec =2;
	time_query.millsec = 0;
	vobsub_query_line_info(parser_info,&time_query,&line_info);
	time_query.hour =0;
	time_query.min =0;
	time_query.sec =24;
	time_query.millsec = 0;
	vobsub_query_line_info(parser_info,&time_query,&line_info);
	/*{
		int packet_len=0;
		void *packet;
		unsigned int timestamp;
		unsigned int timeend;
		while ((packet_len=vobsub_get_next_packet(parser_info->finfo, &packet, &timestamp)) >= 0) {
			if(vobsub_show_image(packet,packet_len,timestamp,parser_info,&line_info)==1)
				printf("Good A MAP!\n");
		}
	}*/


	//printf("%02d:%02d:%02d,%03d-->%02d:%02d:%02d,%03d\n",line_info.t_start.hour,line_info.t_start.min,\
								line_info.t_start.sec,line_info.t_start.millsec,line_info.t_end.hour,\
								line_info.t_end.min,line_info.t_end.sec,line_info.t_end.millsec);
	return 0;
}
