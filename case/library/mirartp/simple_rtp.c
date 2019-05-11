/* Imported from the dvbstream-0.2 project
 *
 * Modified for use with MPlayer, for details see the changelog at
 * http://svn.mplayerhq.hu/mplayer/trunk/
 * $Id: rtp.c 29305 2009-05-13 02:58:57Z diego $
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>


#if !HAVE_WINSOCK2_H
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <errno.h>


/* MPEG-2 TS RTP stack */

#include "simple_rtp.h"

// RTP reorder routines
// Also handling of repeated UDP packets (a bug of ExtremeNetworks switches firmware)
// rtpreord procedures
// write rtp packets in cache
// get rtp packets reordered

static int prev_seq = -6;


struct rtpbits {
  unsigned int v:2;           /* version: 2 */
  unsigned int p:1;           /* is there padding appended: 0 */
  unsigned int x:1;           /* number of extension headers: 0 */
  unsigned int cc:4;          /* number of CSRC identifiers: 0 */
  unsigned int m:1;           /* marker: 0 */
  unsigned int pt:7;          /* payload type: 33 for MPEG2 TS - RFC 1890 */
  unsigned int sequence:16;   /* sequence number: random */
};

struct rtpheader {	/* in network byte order */
  struct rtpbits b;
  int timestamp;	/* start: random */
  int ssrc;		/* random */
};

struct rtpbuffer
{
	unsigned char  data[MAXRTPPACKETSIN][PACKET_BUFFER_SIZE];
	unsigned short  seq[MAXRTPPACKETSIN];
	unsigned short  len[MAXRTPPACKETSIN];
	unsigned short first;
};
static struct rtpbuffer rtpbuf;


static int getrtp2(int fd, struct rtpheader *rh, char** data, int* lengthData);

static int rtp_create_and_bind(const char *addr, int port)
{
	int err;
	int optval = 1;
	int sock=-1;
	struct sockaddr_in saddr;
	int rcvbufsize,rbslen=sizeof(int),rbsreturn;
	struct timeval tv;

	saddr.sin_family = AF_INET;
	err = inet_aton (addr, &saddr.sin_addr);
	if (err < 0)
	{
		printf("Error in socket address:%s.\n", strerror(errno));
		return -1;
	}
	saddr.sin_port = htons (port);

	sock = socket (PF_INET, SOCK_DGRAM, 0);
	if (sock==-1){ 
		printf("Error [%s][%d]\n", __FUNCTION__,__LINE__);
		return -1;
	}

	#if 1
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	err = setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO,&tv, sizeof (tv));
	if (err < 0)
	{
		printf("Fail to set SO_RCVTIMEO socket: %s.", strerror(errno));
	}
	#elif 0
	current_opts = fcntl(sock, F_GETFL, 0 );
	fcntl(sock, F_SETFL, current_opts | O_NONBLOCK);
	#endif
	
	err = setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,&optval, sizeof (optval));
	if (err < 0)
	{
		printf("Fail to set rtp address reusable: %s.",strerror(errno));
	}

	// set the tcp received size to 200k
	rcvbufsize = 500*1024;
	rbslen=sizeof(int);
	rbsreturn = setsockopt(sock,SOL_SOCKET,SO_RCVBUF,&rcvbufsize,rbslen);
	if(rbsreturn < 0) {
		printf("Fail to set socket rcv buffer: %s.",strerror(errno));
	}
	

	err = bind (sock,(struct sockaddr *) &saddr,sizeof (saddr));
	if (err != 0)
	{
		printf("Fail to bind rtp socket to port %d: %s.", port,strerror(errno));
		close (sock);
		return -1;
	}

	return sock;
}


// RTP Reordering functions
// Algorithm works as follows:
// If next packet is in sequence just copy it to buffer
// Otherwise copy it in cache according to its sequence number
// Cache is a circular array where "rtpbuf.first" points to next sequence slot
// and keeps track of expected sequence

// Initialize rtp cache
static void rtp_cache_reset(unsigned short seq)
{
	int i;

	rtpbuf.first = 0;
	rtpbuf.seq[0] = ++seq;

	for (i=0; i<MAXRTPPACKETSIN; i++) {
		rtpbuf.len[i] = 0;
	}
}

