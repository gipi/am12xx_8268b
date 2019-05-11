/**
 * @file librtspsink.c
 *
 * Implementation of an RTSP sink that is interoperable with
 * Remote Display Specificaiton v1.0 and Wi-Fi Display Specification
 * v1.39.
 *
 * INTEL CONFIDENTIAL
 * Copyright 2010-2012 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.  Title to the Material remains with Intel
 * Corporation or its suppliers and licensors.  The Material contains trade
 * secrets and proprietary and confidential information of Intel or its
 * suppliers and licensors.  The Material is protected by worldwide copyright
 * and trade secret laws and treaty provisions. No part of the Material may
 * be used, copied, reproduced, modified, published, uploaded, posted,
 * transmitted, distributed, or disclosed in any way without Intel's prior
 * express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or
 * delivery of the Materials,  either expressly, by implication, inducement,
 * estoppel or otherwise.  Any license under such intellectual property
 * rights must be express and approved by Intel in writing.
 */

// {{{ Includes
#include <arpa/inet.h>
#include <stdlib.h>      // for free()
#include <string.h>      // for memset, strdup
#include <errno.h>       // for errno
#include <assert.h>
#include <stdarg.h>      // for va_start
#include <unistd.h>      // for close, read, open, sleep
#include <sys/time.h>    // For get_tick, gettimeofday
#include <sys/socket.h>  // For shutdown
#include <fcntl.h>

#include "librtspsink.h"
#include "affinity.h"
// }}}

// {{{ Debug helpers
#ifndef _WIDI_DEBUG_MACROS
#define _WIDI_DEBUG_MACROS
#define DEBUG(...)    logger( __func__, __LINE__, LOG_DEBUG, __VA_ARGS__ )  ///< Messages used for development; they aren't shown for production builds
#define NOTICE(...)   logger( __func__, __LINE__, LOG_NOTICE, __VA_ARGS__ ) ///< Informational messages that do not indicate an abnormal situation
#define ERROR(...)    logger( __func__, __LINE__, LOG_ERR, __VA_ARGS__ )    ///< Messages that indicate an abnormal situation, but program operation can continue
#define CRITICAL(...) logger( __func__, __LINE__, LOG_CRIT, __VA_ARGS__ )   ///< Messages that indicate a severe situation where normal program operation cannot continue
#endif // _WIDI_DEBUG_MACROS
// }}}

#define WIFI_DIRECT_JSON 1		//added for miracast json file
/**
 * Internal helper function to print debug messages. Function name, file name,
 * and line number are included automatically. Outputs to syslog.
 *
 * @returns None
 */
static void logger( const char* func, int line, int priority, char* format, ... )
    __attribute__ ((format (printf, 4, 5)));
static void logger( const char* func, int line, int priority, char* format, ... )
{
    char new_format[255];
    assert( strlen(format) < sizeof(new_format) );
    snprintf( new_format, sizeof(new_format), "%s: %s (%s:%d)", func, format, __FILE__, line );

    va_list args;
    va_start( args, format );
    vsyslog( priority, new_format, args );
    va_end( args );
}

/**
* A wrapper sleep function for rtsp. Resolution is milliseconds.
*/
static void rtsp_local_sleep(unsigned long time_ms)
{
	struct timespec req;
	struct timespec rem;
	int ret;

	req.tv_sec = time_ms/1000;
	req.tv_nsec = (time_ms%1000)*1000*1000;

	for(;;)
	{
		ret = nanosleep(&req, &rem);
		if(ret == -1)
		{
			int errsv = errno;
			if(errsv == EINTR)
			{
				req = rem;
				continue;	//Restart if interrupted by handler
			}
			else
			{
				printf("nanosleep: errno:%d, %s\n", errsv, strerror(errsv));
			}
		}
		break;
	}
}

/**
 * Returns a pointer to a binary representation of a sample EDID that
 * can be used if a real EDID is not available. Stores the length of the
 * EDID in the edidlen parameter.
 *
 * @returns pointer to the binary EDID data
 */
uint8_t* rtsp_get_sample_edid
    ( long* edidlen ///< [out] the length of the returned binary data will be stored here
    )
{
    static uint8_t sample_edid[] = {
        0x0,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0,
        0x4d, 0xd9, 0x0,  0x82, 0x1,  0x1,  0x1,  0x1,
        0x0,  0xf,  0x1,  0x3,  0x80, 0x0,  0x0,  0x78,
        0xa,  0xd,  0xc9, 0xa0, 0x57, 0x47, 0x98, 0x27,
        0x12, 0x48, 0x4c, 0x0,  0x0,  0x0,  0x1,  0x1,
        0x1,  0x1,  0x1,  0x1,  0x1,  0x1,  0x1,  0x1,
        0x1,  0x1,  0x1,  0x1,  0x1,  0x1,  0x2,  0x3a,
        0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
        0x45, 0x0,  0x10, 0x9,  0x0,  0x0,  0x0,  0x1e,
        0x8c, 0xa,  0xd0, 0x8a, 0x20, 0xe0, 0x2d, 0x10,
        0x10, 0x3e, 0x96, 0x0,  0x4,  0x3,  0x0,  0x0,
        0x0,  0x18, 0x0,  0x0,  0x0,  0xfc, 0x0,  0x53,
        0x4f, 0x4e, 0x59, 0x20, 0x54, 0x56, 0xa,  0x20,
        0x20, 0x20, 0x20, 0x20, 0x0,  0x0,  0x0,  0xfd,
        0x0,  0x3b, 0x3d, 0xf,  0x46, 0xf,  0x0,  0xa,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1,  0x45,

        //CEA Extension Block

        0x2,  0x3,  0x1c, 0x76, 0x48, 0x5,  0x90, 0x3,
        0x4,  0x6,  0x7,  0x1,  0x2,  0x23, 0x9,  0x7,
        0x7,  0x83, 0x1,  0x0,  0x0,  0x66, 0x3,  0xc,
        0x0,  0x10, 0x0,  0x80, 0x1,  0x1d, 0x0,  0x72,
        0x51, 0xd0, 0x1e, 0x20, 0x6e, 0x28, 0x55, 0x0,
        0x10, 0x9,  0x0,  0x0,  0x0,  0x1e, 0x8c, 0xa,
        0xa0, 0x14, 0x51, 0xf0, 0x16, 0x0,  0x26, 0x7c,
        0x43, 0x0,  0x4,  0x3,  0x0,  0x0,  0x0,  0x98,
        0x1,  0x1d, 0x80, 0x18, 0x71, 0x1c, 0x16, 0x20,
        0x58, 0x2c, 0x25, 0x0,  0x10, 0x9,  0x0,  0x0,
        0x0,  0x9e, 0x8c, 0xa,  0xd0, 0x8a, 0x20, 0xe0,
        0x2d, 0x10, 0x10, 0x3e, 0x96, 0x0,  0x10, 0x9,
        0x0,  0x0,  0x0,  0x18, 0x0,  0x0,  0x0,  0x0,
        0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
        0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
        0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x61
    };

    *edidlen = (uint16_t)sizeof( sample_edid );

    return sample_edid;
}

#define RTSP_CONFIG_CHECKSUM 0x40c20501

/**
 * Checks the rtsp library handle for a valid checksum. This indicates
 * whether the library has been initialized correctly.
 *
 * @returns 1 if the handle has a valid checksum
 * @returns 0 if the handle is invalid
 */
inline int rtsp_checksum_ok
    ( const rtsp_handle h  ///< [in] rtsp library handle
    )
{
    const int ok = h && ( h->checksum == RTSP_CONFIG_CHECKSUM );
    if ( ! ok ) {
        CRITICAL( "invalid h checksum, was init_rtsp_config() called?" );
    }

    return ok;
}
#if 0
// New code for later - add more conditions, and refactor name to rtsp_handle_ok
inline int rtsp_checksum_ok
    ( const rtsp_handle h  ///< [in] rtsp library handle
    )
{
    if ( ! h || h->checksum != RTSP_CONFIG_CHECKSUM ) {
        CRITICAL( "invalid h checksum, was init_rtsp_config() called?" );
        return 0;
    }

    if ( h->rtsp_mode != RTSP_MODE_RDS && h->rtsp_mode != RTSP_MODE_WFD ) {
        CRITICAL( "invalid h->rtsp_mode" );
        return 0;
    }

    if ( h->library_is_shutdown ) {
        return 0;
    }

    return 1;
}
#endif

/**
 * Helper function to find nodes in the internal parameter key-value
 * linked list.
 *
 * @returns pointer to the rtsp_param_t node with matching "key"
 * @returns NULL if no node with a matching key exists
 */
