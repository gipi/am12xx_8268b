/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef MPLAYER_RTP_H
#define MPLAYER_RTP_H

#define __RTP_USE_QUEUE_

#ifdef __RTP_USE_QUEUE_

#define __MAX_PACKETS_IN_QUEUE 15
#define __RTP_SEQUENCE_MAX 65536

/**
* queue rtp packet
*/
struct rtp_packet{
	struct rtp_packet *next;
	char *pdata;
	int len;
	int seq;
};

struct rtp_queue{
	/** 
	* Maximum number of packets to be queued.
	* If received packets exceeds this number,just drop it.
	*/
	int max_packets;

	/**
	* current packets number in the queue.
	*/
	int cur_packets;

	/**
	* sequence number of the next wanted packets.
	*/
	int next_packet_seq;
	int prev_drop_old_seq;
	int drop_old_continue_num;

	int first;

	/**
	* the head of the queue.
	*/
	struct rtp_packet qhead;
	
};

#endif

#define MAXRTPPACKETSIN 512   // The number of max packets being reordered
#define PACKET_BUFFER_SIZE 2048
#define PACKET_SESSION_SIZE PACKET_BUFFER_SIZE*6

struct simple_rtp_session {
	int sock;
	unsigned char *recv_buf;
	int recv_len;
	int abort;
#ifdef __RTP_USE_QUEUE_
	struct rtp_queue rtp_packet_queue;
#endif
};

int read_rtp_from_server(int fd, char *buffer, int length);


/**
* Open a simple rtp session.
*/
extern struct simple_rtp_session * rtp_session_open(char *addr, int port);

/**
* Destroy a simple rtp session.
*/
extern int rtp_session_close(struct simple_rtp_session *session);


/**
* Get data from a simple rtp session.
* Data will be stored in the "session->recv_buf" and data length will
* be indicated by "session->recv_len".
*/
extern int rtp_session_fetch_data(struct simple_rtp_session *session);


#endif /* MPLAYER_RTP_H */