// Write in a cache the rtp packet in right rtp sequence order
static int rtp_cache(int fd, char *buffer, int length)
{
	struct rtpheader rh;
	int newseq;
	char *data;
	unsigned short seq;
	static int is_first = 1;
	static int successive_cnt = 0;
	int i,j;

	getrtp2(fd, &rh, &data, &length);
	if(length==0){
		printf(">>> getrtp2 error: length is 0\n");
		return 0;
	}
	else if(length<0){
		printf(">>> getrtp2 error: length is -1\n");
		return -1;
	}
	seq = rh.b.sequence;

	newseq = seq - rtpbuf.seq[rtpbuf.first];
	
	if ((newseq == 0) || is_first)
	{
		
		//mp_msg(MSGT_NETWORK, MSGL_DBG4, "RTP (seq[%d]=%d seq=%d, newseq=%d)\n", rtpbuf.first, rtpbuf.seq[rtpbuf.first], seq, newseq);
		//printf("s:%d\n",seq);
		if(is_first){
			rtp_cache_reset(seq);
		}
		
		/**
		if(seq != (prev_seq+1)){
			printf("2>>prev:%d,next:%d\n",prev_seq,seq);
		}
		prev_seq = seq;
		*/
		
		if(is_first==0){
			rtpbuf.first = ( 1 + rtpbuf.first ) % MAXRTPPACKETSIN;
			rtpbuf.seq[rtpbuf.first] = ++seq;
		}
		else{
			is_first = 0;
		}
		goto feed;
	}

	if (newseq >= MAXRTPPACKETSIN)
	{
		printf("Overrun(seq[%d]=%d seq=%d, newseq=%d)\n", rtpbuf.first, rtpbuf.seq[rtpbuf.first], seq, newseq);
		i=rtpbuf.first;
		do{
			if(rtpbuf.len[i] > 0){
				printf("overdrop:%d,%d\n",rtpbuf.seq[i],rtpbuf.len[i]);
			}
			i++;
			i%=MAXRTPPACKETSIN;
		}while(i!=rtpbuf.first);
		
		rtp_cache_reset(seq);

		/**
		printf("s:%d\n",seq);
		if(seq != (prev_seq+1)){
			printf("3>>prev:%d,next:%d\n",prev_seq,seq);
		}
		prev_seq = seq;
		*/
		goto feed;
	}

	if (newseq < 0)
	{
		// Is it a stray packet re-sent to network?
		for (i=0; i<MAXRTPPACKETSIN; i++) {
			if (rtpbuf.seq[i] == seq) {
				printf("Stray packet (seq[%d]=%d seq=%d, newseq=%d found at %d)\n", rtpbuf.first, rtpbuf.seq[rtpbuf.first], seq, newseq, i);
				return  0; // Yes, it is!
			}
		}

		// Some heuristic to decide when to drop packet or to restart everything
		//if (newseq > -(3 * MAXRTPPACKETSIN)) {
		if((-newseq) >= MAXRTPPACKETSIN){
			printf("Too Old packet (seq[%d]=%d seq=%d, newseq=%d)\n", rtpbuf.first, rtpbuf.seq[rtpbuf.first], seq, newseq);
			return  0; // Yes, it is!
		}

		printf("Underrun(seq[%d]=%d seq=%d, newseq=%d)\n", rtpbuf.first, rtpbuf.seq[rtpbuf.first], seq, newseq);

		/**
		printf("s:%d\n",seq);
		if(seq != (prev_seq+1)){
			printf("4>>prev:%d,next:%d\n",prev_seq,seq);
		}
		prev_seq = seq;
		*/
		
		rtp_cache_reset(seq);
		goto feed;
	}

	//printf("Out of Seq (seq[%d]=%d seq=%d, newseq=%d)\n", rtpbuf.first, rtpbuf.seq[rtpbuf.first], seq, newseq);
	newseq = ( newseq + rtpbuf.first ) % MAXRTPPACKETSIN;
	memcpy (rtpbuf.data[newseq], data, length);
	rtpbuf.len[newseq] = length;
	rtpbuf.seq[newseq] = seq;

	successive_cnt++;
	//if(successive_cnt > (MAXRTPPACKETSIN/5)){
	if(successive_cnt > 20){
		/**
		* we are likely to say that the packet we want has been lost,
		* so we drop it and begin the next round.
		*/
		//i = (rtpbuf.first+1)% MAXRTPPACKETSIN;
		i = rtpbuf.first;
		j=0;
		while (rtpbuf.len[i] == 0) {
			printf("drop packet1 %hu\n", rtpbuf.seq[rtpbuf.first]+j);
			i = ( 1 + i ) % MAXRTPPACKETSIN;
			j++;
			if (rtpbuf.first == i) {
				printf("drop too much\n");
				break;
			}
		}
		if (rtpbuf.first != i){
			rtpbuf.first = i;
		}
		successive_cnt = 0;
	}

	return 0;

feed:
	successive_cnt = 0;
	memcpy (buffer, data, length);
	return length;
}