static rtsp_param_t* rtsp_param_find_node
    ( rtsp_handle h    ///< [in] configured rtsp library handle
    , const char* key  ///< [in] node key to search for
    )
{
    assert( rtsp_checksum_ok(h) );

    rtsp_param_t* p = NULL;

    for ( p = h->params; p != NULL; p = p->next ) {
        if ( ! p->deleted ) {
            if ( strcasecmp( key, p->key ) == 0 ) {
                return p;
            }
        }
    }

    return NULL;
}

/**
 * Helper function to delete nodes from the internal parameter key-value
 * linked list. The memory for the key and value fields is freed, but
 * the node is left in place in the linked list and marked as deleted.
 *
 * @returns Always returns 0
 */
static int rtsp_param_del_node
    ( rtsp_param_t* p  ///< [in] pointer to the node to delete
    )
{
    if ( p ) {
        if ( p->key ) {
            free( p->key );
            p->key = NULL;
        }

        if ( p->value ) {
            free( p->value );
            p->value = NULL;
        }

        p->deleted = 1;
    }

    return 0;
}

/**
 * Removes the parameter with the specified key from the RTSP parameters
 * stored in memory.
 *
 * @returns 0 if the key was found and deleted
 * @returns 1 if the key did not exist
 */
int rtsp_param_remove
    ( rtsp_handle h   ///< [in] configured rtsp library handle
    , const char* key ///< [in] key to remove from the RTSP parameter list
    )
{
    assert( rtsp_checksum_ok(h) );

    rtsp_param_t* p = rtsp_param_find_node( h, key );

    if ( p ) {
        rtsp_param_del_node( p );
        return 0;
    } else {
        return 1;
    }
}

/**
 * Completely removes all nodes from the internal parameters linked
 * list. All memory is freed.
 *
 * @returns Always returns 0
 */
int rtsp_param_free_all
    ( rtsp_handle h   ///< [in] configured rtsp library handle
    )
{
    if ( ! rtsp_checksum_ok(h) ) return 1;

    rtsp_param_t* p = h->params;
    rtsp_param_t* temp;

    while ( p ) {
        rtsp_param_del_node( p );
        temp = p;
        p = p->next;
        free( temp );
    }

    return 0;
}

/**
 * Helper function to allocate memory for a new linked list node and
 * populate the values. The node is not inserted into the linked list.
 *
 * @returns a pointer to the newly created node
 * @returns NULL if memory couldn't be allocated
 */
static rtsp_param_t* rtsp_param_new_node
    ( const char* key   ///< [in]
    , const char* value ///< [in]
    )
{
    rtsp_param_t* p = calloc( 1, sizeof(rtsp_param_t) );
    if ( p == NULL ) {
        CRITICAL( "%s(%d): calloc() failed", __func__, __LINE__ );
        return NULL;
    }

    p->key = strdup( key );
    p->value = strdup( value );

    return p;
}

/**
 * Sets or updates an RTSP parameter. The RTSP library maintains an
 * internal list of RTSP parameters. When a GET_PARAMETER request comes
 * for a parameter, the library will automatically respond with the
 * value. If the specified "key" does not exist, it is added. If the
 * "key" already exists, it is deleted first, then the key is added,
 * effectively implementing an update.
 *
 * @returns 0 on success
 * @returns 1 on failure
 * @returns -1 if handle is invalid
 */
int rtsp_param_set
    ( rtsp_handle h     ///< [in] configured rtsp library handle
    , const char* key   ///< [in] name of the parameter to set
    , const char* value ///< [in] value of the parameter to set
    )
{
    if ( ! rtsp_checksum_ok(h) ) {
        return -1;
    }

    DEBUG( "key=%s value=%s", key, value );
    if ( key == NULL || value == NULL ) return 1;

    rtsp_param_t* p = h->params;
    rtsp_param_t* new_node = rtsp_param_new_node( key, value );

    // Remove any existing entries with same key, effectively
    // overwriting them
    rtsp_param_remove( h, key );

    if ( p == NULL ) {
        h->params = new_node;
        return 0;
    }

    while ( p->next != NULL ) {
        p = p->next;
    }

    p->next = new_node;
    return 0;
}

/**
 * Retrieves the string value of an RTSP value from the RTSP library's
 * internal storage.
 *
 * @returns pointer to a buffer containing the string value of the
 *          parameter
 * @returns NULL if the key does not exist
 */
const char* rtsp_param_get
    ( rtsp_handle h   ///< [in] configured rtsp library handle
    , const char* key ///< [in] key name of the RTSP parameter to retrieve
    )
{
    if ( ! rtsp_checksum_ok(h) ) return NULL;

    const rtsp_param_t* p = rtsp_param_find_node( h, key );

    return p ? p->value : NULL;
}

/**
 * @returns -1 if no connector is attached
 * @returns 0-255 representing a valid wfd_connector_type valid if a connector is attached
 */
static int get_wfd_connector_type
    ( rtsp_handle h             ///< [in] configured rtsp library handle
    )
{
    switch ( h->connector_type ) {
        case RTSP_CONNECTOR_NONE:      return -1;
        case RTSP_CONNECTOR_VGA:       return 0;
        case RTSP_CONNECTOR_HDMI:      return 5;
        case RTSP_CONNECTOR_COMPOSITE: return 2;
        case RTSP_CONNECTOR_DVI:       return 4;
        default:                       return 255;
    }
}

static inline void wfd_connector_type_helper
    ( rtsp_handle h
    , WFD_PARAMETER_STRUCT* p
    )
{
    if ( ! p ) return;

    p->u.connectorType.ucSupported = 1;

    const int conn_type = get_wfd_connector_type( h );
    if ( conn_type == -1 ) {
        p->u.connectorType.ucDisconnected = 1;
    } else {
        p->u.connectorType.ucDisconnected = 0;
        p->u.connectorType.ucType = conn_type;
    }
}

/**
 * If an RTSP session is active, takes the appropriate action for sending a
 * hotplug event to either an RDS or WFD source.
 *
 * @returns 0 on success
 * @returns 2 if RTSP session is not active
 */
static int send_hotplug_to_source
    ( rtsp_handle h             ///< [in] configured rtsp library handle
    )
{
    DEBUG( "entering" );
    if ( ! rtsp_checksum_ok(h) ) return 1;

    if ( ! h->rtsp_session_running ) {
        DEBUG( "no active RTSP session; ignoring" );
        return 2;
    }

    char mime_edid[700]; // Per RDS, max size is 684
    long mime_edid_len = sizeof(mime_edid);
    char cp_spec[50];
    char buf[1000]; // Has to hold MIME EDID plus the other fields

    if ( h->edidlen > 0 ) {
        rtsplib_encodemime( h->rtsp_mode,
                h->hRtspLib, mime_edid, &mime_edid_len,
                h->edid, h->edidlen );
    } else {
        snprintf( mime_edid, sizeof(mime_edid), "none" );
    }

    if ( h->hdcp2_port == 0 || h->connector_type != RTSP_CONNECTOR_HDMI ) {
        snprintf( cp_spec, sizeof(cp_spec), "none" );
    } else {
        snprintf( cp_spec, sizeof(cp_spec), "HDCP2.1 port=%d", h->hdcp2_port );
    }

    if ( h->rtsp_mode == RTSP_MODE_RDS ) {
        snprintf( buf, sizeof(buf),
                "rds_status: busy=%d, display_connected=%d\r\n"
                "rds_display_edid: %s\r\n"
                "rds_content_protection: %s\r\n\r\n",
                h->rtsp_session_running, h->display_connected,
                mime_edid,
                cp_spec );
        rtsplib_setparam( h->rtsp_mode, h->hRtspLib, buf );
	} else if ( h->rtsp_mode == RTSP_MODE_WFD ) {
        WFD_PARAMETER_STRUCT params[3], *p;
        memset( params, 0, sizeof(params) );

        // Set wfd_display_edid
        p = &params[0];
        p->nId = wfd_display_edid;
        p->u.displayEdid.pucEdid = h->edid;
        p->u.displayEdid.usLength = h->edidlen;

        // Set wfd_content_protection
        p++;
        p->nId = wfd_content_protection;
        p->u.contentProtection.fHDCPVersion = 2.1;
        if ( h->edidlen ) {
            p->u.contentProtection.usPort = h->hdcp2_port;
        } else {
            p->u.contentProtection.usPort = 0;
        }

        // Set wfd_connector_type
        p++;
        p->nId = wfd_connector_type;
        wfd_connector_type_helper( h, p );

        // Send all three params at once
        WFD_RTSPSetParamStruct( h->hRtspLib, params, sizeof(params)/sizeof(params[0]) );
    }

    return 0;
}