// Get next packet in cache
// Look in cache to get first packet in sequence
static int rtp_get_next(int fd, char *buffer, int length)
{
	int i;
	unsigned short nextseq;
	
	// If we have empty buffer we loop to fill it
	for (i=0; i < MAXRTPPACKETSIN -3; i++) {
		if (rtpbuf.len[rtpbuf.first] != 0) break;
		length = rtp_cache(fd, buffer, length);
		// returns on first packet in sequence
		if (length > 0) {
			//mp_msg(MSGT_NETWORK, MSGL_DBG4, "Getting rtp [%d] %hu\n", i, rtpbuf.first);
			return length;
		} 
		else if (length < 0) {
			printf(">>> rtp cache error:%d\n",length);
			//break;
			return 0;
		}	
	}

	i = rtpbuf.first;
	while (rtpbuf.len[i] == 0) {
		printf("Lost packet %hu\n", rtpbuf.seq[i]);
		i = ( 1 + i ) % MAXRTPPACKETSIN;
		if (rtpbuf.first == i) break;
	}
	rtpbuf.first = i;

	// Copy next non empty packet from cache
	//printf("Getting rtp from cache [%d] %hu\n", rtpbuf.first, rtpbuf.seq[rtpbuf.first]);
	memcpy (buffer, rtpbuf.data[rtpbuf.first], rtpbuf.len[rtpbuf.first]);
	length = rtpbuf.len[rtpbuf.first]; // can be zero?

	// Reset fisrt slot and go next in cache
	rtpbuf.len[rtpbuf.first] = 0;
	nextseq = rtpbuf.seq[rtpbuf.first];

	/**
	printf("s:%d\n",nextseq);
	if(nextseq != (prev_seq+1)){
		printf("1>> prev:%d,next:%d\n",prev_seq,nextseq);
	}
	prev_seq = nextseq;
	*/	
	rtpbuf.first = ( 1 + rtpbuf.first ) % MAXRTPPACKETSIN;
	rtpbuf.seq[rtpbuf.first] = nextseq + 1;

	return length;

}

#ifdef __RTP_USE_QUEUE_
static int rtp_release_packet(struct rtp_packet *ppacket)
{
	if(ppacket == NULL){
		return -1;
	}

	if(ppacket->pdata){
		free(ppacket->pdata);
	}

	free(ppacket);

	return 0;
}