/**
 * Updates the EDID and connector type stored in the RTSP library with the
 * provided values. These values will then be automatically provided to a
 * source asking for them. If a connection is currnetly active, a hotplug event
 * will be sent.
 *
 * @returns none
 */
void rtsp_set_edid_and_connector
    ( rtsp_handle h                         ///< [in] configured rtsp library handle
    , const uint8_t* newedid                ///< [in] pointer to a binary buffer containing the new EDID
    , int newedidlen                        ///< [in] length of the EDID buffer
    , const rtsp_connector_t connector_type ///< [in] VGA or HDMI
    )
{
    if( ! rtsp_checksum_ok(h) ) return;

    // Set EDID
    if ( newedidlen < 0 ) newedidlen = 0;
    if ( newedidlen > MAX_EDID_LEN ) newedidlen = MAX_EDID_LEN;
    if ( newedid ) {
        memcpy( h->edid, newedid, newedidlen );
    }
    h->edidlen = newedidlen;

    // Set connector and display connected
    h->connector_type = connector_type;
    h->display_connected = ( connector_type != RTSP_CONNECTOR_NONE );

    // Send a hotplug event if we are in an RTSP session
    send_hotplug_to_source( h );
}


static void rtsp_log_hdcp_decl_support()
{
	FILE *fp;

	fp = fopen("/tmp/hdcp_decl_support.txt","w");
	if(fp){
		fprintf(fp,"1");
		fclose(fp);
	}
}
void rtsp_set_hdcp2_port
    ( rtsp_handle h           ///< [in] configured rtsp library handle
    , const unsigned int port ///< [in] HDCP2 port
    )
{
    assert( rtsp_checksum_ok(h) );
    assert( port <= 65535 );

	if(port != 0){
		rtsp_log_hdcp_decl_support();
	}
    h->hdcp2_port = port;
}

/**
 * Helper function to close the RTSP server TCP socket if it is open.
 *
 * @returns none
 */
static void close_socket
    ( rtsp_handle h       ///< [in] configured rtsp library handle
    )
{
    if ( ! rtsp_checksum_ok(h) ) return;

    if ( h->socket ) {
        shutdown( h->socket, SHUT_RDWR );
        close( h->socket );
        DEBUG( "TCP socket closed" );
        h->socket = 0;
        h->socket_connected = 0;
    }
}

/**
* If the rtsp socket is non-block then the connect() will 
* usually return EINPROGRESS at first, for this situation
* we must use the select() to determine the asynchrounous
* TCP connection to finish.
*/
static int wait_async_connect(int fd)
{
	fd_set rset,wset;
	int err,fderr,len;
	struct timeval tv;
	int ret=-1;

	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_SET(fd, &rset);
	FD_SET(fd, &wset);

	tv.tv_sec = 5;
	tv.tv_usec = 0;

	err = select(fd+1,&rset,&wset,NULL,&tv);

	if(err == 0){
		DEBUG(">>> timeout happened\n");
	}
	else if(err < 0){
		DEBUG(">>> error happened,%d\n",errno);
	}
	else{
		if(FD_ISSET(fd,&rset) || FD_ISSET(fd,&wset)){
			len = sizeof(fderr);
			if(getsockopt(fd,SOL_SOCKET,SO_ERROR,&fderr,&len)<0){
				DEBUG(">>> getsockopt error\n");
			}
			else{
				DEBUG(">>> getsockopt: %d\n",fderr);
				ret=0;
			}
		}
		else{
			DEBUG(">>> fd not set\n");
		}
	}

	return ret;
}

/**
 * Opens an outgoing TCP socket to the specified RTSP source IP address and TCP
 * port. Will not block and will return immediately on error.
 *
 * @returns 0 on success
 * @returns 1 on failure
 * @returns -1 if handle is invalid
 */
#ifdef WIFI_DIRECT_JSON
static int collect_socket_open_ok = 0;
static int collect_socket_status_when_exit = 0;
static int collect_socket_timeout = 0;
static int collect_socket_read_error = 0xffff;
static int collect_socket_read_error_code = 0;
#endif

int rtsp_sink_connect_to_source
    ( rtsp_handle h        ///< [in] configured rtsp library handle
    )
{
	int err;
	
    if ( ! rtsp_checksum_ok(h) ) return -1;

    NOTICE( "ip_address=%s, port %d", h->source_ip_address, h->source_port );

    // Open socket if necessary
    if ( ! h->socket ) {
        const int sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        if ( sockfd < 0 ) {
            /*CRITICAL( "error opening socket" );*/
            return 1;
        }

        int current_opts = fcntl( sockfd, F_GETFL, 0 );
        if ( fcntl( sockfd, F_SETFL, current_opts | O_NONBLOCK ) == -1 ) {
            /*CRITICAL( "unable to set socket to non-blocking" );*/
            close( sockfd );
            return 1;
        }


        DEBUG( "socket opened" );
        h->socket = sockfd;
    }

    struct sockaddr_in remote;
    memset( &remote, 0, sizeof(remote) );
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = inet_addr( h->source_ip_address );
    remote.sin_port = htons( h->source_port );

    int rc = connect( h->socket, (struct sockaddr*) &remote, sizeof(struct sockaddr) );
    if ( rc >= 0 ) {
        h->socket_connected = 1;
        NOTICE( "opened TCP connection" );
	#ifdef WIFI_DIRECT_JSON
		collect_socket_open_ok = 1;
	#endif
        return 0;
    } else {
    	err = errno;
		ERROR( "unable to open TCP connection,%d",err );
		if(err == EINPROGRESS){
			//ERROR( "unable to open TCP connection,%d",err );
			if(wait_async_connect(h->socket)==0){
				 h->socket_connected = 1;
				 NOTICE( "opened TCP connection ok" );
			#ifdef WIFI_DIRECT_JSON
				collect_socket_open_ok = 1;
			#endif
				 return 0;
			}
		}
        return 1;
    }
}

/**
 * Helper function that returns the passed in buffer as a MIME-encoded
 * RTSP response.
 *
 * @returns an RTSP library abstraction return code
 */
static long set_parameter_struct_mime_encoded
    ( rtsp_handle h     ///< [in] configured rtsp library handle
    , void* pVoid       ///< [in] pointer to PARAMETER_STRUCT or WFD_PARAMETER_STRUCT
    , uint8_t* buf      ///< [in] pointer to a binary buffer to encode as MIME
    , const long buflen ///< [in] length of the binary buffer to encode
    )
{
    if ( ! rtsp_checksum_ok(h) ) return 1;

    const int rtsp_mode = h->rtsp_mode;
    long encodedlen;
    long* ncount;
    char* szvalue;

    // get length but don't encode
    rtsplib_encodemime( rtsp_mode, h->hRtspLib, NULL, &encodedlen, buf, buflen );
    encodedlen++; // add space for NULL
    DEBUG( "encoded length is %ld (including NULL)", encodedlen );

    if ( rtsp_mode == RTSP_MODE_RDS ) {
        PARAMETER_STRUCT* p = (PARAMETER_STRUCT*)pVoid;
        ncount = (long*)&p->nCount;
        szvalue = p->szValue;
    } else if ( rtsp_mode == RTSP_MODE_WFD ) {
        WFD_PARAMETER_STRUCT* p = (WFD_PARAMETER_STRUCT*)pVoid;
        ncount = &p->nCount;
        szvalue = p->szValue;
    } else {
        CRITICAL( "unsupported rtsp_mode=%d", rtsp_mode );
        return rtsplib_error_code( rtsp_mode, "INSUFFICIENT_BUF" );
    }

    if ( *ncount < encodedlen || NULL == szvalue ) {
        DEBUG( "buffer from RTSP libary is not big enough; requesting a bigger one" );
        *ncount = encodedlen;
        return rtsplib_error_code( rtsp_mode, "INSUFFICIENT_BUF" );
    }

    return rtsplib_encodemime( rtsp_mode, h->hRtspLib, szvalue,
            ncount, buf, buflen );
}

/**
 * Reads specified file into a buffer.  Allocates memory for the buffer
 * and returns a pointer to the buffer and the size of the buffer.
 * Caller is responsible for freeing the buffer. Currently limited to
 * 64kbytes; if the file is bigger than this, the buffer will only
 * contain the first 64kbytes.
 *
 * @returns 0 if the file was read successfully and memory allocated
 * @returns 1 if an error occurred; "buffer" and "buf_len" are undefined
 */