static int rtp_queue_packet(struct rtp_queue *queue,char *data,int len,int seq)
{
	struct rtp_packet *packet;
	struct rtp_packet *prev,*next;

	if(queue==NULL || data==NULL || len==0){
		return -1;
	}

	if( (seq<queue->next_packet_seq) && (queue->next_packet_seq-seq)<(__RTP_SEQUENCE_MAX)/4){
		printf("drop old pacekt:%d,%d\n",seq,queue->next_packet_seq);
		if((queue->prev_drop_old_seq+1)==seq){
			++queue->drop_old_continue_num;
			if(queue->drop_old_continue_num > 50){
				printf("reset rtp queue next packet seq to 0.\n");
				queue->next_packet_seq = 0;
				queue->drop_old_continue_num = 0;
			}
		}else{
			queue->drop_old_continue_num = 0;
		}
		queue->prev_drop_old_seq = seq;
		return -1;
	}else{
		queue->drop_old_continue_num = 0;
	}

	if(queue->cur_packets >= queue->max_packets ){
		printf("rtp queue is full, drop the oldest\n");
		packet = queue->qhead.next;
		queue->qhead.next = packet->next;
		rtp_release_packet(packet);
		queue->cur_packets--;
	}

	packet = (struct rtp_packet *)malloc(sizeof(struct rtp_packet));
	if(packet == NULL){
		printf("not enough memory for rtp packet\n");
		return -1;
	}

	packet->pdata = (char *)malloc(len);
	if(packet->pdata == NULL){
		printf("not enough memory for rtp packet\n");
		free(packet);
		return -1;
	}

	memcpy(packet->pdata,data,len);
	packet->len = len;
	packet->next = NULL;
	packet->seq = seq;

	/** insert the packet to queue */
	if(queue->qhead.next == NULL){
		queue->qhead.next = packet;
		queue->cur_packets++;
	}
	else{
		next = queue->qhead.next;
		prev = &queue->qhead;

		while(next){
			/**
			* why add condition (next->seq-seq)<60000?
			* because to prevent sequence overflow.
			*/
			if(next->seq >= seq && (next->seq-seq)<60000){
				break;
			}
			prev = next;
			next = next->next;
		}
		if(next && next->seq == seq){
			printf("duplicated rtp packet:%d\n",seq);
			free(packet->pdata);
			free(packet);
			return -1;
		}
		else{
			prev->next = packet;
			packet->next = next;
			queue->cur_packets++;
		}
	}
	
	return 0;
}

static int rtp_get_next_from_queue(struct simple_rtp_session *session, char *buffer, int length)
{
	struct rtp_queue *queue = &session->rtp_packet_queue;
	struct rtp_packet *ppacket,*newpacket,*prev;
	struct rtpheader rh;
	char *data;
	int len;
	
	if(session == NULL){
		return 0;
	}

	if(queue->first){
		/** first time fetch data */
		getrtp2(session->sock, &rh, &data, &len);
		if(len <=0 ){
			printf("rtp first time get data error\n");
			return 0;
		}
		queue->first = 0;
		queue->next_packet_seq = rh.b.sequence+1;
		queue->next_packet_seq %= __RTP_SEQUENCE_MAX;
		if(len > length){
			printf("rtp data too big 1,drop paceket:%d\n",rh.b.sequence);
			return 0;
		}
		memcpy(buffer,data,len);
		return len;
	}
	ppacket = queue->qhead.next;
	if(ppacket){
		if(ppacket->seq == queue->next_packet_seq){
			/** that's the packet we want */
			len = ppacket->len;
			if( len <= length && len>0){
				memcpy(buffer,ppacket->pdata,len);
			}
			else{
				len = 0;
				printf("rtp data too big 2,drop paceket:%d,len=%d\n",queue->next_packet_seq,ppacket->len);
			}
			queue->next_packet_seq++;
			queue->next_packet_seq %= __RTP_SEQUENCE_MAX;
			queue->qhead.next = ppacket->next;
			rtp_release_packet(ppacket);
			queue->cur_packets--;
			return len;
		}
		else{
			/** if queue full, we need to drop some packets */
			if(queue->cur_packets >= queue->max_packets){
				printf("drop packets:%d-%d\n",queue->next_packet_seq,ppacket->seq-1);
				queue->next_packet_seq = ppacket->seq;
				return 0;
			}
		}
	}
	/** get new packet */
	getrtp2(session->sock, &rh, &data, &len);
	if(len <=0 ){
		printf("rtp get data error len=%d\n",len);
		return 0;
	}
	/** the new packet if exactly what we want */
	if(rh.b.sequence == queue->next_packet_seq){
		if(len > length){
			printf("rtp data too big 2,drop paceket:%d\n",rh.b.sequence);
			queue->next_packet_seq++;
			return 0;
		}
		memcpy(buffer,data,len);
		queue->next_packet_seq++;
		queue->next_packet_seq %= __RTP_SEQUENCE_MAX;
		queue->drop_old_continue_num = 0;
		return len;
	}
	/** queue the packet */
	rtp_queue_packet(queue,data,len,rh.b.sequence);
	return 0;
}
#endif