int rtsp_read_file_into_buffer
    ( const char* filepath ///< [in] path name to read
    , uint8_t** buffer     ///< [out] pointer to the buffer containing the file's contents
    , long* buf_len    ///< [out] size of the buffer
    )
{
    FILE* fp;
    int file_length;
    int rc;

    fp = fopen( filepath, "rb" );
    if ( NULL == fp ) {
        ERROR( "Unable to open file %s", filepath );
        return 1;
    }

    // File length
    fseek( fp, 0, SEEK_END );
    file_length = ftell( fp );
    fseek( fp, 0, SEEK_SET );

    if ( file_length > UINT16_MAX ) {
        NOTICE( "warning, file too big, truncating to 64k" );
        file_length = UINT16_MAX;
    }

    *buffer = calloc( file_length, 1 );
    if ( NULL == *buffer ) {
        *buf_len = 0;
        ERROR( "Unable to allocate %d bytes of memory", file_length );
        fclose( fp );
        return 1;
    }

    *buf_len = (uint16_t)file_length;

    // Read file into buffer
    rc = fread( *buffer, 1, file_length, fp );
    if ( rc != file_length ) {
        ERROR( "Read %d bytes but expected %d", rc, file_length );
        fclose( fp );
        free( *buffer );
        return 1;
    }

    fclose( fp );
    return 0;
}

/**
 * A helper function called by @ref rtsp_notification_callback to
 * set the value of a string response with bounds checking.
 *
 * @returns
 */
long rtsp_set_parameter_struct_szvalue
    ( const int rtsp_mode     ///< [in]
    , void* pVoid             ///< [in] PARAMETER_STRUCT or WFD_PARAMETER_STRUCT
    , const char* new_szValue ///< [in]
    )
{
    char* szvalue = NULL;
    long* ncount;

    DEBUG( "new_szvalue=%s", new_szValue );

    if ( pVoid == NULL ) {
        CRITICAL( "%s(%d) passed null pointer p", __FILE__, __LINE__ );
        return rtsplib_error_code( rtsp_mode, "INSUFFICIENT_BUF" );
    }

    if ( rtsp_mode == RTSP_MODE_RDS ) {
        PARAMETER_STRUCT* p = (PARAMETER_STRUCT*)pVoid;
        ncount = (long*)&p->nCount;
        szvalue = p->szValue;
    }

    if ( rtsp_mode == RTSP_MODE_WFD ) {
        WFD_PARAMETER_STRUCT* p = (WFD_PARAMETER_STRUCT*)pVoid;
        ncount = &p->nCount;
        szvalue = p->szValue;
    }

    if ( szvalue == NULL ) {
        CRITICAL( "p->szValue is NULL" );
        return rtsplib_error_code( rtsp_mode, "INSUFFICIENT_BUF" );
    }

    int bytes_needed = strlen(new_szValue) + 1;
    if ( *ncount < bytes_needed ) {
        *ncount = bytes_needed;
        return rtsplib_error_code( rtsp_mode, "INSUFFICIENT_BUF" );
    }

    snprintf( szvalue, *ncount, "%s", new_szValue );
    return rtsplib_error_code( rtsp_mode, "NO_ERROR" );
}


/**
 * A helper function called by @ref rtsp_notification_callback to
 * set the manufacturer's logo bitmap.
 *
 * @returns
 */
static long set_parameter_struct_logo
    ( rtsp_handle h       ///< [in] configured rtsp library handle
    , void* p ///< [in]
    )
{
    if ( ! rtsp_checksum_ok(h) ) return 1;

    const int rtsp_mode = h->rtsp_mode;
    const char* logo_path = h->manufacturer_logo_path;

    uint8_t* rawlogo;
    long rawlogolen;
    long rc;

    if ( logo_path == NULL || strlen(logo_path) == 0 ) {
        return rtsp_set_parameter_struct_szvalue( rtsp_mode, p, "none" );
    }

    // rtsp_read_file_into_buffer() will calloc() memory that needs to be freed!
    rc = rtsp_read_file_into_buffer( logo_path, &rawlogo, &rawlogolen );
    if ( 0 != rc ) {
        ERROR( "error reading file" );
        return rtsp_set_parameter_struct_szvalue( rtsp_mode, p, "none" );
    }

    rc = set_parameter_struct_mime_encoded( h, p, rawlogo, rawlogolen );

    // Free memory allocated by read_file_into_buffer()
    if ( rawlogo ) {
        free( rawlogo );
    }

    return rc;
}

/**
 * Helper function used to read from the socket; will block up to the indicated blocktime_ms.
 *
 * @returns 0 if data successfully read
 * @reutrns 1 if receive timed out without any data
 * @returns -1 on fatal error while reading from socket
 */
static long read_socket
    ( rtsp_handle h       ///< [in] configured rtsp library handle
    , uint8_t* outbuf     ///< [out]
    , long outbufsize     ///< [in]
    , long* bytesread     ///< [out] the number of bytes read goes here
    , int blocktime_ms   ///< [in] maximum time to wait in milliseconds
    )
{
    if ( ! rtsp_checksum_ok(h) ) return 1;

    if ( ! h->socket_connected ) {
        ERROR( "socket not ready" );
        return -1;
    }

    fd_set rfds;
    FD_ZERO( &rfds );
    FD_SET( h->socket, &rfds );

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = blocktime_ms * 1000;

    int rc = select( h->socket+1, &rfds, NULL, NULL, &tv );

    // Check if socket was closed while we were waiting
    if ( ! h->socket_connected || rc == -1 ) {
        printf( "error calling select() on socket" );
        return -1;
    }

    if ( FD_ISSET( h->socket, &rfds) ) {
        FD_CLR( h->socket, &rfds );

        int len = read( h->socket, outbuf, outbufsize-1 );
        if ( len <= 0 ) {
		#ifdef WIFI_DIRECT_JSON
			collect_socket_read_error = len;
			collect_socket_read_error_code = errno;
		#endif

			printf( "unable to read socket: %s,%d", strerror(errno),len);
			close_socket( h );
            return -1;
        }

        outbuf[len] = 0;
		DEBUG( "read: [\n%s]", outbuf );
        *bytesread = len;
        return 0;
	}

    // Timed out but not data received
    return 1;
}

/** Callback used by underlying RTSP library to read the socket. Will
 * automatically open a socket if needed. If there are any errors while opening
 * the socket, or while trying to receive data, will automatically retry up to
 * the nTimeout limit unless a teardown or cancel request is received.
 *
 * @returns RTSP library response code
 */

static long receive
    ( uint8_t* pData    ///< [out]
    , long nBufSize     ///< [in]
    , long* pnBytesRead ///< [out] the number of bytes read goes here
    , long timeout_ms     ///< [in] maximum time to wait in milliseconds
    , void* pContext    ///< [in]
    )
{
    DEBUG( "timeout_ms=%ld", timeout_ms );
    rtsp_handle h = (rtsp_handle)pContext;
    if ( ! rtsp_checksum_ok(h) ) return 1;

    const int step_ms = 200;
    int elapsed_ms = 0;

    do {
        if ( *h->cancel_request || h->teardown_received ) {
			printf(">> Not connect:%d,%d,%d\n",__LINE__,*h->cancel_request,h->teardown_received);
            return rtsplib_error_code( h->rtsp_mode, "SOCKET_NOT_CONNECTED" );
            // socket will be closed by rtsp_run_session() or rtsp_shutdown()
        }

        if ( h->requests->teardown ) {
            // Send TEARDOWN to the source but don't close the socket
            rtsplib_teardown( h->rtsp_mode, h->hRtspLib );
            h->requests->teardown = 0;
        }

        if ( h->requests->pause ) {
            rtsplib_pause( h->rtsp_mode, h->hRtspLib );
            h->requests->pause = 0;
        }

        if ( h->requests->unpause ) {
            rtsplib_play( h->rtsp_mode, h->hRtspLib );
            h->requests->unpause = 0;
        }

        if ( ! h->socket_connected ) {
            int rc = rtsp_sink_connect_to_source( h );
            if ( rc != 0 && elapsed_ms % 1000 == 0 ) {
                // display this message only once per second to reduce log verbosity
                DEBUG( "%d ms elapsed, will keep trying to open socket for %ld ms", elapsed_ms, timeout_ms );
            }
        }

        if ( read_socket( h, pData, nBufSize, pnBytesRead, 0 ) == 0 ) {
             return rtsplib_error_code( h->rtsp_mode, "NO_ERROR" );
        }

        //usleep( step_ms * 1000 );
		rtsp_local_sleep(step_ms);
        elapsed_ms += step_ms;

    } while ( elapsed_ms < timeout_ms );

	if(elapsed_ms >= timeout_ms){
		printf(">> Receive timeout out:elapse=%d,expect=%d\n",elapsed_ms,timeout_ms);
	#ifdef WIFI_DIRECT_JSON
		collect_socket_timeout = 1;
	#endif
	}
	else{
	#ifdef WIFI_DIRECT_JSON
		collect_socket_timeout = 0;
	#endif	
	}

#ifdef WIFI_DIRECT_JSON
	if(h->socket_connected){
		collect_socket_status_when_exit = 1;
	}
	else{
		collect_socket_status_when_exit = 0;
	}
#endif

    if ( h->socket_connected ) {
        return rtsplib_error_code( h->rtsp_mode, "SOCKET_RECV_TIMEOUT" );
    } else {
    	printf(">> Not connect:%d,%d,%d\n",__LINE__,*h->cancel_request,h->teardown_received);
        return rtsplib_error_code( h->rtsp_mode, "SOCKET_NOT_CONNECTED" );
    }
}