// Read next rtp packet using cache
int read_rtp_from_server(int fd, char *buffer, int length) 
{
	char tmp[1600];
	int len,total=0;
	
	// Following test is ASSERT (i.e. uneuseful if code is correct)
	if(buffer==NULL || length<PACKET_BUFFER_SIZE) {
		printf("RTP buffer invalid; no data return from network\n");
		return 0;
	}

#if 0
	// loop just to skip empty packets
	while ((length = rtp_get_next(fd, buffer, length)) == 0) {
		printf("Got empty packet from RTP cache!?\n");
	}

	return length;
#else

	while(total < length){
		len = rtp_get_next(fd, tmp, 1600);
		if(len > 0){
			memcpy(buffer+total,tmp,len);
			total += len;
			
			/** gap protection */
			if(total+1600 >= length){
				break;
			}
		}
	}

	return total;
#endif

}

int read_rtp_from_server2(struct simple_rtp_session *session,char *buffer, int length) 
{
	char tmp[1600];
	int len,total=0;
	int fd;
	int zero_len=0;

	fd = session->sock;
	if(fd < 0){
		return 0;
	}
	
	if(buffer==NULL || length<=0) {
		printf("RTP buffer invalid; no data return from network\n");
		return 0;
	}

	while(total < length){
		
		if(session->abort){
			printf("RTP abort session\n");
			total = 0;
			break;
		}
		
	#ifdef __RTP_USE_QUEUE_
		len = rtp_get_next_from_queue(session, tmp, 1600);
	#else
		len = rtp_get_next(fd, tmp, 1600);
	#endif
	
		if(len > 0){
			memcpy(buffer+total,tmp,len);
			total += len;
			/** gap protection */
			if(total+1600 >= length){
				break;
			}
		}
		else{
			
		#ifndef __RTP_USE_QUEUE_
			zero_len++;
			if(zero_len >= 10){
				printf("rtp socket may be error? too much zero length packet!\n");
				break;
			}
		#endif
		}
		
	}
	return total;
}


static int getrtp2(int fd, struct rtpheader *rh, char** data, int* lengthData) 
{
	static char buf[1600];
	unsigned int intP;
	char* charP = (char*) &intP;
	int headerSize;
	int lengthPacket;

	#if 0
	fd_set rfds;
	struct timeval tv;
	int retval;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	tv.tv_sec = 2;
	tv.tv_usec = 0;

	retval = select(fd+1,&rfds,NULL,NULL,&tv);
	if(retval <= 0){
		*lengthData = -1;
		return 0;
	}
	else if(retval > 0){
		if(FD_ISSET(fd,&rfds)){
			lengthPacket=recv(fd,buf,1590,0);
		}
		else{
			*lengthData = 0;
			return 0;
		}
	}
	#else
	lengthPacket=recv(fd,buf,1590,0);
	#endif

	if (lengthPacket<0){
		if(EWOULDBLOCK == errno){
			printf("[%s]socket time out!\n", __func__);
			*lengthData = -1;
			return 0;
		}
		printf("rtp: socket read error\n");
		*lengthData = 0;
		return 0;
	}
	else if (lengthPacket<12){
		printf("rtp: packet too small (%d) to be an rtp frame (>12bytes)\n", lengthPacket);
		*lengthData = 0;
		return 0;
	}
	
	rh->b.v  = (unsigned int) ((buf[0]>>6)&0x03);
	rh->b.p  = (unsigned int) ((buf[0]>>5)&0x01);
	rh->b.x  = (unsigned int) ((buf[0]>>4)&0x01);
	rh->b.cc = (unsigned int) ((buf[0]>>0)&0x0f);
	rh->b.m  = (unsigned int) ((buf[1]>>7)&0x01);
	rh->b.pt = (unsigned int) ((buf[1]>>0)&0x7f);
	intP = 0;
	memcpy(charP+2,&buf[2],2);
	rh->b.sequence = ntohl(intP);
	intP = 0;
	memcpy(charP,&buf[4],4);
	rh->timestamp = ntohl(intP);

	headerSize = 12 + 4*rh->b.cc; /* in bytes */

	*lengthData = lengthPacket - headerSize;
	*data = (char*) buf + headerSize;

	//if(rh->b.sequence > 65530|| rh->b.sequence<5){
	//	printf("r: %d\n",rh->b.sequence);
	//}
	//  mp_msg(MSGT_NETWORK,MSGL_DBG2,"Reading rtp: v=%x p=%x x=%x cc=%x m=%x pt=%x seq=%x ts=%x lgth=%d\n",rh->b.v,rh->b.p,rh->b.x,rh->b.cc,rh->b.m,rh->b.pt,rh->b.sequence,rh->timestamp,lengthPacket);

	return 0;
}