/**
 * TODO
 *
 * @returns 0 if data successfully sent
 * @reutrns 1 if timed out waiting for socket
 * @returns -1 on fatal error while writing socket
 */
static long write_socket
    ( rtsp_handle h      ///< [in] configured rtsp library handle
    , uint8_t* buf       ///< [in] pointer to buffer to write to socket
    , long bufsize       ///< [in] number of bytes to write from buffer
    , int blocktime_ms   ///< [in] maximum time to wait in milliseconds for entire packet to transmit
    )
{
    DEBUG( "entering with %ld bytes to send in %d ms: [\n%s]", bufsize, blocktime_ms, buf );
    if ( ! rtsp_checksum_ok(h) ) return 1;

    if ( ! h->socket_connected ) {
        DEBUG( "socket is closed; abandoning write attempt" );
        return -1;
    }

    const int step_ms = 200;
    int buf_remaining = bufsize;
    do {
        // Wait for socket to become ready for writing
        fd_set wfds;
        FD_ZERO( &wfds );
        FD_SET( h->socket, &wfds );

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = step_ms * 1000;

        int rc = select( h->socket+1, NULL, &wfds, NULL, &tv );

        // Check if socket was closed while we were waiting
        if ( rc == -1 || ! h->socket_connected ) {
            return -1;
        }

        if ( FD_ISSET( h->socket, &wfds ) ) {
            ssize_t bytes_written = write( h->socket, buf, buf_remaining );

            if ( bytes_written < 0 ) {
                ERROR( "unable to write socket: %s", strerror(errno) );
                close_socket( h );
                return -1;
            }

            buf += bytes_written;
            buf_remaining -= bytes_written;
            DEBUG( "wrote %d bytes, %d remaining", bytes_written, buf_remaining );
        }

        blocktime_ms -= step_ms;
        DEBUG( "buf_remaining=%d, time_remaining_ms=%d", buf_remaining, blocktime_ms );
    } while ( buf_remaining > 0 && blocktime_ms > 0 );

    if ( buf_remaining > 0 ) {
        ERROR( "timed out with %d bytes remaining to write", buf_remaining );
        return 1;
    }

    return 0;
}

static long transmit
    ( uint8_t* pData      ///< [in]
    , const long nBufSize ///< [in]
    , void* pContext      ///< [in]
    )
{
    //DEBUG( "entering with %ld bytes to send: [%s]", nBufSize, pData );

    rtsp_handle h = (rtsp_handle)pContext;
    if ( ! rtsp_checksum_ok(h) ) return 1;

    const int timeout_ms = 5000;
    const int step_ms = 200;
    int time_left_ms = timeout_ms;

    do {
        if ( *h->cancel_request || h->teardown_received ) {
			printf(">> Not connect:%d,%d,%d\n",__LINE__,*h->cancel_request,h->teardown_received);
            return rtsplib_error_code( h->rtsp_mode, "SOCKET_NOT_CONNECTED" );
            // socket will be closed by rtsp_run_session() or rtsp_shutdown()
        }

        if ( ! h->socket_connected ) {
            if ( h->teardown_sent ) {
                // Handle the condition where the sink wants to send a TEARDOWN
                // but the socket is in an error state. In this case, we want
                // to exit as quickly as possible and not try and send the
                // TEARDOWN
                printf(">> Not connect:%d,%d,%d\n",__LINE__,*h->cancel_request,h->teardown_received);
                return rtsplib_error_code( h->rtsp_mode, "SOCKET_NOT_CONNECTED" );
            }

            rtsp_sink_connect_to_source( h );
        }

        if ( write_socket( h, pData, nBufSize, time_left_ms ) == 0 ) {
             return rtsplib_error_code( h->rtsp_mode, "NO_ERROR" );
        }

        //usleep( step_ms * 1000 );
        rtsp_local_sleep(step_ms);
        time_left_ms -= step_ms;
    } while ( time_left_ms > 0 );

    if ( h->socket_connected ) {
        return rtsplib_error_code( h->rtsp_mode, "SOCKET_RECV_TIMEOUT" );
    } else {
    	printf(">> Not connect:%d,%d,%d\n",__LINE__,*h->cancel_request,h->teardown_received);
        return rtsplib_error_code( h->rtsp_mode, "SOCKET_NOT_CONNECTED" );
    }
}

/**
 * TODO
 *
 * @returns
 */
static void sleep_callback
    ( const int nMsec ///< [in]
    , void* context   ///< [in]
    )
{
    DEBUG( "nMsec=%d", nMsec );
	//usleep( nMsec * 1000 );
	rtsp_local_sleep(nMsec);
}

/**
 * Callback function for use with RTSP library.
 *
 * @returns current time in milliseconds
 */
static unsigned long get_tick_callback
    ( void* context ///< [in]
    )
{
    struct timeval tv;
    int rc;

    /*DEBUG( "entering" );*/

    rc = gettimeofday( &tv, NULL );
    if ( -1 == rc ){
        CRITICAL( "gettimeofday() failed" );
        exit( 1 );
    }

    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

/**
 * TODO
 *
 * @returns
 */
int rtsp_version_string_to_struct
    ( VERSION_STRUCT* verbuf ///< [out]
    , const char* string     ///< [in]
    )
{
    int i;
    const char* sep = ".";
    char* token;
    char temp[100];
    uint8_t* buf = (uint8_t*)verbuf;

    snprintf( temp, sizeof(temp), "%s", string );
    token = strtok( temp, sep );
    for ( i=0; token && i<sizeof(VERSION_STRUCT); i++ ) {
        buf[i] = (uint8_t)atoi( token );
        token = strtok( NULL, sep );
    }

    return i == sizeof(VERSION_STRUCT);
}

/**
 * TODO
 *
 * @returns
 */
static void log_callback
    ( const int nLevel ///< [in]
    , char* szString   ///< [in]
    , void* pContext   ///< [in]
    )
{
    syslog( nLevel, "%s", szString );
}

/**
 * TODO
 *
 * @returns
 */
/** Returns -1 if unable to handle request */
static long get_parameter_rds
    ( rtsp_handle h       ///< [in] configured rtsp library handle
    , PARAMETER_STRUCT* p ///< [in]
    )
{
    if ( ! rtsp_checksum_ok(h) ) return 1;

    if ( h->rtsp_mode != RTSP_MODE_RDS ) {
        CRITICAL( "calling %s with rtsp_mode != RTSP_MODE_RDS", __func__ );
        return 1;
    }

    const int rtsp_mode = RTSP_MODE_RDS;
    const int param_id = p->nId;
    char* param_name = NULL;
    const long no_error = rtsplib_error_code( rtsp_mode, "NO_ERROR" );

    param_name = rtsplib_param_is_vendor_extension( rtsp_mode, param_id ) ?
        p->szValue : (char*)rtsplib_param_name( rtsp_mode, param_id );

    DEBUG( "param_id=%d param_name=%s", param_id, param_name );

    // See if this is a string value in the internal parameter list
    {
        const char* val = rtsp_param_get( h, param_name );
        if ( val ) {
            return rtsp_set_parameter_struct_szvalue( rtsp_mode, p, val );
        }
    }
    // Automatically handle requests for parameters stored in h
    switch ( param_id ) {
        case rds_status:
			p->u.status.nBusy = h->rtsp_session_running;
			p->u.status.nDisplayConnected = h->display_connected;
            return no_error;

        case rds_display_edid:
            if ( h->edidlen == 0 ) {
                return rtsp_set_parameter_struct_szvalue( rtsp_mode, p, "none" );
            } else {
                return set_parameter_struct_mime_encoded( h, p,
                        h->edid, h->edidlen );
            }

        case rds_audio_formats:
            return rtsp_set_parameter_struct_szvalue( rtsp_mode, p, "deprecated" );

        case rds_video_formats:
            return rtsp_set_parameter_struct_szvalue( rtsp_mode, p, "deprecated" );

        case rds_rtp_profile:
            p->u.profile = RTP_AVPF;
            return no_error;

        case rds_sink_manufacturer_logo:
            return set_parameter_struct_logo( h, p );

        case rds_sink_version:
            snprintf( p->u.version.szProductId, RDS_PRODUCT_ID_LEN, "%s",
                      h->product_id );
            p->u.version.verHardware = h->hardware_version;
            p->u.version.verSoftware = h->software_version;
            return no_error;

        case rds_content_protection:
            p->u.contentProtection.fHDCPVersion = 2.0;
			// Disable the hdcp function
            //p->u.contentProtection.usPort = h->hdcp2_port;
            p->u.contentProtection.usPort = 0;
            return no_error;

        case rds_fast_cursor_disabled:
            return rtsp_set_parameter_struct_szvalue( rtsp_mode, p, "0" );

        default:
            // If we make it this far, it means that we couldn't handle the
            // request ourselves; need to call the registered callback handler
            // to handle it for us
            DEBUG( "primary handler couldn't handle %s, passing to callback handler", param_name );
            return -1;

    }
}

static long set_wfd_parameter_video_format
    ( rtsp_handle h           ///< [in] configured rtsp library handle
    , WFD_PARAMETER_STRUCT* p ///< [in]
    , const int is_3d         ///< [in] 0=2D, 1=3D
    )
{
    DEBUG( "entering" );

    const int num_profiles = 1;
    const int space_needed = num_profiles * sizeof( WFD_H264_PROFILE );

    if ( p->nCount < space_needed ) {
        p->nCount = space_needed;
        return WFD_RTSP_INSUFFICIENT_BUF;
    }

	/**
	* native resolution and corresponding refresh rates.
	* see table 5-13
	*/
    //p->u.videoFormat.native = 0; // CEA 640x480p60
    p->u.videoFormat.native = 0x40; // CEA 1280x720p60
	
	/**
	* we prefer 1280*720.
	*/
    p->u.videoFormat.preferred_display_mode_supported = 0;
    p->u.videoFormat.num_profiles = num_profiles;

    WFD_H264_PROFILE* vfp = &p->u.videoFormat.aProfiles[0];

    vfp->level = 0x2;   // constrained baseline profile
    vfp->profile = 0x1; // H.264 Level 3.1
    vfp->max_hres = 1920;
    vfp->max_vres = 1080;
    vfp->misc_params.latency = 0;
    vfp->misc_params.min_slice_size = 0;
    vfp->misc_params.slice_enc_params = 0; // 16-1 = 0xf
    vfp->misc_params.frame_rate_control_support = 0x11; // not supported

    // By default set supported to none
    p->u.videoFormat.supported = 0;

    if ( is_3d ) {
        if ( h->s3d_support ) {
            p->u.videoFormat.supported = 1;
            vfp->misc_params.S3D_Capability = h->s3d_support;
        }
    } else { // 2D
        if ( h->cea_support | h->vesa_support | h->hh_support ) {
            p->u.videoFormat.supported = 1;
            vfp->misc_params.CEA_Support = h->cea_support;
            vfp->misc_params.VESA_Support = h->vesa_support;
            vfp->misc_params.HH_Support = h->hh_support;
        }
    }

    return rtsplib_error_code( h->rtsp_mode, "NO_ERROR" );
}

static long set_wfd_parameter_audio_codecs
    ( rtsp_handle h           ///< [in] configured rtsp library handle
    , WFD_PARAMETER_STRUCT* p ///< [in]
    )
{
    DEBUG( "entering" );

    int num_codecs = 0;
    if ( h->lpcm_support ) num_codecs++;
    if ( h->aac_support ) num_codecs++;
    if ( h->ac3_support ) num_codecs++;

    const int space_needed = num_codecs * sizeof( WFD_AUDIO_CODEC );
    if ( p->nCount < space_needed ) {
        p->nCount = space_needed;
        return WFD_RTSP_INSUFFICIENT_BUF;
    }

    p->u.audioCodecs.num_codecs = num_codecs;
    p->u.audioCodecs.supported = 1;

    WFD_AUDIO_CODEC* acp = &p->u.audioCodecs.aCodecs[0];

    if ( h->lpcm_support ) {
        acp->audio_format = WFD_LPCM;
        acp->mode = h->lpcm_support;
        acp->latency = 0;
    }

    if ( h->aac_support ) {
        acp++;
        acp->audio_format = WFD_AAC;
        acp->mode = h->aac_support;
        acp->latency = 0;
    }

    if ( h->ac3_support ) {
        acp++;
        acp->audio_format = WFD_AC3;
        acp->mode = h->ac3_support;
        acp->latency = 0;
    }

    return rtsplib_error_code( h->rtsp_mode, "NO_ERROR" );
}

/**
 * TODO
 *
 * @returns -1 if unable to handle request
 *
 */
static long get_parameter_wfd
    ( rtsp_handle h           ///< [in] configured rtsp library handle
    , WFD_PARAMETER_STRUCT* p ///< [in]
    )
{
    if ( ! rtsp_checksum_ok(h) ) return 1;

    const int rtsp_mode = h->rtsp_mode;
    const int param_id = p->nId;
    char* param_name = NULL;
    const long no_error = rtsplib_error_code( rtsp_mode, "NO_ERROR" );

    param_name = rtsplib_param_is_vendor_extension( rtsp_mode, param_id ) ?
        p->szParam : (char*)rtsplib_param_name( rtsp_mode, param_id );

    DEBUG( "param_id=%d param_name=%s", param_id, param_name );

    // See if this is a string value in the internal parameter list
        const char* val = rtsp_param_get( h, param_name );
        if ( val ) {
            return rtsp_set_parameter_struct_szvalue( rtsp_mode, p, val );
        }

    // handle logo
    if ( strcmp( param_name, "intel_sink_manufacturer_logo" ) == 0 ) {
        return set_parameter_struct_logo( h, p );
    }

    switch ( param_id ) {
        case wfd_display_edid:
            if ( p->nCount < h->edidlen ) {
                p->nCount = h->edidlen;
                return WFD_RTSP_INSUFFICIENT_BUF;
            } else {
                memcpy( p->u.displayEdid.pucEdid, h->edid, h->edidlen );
                p->u.displayEdid.usLength = h->edidlen;
                return no_error;
            }

        case wfd_audio_codecs:
            return set_wfd_parameter_audio_codecs( h, p );

        case wfd_video_formats:
            return set_wfd_parameter_video_format( h, p, 0 );

        case wfd_3d_video_formats:
            return set_wfd_parameter_video_format( h, p, 1 );

        case wfd_content_protection:
            p->u.contentProtection.fHDCPVersion = 2.1;
			// Disable the hdcp function
            p->u.contentProtection.usPort = h->hdcp2_port;
            //p->u.contentProtection.usPort = 0;
            return no_error;

        case wfd_client_rtp_ports:
            p->u.clientPorts.eMode = WFD_RTP_PLAY;
            p->u.clientPorts.usPort0 = h->rtp_port;
            p->u.clientPorts.usPort1 = 0;
            return rtsp_set_parameter_struct_szvalue( rtsp_mode, p, "RTP/AVP/UDP;unicast" );

        case wfd_uibc_capability:
			memset( p->u.uibcCapability.aucCategory, 0, CountOfWFD_InputCategory );
			memset( p->u.uibcCapability.aucType, 0, CountOfWFD_InputType );
			p->u.uibcCapability.usCountOfCapPairs = 0;
			p->u.uibcCapability.usPort = 0;
            return no_error;

        case wfd_connector_type:
            wfd_connector_type_helper( h, p );
            return no_error;

        default:
            // If we make it this far, it means that we couldn't handle the
            // request ourselves; need to call the registered callback handler
            // to handle it for us
            DEBUG( "primary handler couldn't handle %s, passing to callback handler", param_name );
            return -1;
    }
}

/**
 * Handles GET PARAMETER requests from the RTSP source. Implemented as a
 * wrapper that calls the underlying RDS or WFD versions of the RTSP
 * library as appropriate.
 *
 * @returns passes through the return code from get_parameter_rds() or
 *          get_parameter_wfd()
 */
static long get_parameter_handler
    ( rtsp_handle h ///< [in] configured rtsp library handle
    , void* p       ///< [in] pointer to the RTSP library parameter struct, either a PARAMETER_STRUCT* or a WFD_PARAMETER_STRUCT*
    )
{
    if ( ! rtsp_checksum_ok(h) ) return 1;

    const int rtsp_mode = h->rtsp_mode;
    long rc = -1;

    if ( rtsp_mode == RTSP_MODE_RDS ) {
        rc = get_parameter_rds( h, (PARAMETER_STRUCT*)p );
    } else if ( rtsp_mode == RTSP_MODE_WFD ) {
        rc = get_parameter_wfd( h, (WFD_PARAMETER_STRUCT*)p );
    } else {
        DEBUG( "unsupported rtsp_mode, shouldn't get here" );
        assert( 0 );
    }

    return rc;
}

/**
 * TODO
 *
 * @returns
 */
static long set_parameter_rds
    ( rtsp_handle h           ///< [in] configured rtsp library handle
    , PARAMETER_STRUCT* p     ///< [in]
    , RDSRTSPNotifications id ///< [in]
    )
{
    if ( ! rtsp_checksum_ok(h) ) return 1;

    const int rtsp_mode = h->rtsp_mode;
    const int param_id = p->nId;
    const long no_error = rtsplib_error_code( rtsp_mode, "NO_ERROR" );
    const char* param_name = rtsplib_param_name( rtsp_mode, param_id );

    DEBUG( "param=%s", param_name );

    switch ( param_id ) {
        case rds_trigger_method:
        case rds_presentation_URL:
        case rds_rtp_profile_setting:
        case rds_keepalive:
            // These are automatically handled by the RTSPLIB
            return no_error;

        // Affinity handshake
        case rds_init_intel_hdshk:
        case rds_cfrm_intel_hdshk:
            return rtsp_affinity_check( h, p );

        default:
            // If we make it this far, it means that we couldn't handle the
            // request ourselves; need to call the registered callback handler
            // to handle it for us
            DEBUG( "primary handler couldn't handle %s, passing to callback handler", param_name );
            return -1;
    }
}

/**
 * TODO
 *
 * @returns
 */
static long set_parameter_wfd
    ( rtsp_handle h           ///< [in] configured rtsp library handle
    , WFD_PARAMETER_STRUCT* p ///< [in]
    , WFD_RTSPNotifications id ///< [in]
    )
{
    if ( ! rtsp_checksum_ok(h) ) return 1;

    const int rtsp_mode = h->rtsp_mode;
    const int param_id = p->nId;
    const char* param_name = rtsplib_param_name( rtsp_mode, param_id );
    const long no_error = rtsplib_error_code( rtsp_mode, "NO_ERROR" );

    DEBUG( "param_name=%s", param_name );

    switch ( param_id ) {
        case wfd_trigger_method:
            // These are automatically handled by the RTSPLIB
            return no_error;

        default:
            // If we make it this far, it means that we couldn't handle the
            // request ourselves; need to call the registered callback handler
            // to handle it for us
            DEBUG( "primary handler couldn't handle %s, passing to callback handler", param_name );
            return -1; // returning -1 instructs the caller to fall back to secondary handler
    }
}

/**
 * Handles SET PARAMETER requests from the RTSP source. Implemented as a
 * wrapper that calls the underlying RDS or WFD versions of the RTSP
 * library as appropriate.
 *
 * @returns
 */
static long set_parameter_handler
    ( rtsp_handle h ///< [in] configured rtsp library handle
    , void* p       ///< [in] pointer to the RTSP library parameter struct, either a PARAMETER_STRUCT* or a WFD_PARAMETER_STRUCT*
    , long id       ///< [in]
    )
{
    if ( ! rtsp_checksum_ok(h) ) return 1;

    const int rtsp_mode = h->rtsp_mode;
    long rc = -1;

    if ( rtsp_mode == RTSP_MODE_RDS ) {
        rc = set_parameter_rds( h, (PARAMETER_STRUCT*)p, id );
    } else if ( rtsp_mode == RTSP_MODE_WFD ) {
        rc = set_parameter_wfd( h, (WFD_PARAMETER_STRUCT*)p, id );
    } else {
        DEBUG( "unsupported rtsp_mode, shouldn't get here" );
        assert( 0 );
    }

    return rc;
}

/**
 * The RTSP library calls this function first whenever a notification
 * arrives. For GET PARAMETER and SET PARAMETER requests, separate
 * handlers are invoked to try and handle the request internally by the
 * library; if these handlers return -1, then the RTSP library invokes
 * the callback function that the caller provided to rtsp_init() and
 * passes the notification up the stack. TEARDOWN and TIMEOUT requests
 * are handled automatically by the library.
 *
 * @returns
 */
static long notification_handler
    ( const int id   ///< [in]
    , void* szValue  ///< [in]
    , void* pContext ///< [in]
    )
{
    rtsp_handle h = (rtsp_handle)pContext;
    if ( ! rtsp_checksum_ok(h) ) return 1;

    const int rtsp_mode = h->rtsp_mode;
    const char* notif_name = rtsplib_notification_name( rtsp_mode, id );
    const long no_error = rtsplib_error_code( rtsp_mode, "NO_ERROR" );
    long rc = -1;

    DEBUG( "notif_id=%d notif_name=%s", id, notif_name );

    if ( id == rtsplib_notification_code( rtsp_mode, "GET_PARAMETER" ) ) {
        rc = get_parameter_handler( h, szValue );
    }

    if ( id == rtsplib_notification_code( rtsp_mode, "GET_PARAMETER_RESP" )
            || id == rtsplib_notification_code( rtsp_mode, "SET_PARAMETER" ) ) {
        rc = set_parameter_handler( h, szValue, id );
    }

    if ( id == rtsplib_notification_code( rtsp_mode, "TEARDOWN" ) ) {
        h->teardown_received = 1;
        rc = no_error;
    }

    if ( id == rtsplib_notification_code( rtsp_mode, "TIMEOUT" ) ) {
        rc = no_error;
    }

    if ( rc == -1 ) {
        return h->rtsp_notification_callback( id, szValue, h );
    } else {
        return rc;
    }
}

/**
 * Initializes RTSP library for use. Must be called before calling any other
 * functions in the RTSP libary. Returns a handle that must be passed as the
 * first argument to all other functions in the libary.
 *
 * @returns NULL if an error occured
 * @returns handle if library successfully initialized
 * @returns -2 if one or more parameters incorrect
 */
int rtsp_init
    ( rtsp_handle h                          ///< [out] rtsp library handle to initialize
    , const int rtsp_mode                    ///< [in] RTSP_MODE_RDS or RTSP_MODE_WFD
    , const unsigned int rtp_port            ///< [in] RTP port to receive data plane; must be 1-65535
    , rtsp_fn_notification_callback callback ///< [in]
    , void* caller_context
    )
{
    if ( h->checksum == RTSP_CONFIG_CHECKSUM && ! h->library_is_shutdown ) {
        DEBUG( "library is already initialized; calling twice by accident?" );
    }

    if ( rtsp_mode != RTSP_MODE_RDS && rtsp_mode != RTSP_MODE_WFD ) {
        assert( 0 );
        return -2;
    }

    if ( rtp_port <= 0 || rtp_port >= 65535 ) {
        assert( 0 );
        return -2;
    }

    if ( ! callback ) {
        assert( 0 );
        return -2;
    }

    memset( h, 0, sizeof(rtsp_config_t) );

    h->checksum = RTSP_CONFIG_CHECKSUM;

    h->rtsp_mode = rtsp_mode;
    h->rtp_port = rtp_port;
    h->rtsp_notification_callback = callback;
    h->caller_context = caller_context;

    if ( rtsp_mode == RTSP_MODE_RDS ) {
        rtsp_param_set( h, "rds_friendly_name", "Friendly Name" );
        rtsp_param_set( h, "rds_unique_device_id", "11223344" );
        rtsp_param_set( h, "rds_sink_manufacturer_name", "Manuf Name" );
        rtsp_param_set( h, "rds_sink_model_name", "Model Name" );
    } else if ( rtsp_mode == RTSP_MODE_WFD ) {
        rtsp_param_set( h, "intel_friendly_name", "Friendly Name" );
        rtsp_param_set( h, "intel_unique_device_id", "11223344" );
        rtsp_param_set( h, "intel_sink_manufacturer_name", "Manuf Name" );
        rtsp_param_set( h, "intel_sink_model_name", "Model Name" );
    } else {
        assert( 0 );
    }

    // Set params for rds_sink_version - this only applies to RDS mode
    // As of 2011-08-24, hardware and software versions have to be at
    // least 2.1.0.0 to enable certain features on WiDiApp, such as the
    // fast cursor
    rtsp_version_string_to_struct( &h->hardware_version, "2.1.0.0" );
    rtsp_version_string_to_struct( &h->software_version, "2.1.0.0" );
    snprintf( h->product_id, RDS_PRODUCT_ID_BUF_LEN, "%s", "ProductID" );

	long rc = rtsplib_snkinit( h->rtsp_mode, &h->hRtspLib, h->rtp_port,
            notification_handler, receive, transmit,
            sleep_callback, get_tick_callback, log_callback, h );

	if ( rc != rtsplib_error_code( h->rtsp_mode, "NO_ERROR" ) ) {
		CRITICAL( "rtsplib_snkinit failed with error code %ld", rc );
        return 1;
    }

    NOTICE( "rtspsink library initialized" );
    return 0;
}

/**
 * TODO
 *
 * @returns
 */
int rtsp_shutdown
    ( rtsp_handle h  ///< [in] configured rtsp library handle
    )
{
    DEBUG( "entering" );
    if ( ! rtsp_checksum_ok(h) ) {
        assert( 0 );
        return -1;
    }

    if ( h->library_is_shutdown ) {
        DEBUG( "library is already shutdown; ignoring" );
        return 0;
    }

    close_socket( h );

    rtsplib_shutdown( h->rtsp_mode, h->hRtspLib );
    h->hRtspLib = NULL;

    rtsp_param_free_all( h );

    h->library_is_shutdown = 1;

    DEBUG( "RTSP library has been shut down" );

    return 0;
}

int rtsp_sink_teardown
    ( rtsp_handle h       ///< [in] configured rtsp library handle
    )
{
    DEBUG( "entering" );
    if ( ! rtsp_checksum_ok(h) ) return 1;

    if ( ! h->rtsp_session_running ) {
        DEBUG( "no active RTSP session; ignoring" );
        return 2;
    }

    h->teardown_sent = 1;
    rtsplib_teardown( h->rtsp_mode, h->hRtspLib );

    return 0;
}

/**
 * Starts the RTSP engine with the specified config settings.
 * cancel_request is a pointer to an integer; the RTSP engine will stop
 * if this value changes to a non-zero value.
 *
 * @returns 0 if the RTSP session ended gracefully (with a TEARDOWN)
 * @returns -1 if the RTSP session ended due to a cancel request
 * @returns >0 if the RTSP library exited with an error code
 */
int rtsp_sink_run_session
    ( rtsp_handle h       ///< [in] configured rtsp library handle
    , const char* ip_address
    , const unsigned int port
    , int* cancel_request ///< [in]
    , rtsp_request_t* requests
    )
{
    assert( rtsp_checksum_ok(h) );
    assert( ip_address );
    assert( strlen(ip_address) > strlen("1.1.1.1") );
	
#ifdef WIFI_DIRECT_JSON
	FILE *fd = NULL;
	collect_socket_open_ok = 0;
	collect_socket_status_when_exit = 0;
	collect_socket_timeout = 0;
	collect_socket_read_error = 0xffff;
	collect_socket_read_error_code = 0;
#endif

    if ( port == 0 ) {
        ERROR( "RTSP source port cannot be 0" );
        return 1;
    }

    // Set the IP address and port here for the RTSP source
    // The receive() and transmit() callbacks will automatically open the socket
    snprintf( h->source_ip_address, sizeof(h->source_ip_address), "%s", ip_address );
    h->source_port = port;
    h->cancel_request = cancel_request;
    h->requests = requests;

    long rc;
    const long no_error = rtsplib_error_code( h->rtsp_mode, "NO_ERROR" );

    h->rtsp_session_running = 1;

    do {
        rc = rtsplib_parser( h->rtsp_mode, h->hRtspLib );
        DEBUG( "rtsplib_parser returned %ld", rc );
    //} while ( rc == no_error && !h->teardown_received && !*h->cancel_request );
    } while ( (rc == no_error && !h->teardown_received));
	
#ifdef WIFI_DIRECT_JSON
	fd = fopen("/tmp/rc_out.txt","w");
	if(fd!=NULL){
		fprintf(fd,"RC:%d,TEARDOWN:%d,RTSP_STATE:%d,SOCKET_OPEN:%s,SOCKET_EXIT_STATUS:%s,SOCKET_TIMEOUT:%s,SOCKET_READ_ERROR:%d,SOCKET_READ_ERROR_CODE:%d",\
			rc,\
			h->teardown_received,\
			rtsplib_get_state(h->rtsp_mode, h->hRtspLib),\
			collect_socket_open_ok?"Yes":"No",\
			collect_socket_status_when_exit?"Connect":"Disconnect",\
			collect_socket_timeout?"Yes":"No",\
			collect_socket_read_error,\
			collect_socket_read_error_code);
		fflush(fd);
		fsync(fileno(fd));
		fclose(fd);
	}
	else{
		printf("open /tmp/rc_out.txt error!\n");
	}
#endif

	printf(">>>>> out rc=%d,teardown=%d,state=%d\n",rc,h->teardown_received,rtsplib_get_state(h->rtsp_mode, h->hRtspLib));

    h->rtsp_session_running = 0;

    if ( h->teardown_received ) {
        printf( "RTSP teardown received" );
        rc = 0;
    } else if ( h->cancel_request ) {
        printf( "cancel request received" );
        rc = -1;
    } else {
        printf( "rtsplib_parser() exited with return code %ld", rc );
    }

    close_socket( h );

    DEBUG( "leaving, rc=%ld", rc );
    return rc;
}

/**
 * Use this function to determine if an RTSP session is currenly active.
 *
 * @returns 1 if RTSP libary is configured and session is running with a source
 * @returns 0 if RTSP libary is not configured or session is not running
 */
int rtsp_sink_session_running
    ( rtsp_handle h  ///< [in] configured rtsp library handle as returned by rtsp_init()
    )
{
    if ( ! rtsp_checksum_ok(h) ) {
        return 0;
    }

    return h->rtsp_session_running;
}

/**
 * Updates the values for CEA, VESA, HH, and S3D video formats by logical ORing
 * the existing values with the values provided. If the 'reset' parameter is
 * set to 1, the existing values are set to zero first.
 *
 * @returns 0 on success
 * @returns 1 if the rtsp_handle is invalid
 */
int rtsp_set_video_formats
    ( rtsp_handle h  ///< [in] configured rtsp library handle as returned by rtsp_init()
    , const uint32_t cea_support
    , const uint32_t vesa_support
    , const uint32_t hh_support
    , const uint64_t s3d_support
    , const int reset ///< [in] 0=do not reset existing values, 1=reset existing values to 0 first
    )
{
    DEBUG( "entering" );
    if ( ! rtsp_checksum_ok(h) ) {
        assert( 0 );
        return 1;
    }

    if ( reset ) {
        h->cea_support = 0;
        h->vesa_support = 0;
        h->hh_support = 0;
        h->s3d_support = 0;
    }

    h->cea_support |= cea_support;
    h->vesa_support |= vesa_support;
    h->hh_support |= hh_support;
    h->s3d_support |= s3d_support;

    return 0;
}

/**
 * Updates the values for LPCM, AAC, and AC3 audio formats by logical ORing
 * the existing values with the values provided. If the 'reset' parameter is
 * set to 1, the existing values are set to zero first.
 *
 * @returns 0 on success
 * @returns 1 if the rtsp_handle is invalid
 */
int rtsp_set_audio_codecs
    ( rtsp_handle h  ///< [in] configured rtsp library handle as returned by rtsp_init()
    , const uint32_t lpcm_support
    , const uint32_t aac_support
    , const uint32_t ac3_support
    , const int reset ///< [in] 0=do not reset existing values, 1=reset existing values to 0 first
    )
{
    DEBUG( "entering" );
    if ( ! rtsp_checksum_ok(h) ) {
        assert( 0 );
        return 1;
    }

    if ( reset ) {
        h->lpcm_support = 0;
        h->aac_support = 0;
        h->ac3_support = 0;
    }

    h->lpcm_support |= lpcm_support;
    h->aac_support |= aac_support;
    h->ac3_support |= ac3_support;

    return 0;
}