struct simple_rtp_session * rtp_session_open(char *addr, int port)
{
	struct simple_rtp_session *session = NULL;

	session = (struct simple_rtp_session *)malloc(sizeof(struct simple_rtp_session));
	
	if(session == NULL){
		printf(">>>>> Error %s,%d: Not enough memory\n",__LINE__,__FUNCTION__);
		return NULL;
	}
	memset(session,0,sizeof(struct simple_rtp_session));
	/** open the rtp socket */
	session->sock = rtp_create_and_bind(addr,port);
	if(session->sock <= 0){
		free(session);
		return NULL;
	}
#ifdef __RTP_USE_QUEUE_
	/** init the recv queue */
	memset(&session->rtp_packet_queue,0,sizeof(struct rtp_queue));
	session->rtp_packet_queue.max_packets = __MAX_PACKETS_IN_QUEUE;
	session->rtp_packet_queue.qhead.next = NULL;
	session->rtp_packet_queue.qhead.len = 0;
	session->rtp_packet_queue.qhead.pdata = NULL;
	session->rtp_packet_queue.qhead.seq = 0;
	session->rtp_packet_queue.first = 1;
#else
	/** alloc buffer for session receive */
	session->recv_buf = (unsigned char *)malloc(PACKET_SESSION_SIZE);
	if(session->recv_buf == NULL){
		printf(">>>>> Error %s,%d: Not enough memory\n",__LINE__,__FUNCTION__);
		close(session->sock);
		free(session);
		return NULL;
	}
#endif
	session->recv_len = 0;
	session->abort = 0;
	return session;
	
}

int rtp_session_close(struct simple_rtp_session *session)
{
#ifdef __RTP_USE_QUEUE_
	struct rtp_packet *next,*prev;
	int cnt=0;
#endif
	if(session == NULL){
		return -1;
	}

	if(session->sock > 0){
		close(session->sock);
	}

#ifdef __RTP_USE_QUEUE_
	next = session->rtp_packet_queue.qhead.next;
	while(next){
		prev = next;
		next = next->next;
		
		/** release resource */
		if(prev->pdata){
			free(prev->pdata);
		}
		free(prev);
		cnt++;
	}
	session->rtp_packet_queue.qhead.next = NULL;
	if(cnt != session->rtp_packet_queue.cur_packets){
		printf("May be memory leak???\n");
	}
#else
	if(session->recv_buf){
		free(session->recv_buf);
		session->recv_buf = NULL;
	}
#endif	
	free(session);
	return 0;
}

int rtp_session_abort(struct simple_rtp_session *session)
{
	if(session == NULL){
		return -1;
	}

	session->abort = 1;

	return 0;
}

int rtp_session_fetch_data(struct simple_rtp_session *session)
{
	int rlen;
	
	if((session == NULL) || (session->sock<=0) || (session->recv_buf==NULL)){
		return 0;
	}

	rlen = read_rtp_from_server(session->sock, (char *)session->recv_buf, PACKET_SESSION_SIZE);
	session->recv_len = rlen;

	return rlen;
	
}

int rtp_session_fetch_data2(struct simple_rtp_session *session,char *buffer,int len)
{
	int rlen;
	
	if((session == NULL) || (session->sock<=0)){
		return 0;
	}

	rlen = read_rtp_from_server2(session, buffer, len);
	
	return rlen;
}


