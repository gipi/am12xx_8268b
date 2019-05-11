/**
 * @file sample_rtsp_sink.c
 *
 * A simple wrapper for the sample rtsp sink code.  The code is broken
 * out into two separate files so that the main code logic can be easily
 * linked to another executable.
 *
 * INTEL CONFIDENTIAL
 * Copyright 2010-2011 Intel Corporation All Rights Reserved.
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
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>     // for sleep
#include <stdlib.h>     // for exit
#include <stdint.h>     // for uint8_t
#include <pthread.h>    // For threads
#include <sys/types.h>  // Needed for compiling on Android
#include <sys/socket.h> // Needed for compiling on Android
#include <signal.h>     // for pthread_kill, SIGHUP
#include <assert.h>     // for pthread_kill, SIGHUP
#include <stdarg.h>
#include <sys/stat.h>   // for mkfifo()
#include <fcntl.h>      // for open()
#include <errno.h>      // for ERRNO

#include "librtspsink.h"
#include "affinity.h"

#include <sys/socket.h>
#include <sys/un.h>
#include "mconfig.h"
// }}}

// {{{ Debug helpers
#define DEBUG(...)   logger( __func__, __LINE__, LOG_DEBUG, __VA_ARGS__ )  ///< Messages used for development; they aren't shown for production builds
#define LOGNAME       __FILE__ ": "                             ///< Label prefixed to all DEBUG, NOTICE, ERROR, and CRITICAL calls
#define NOTICE(...)   syslog( LOG_NOTICE, LOGNAME __VA_ARGS__ ) ///< Informational messages that do not indicate an abnormal situation
#define ERROR(...)    syslog( LOG_ERR, LOGNAME __VA_ARGS__ )    ///< Messages that indicate an abnormal situation, but program operation can continue
#define CRITICAL(...) logger( __func__, __LINE__, LOG_CRIT, __VA_ARGS__ )  ///< Messages that indicate a severe situation where normal program operation cannot continue
#define MIRACAST_HDCPDISABLE_PATH	"tmp/MIRACASTDISABLE"

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
    snprintf( new_format, sizeof(new_format), "%s:%d: %s: %s", __FILE__, line, func, format );

    va_list args;
    va_start( args, format );
    vsyslog( priority, new_format, args );
    va_end( args );
}
// }}}

#ifndef THREAD_DATA_T
#define THREAD_DATA_T
typedef struct {
    pthread_t    thread_id;      // is set to 0 if thread is not running
    int          cancel_request; // set to 1 to request thread to exit
    void*        data;
} thread_data_t;
typedef void* (*fn_thread)( void* voidp );
#endif

typedef struct {
    char           source_ip_address[20];
    uint16_t       source_port;
    char*          player_start_script;
    char*          player_stop_script;
    uint16_t       rtp_port;
    int            rtsp_mode;
    int            use_sample_edid;
    int            player_paused;
    char*          named_pipe_filename;
    rtsp_config_t* rtsp_config;
} settings_t;


enum{
	MIRACAST_CMD_PLAY=0,
	MIRACAST_CMD_PAUSE,
	MIRACAST_CMD_STOP,
	MIRACAST_CMD_P2P_FAILED
	
};

typedef struct miracast_cmd_s miracast_cmd_t;
struct miracast_cmd_s
{
	int cmd;
	int param;
};

static int message_channel_fd=-1;
pid_t p2p_pid=-1;
void start_thread( thread_data_t* thread_data, fn_thread thread_func )
{
    DEBUG( "entering" );
    pthread_create( &thread_data->thread_id, NULL, thread_func, thread_data->data );
}

static void send_out_to_player()
{
	miracast_cmd_t cmd;
	
	cmd.cmd = MIRACAST_CMD_STOP;
	cmd.param = 0;

	if(message_channel_fd > 0){
		send(message_channel_fd, (void *)&cmd, sizeof(cmd), 0);
	}
}

static void send_p2p_fail_to_player()
{
	miracast_cmd_t cmd;
	
	cmd.cmd = MIRACAST_CMD_P2P_FAILED;
	cmd.param = 0;

	if(message_channel_fd > 0){
		send(message_channel_fd, (void *)&cmd, sizeof(cmd), 0);
	}
}

void player_stop( rtsp_config_t* rtsp_config )
{
    DEBUG( "entering" );
    settings_t* settings = (settings_t*)rtsp_config->caller_context;

	NOTICE( "-----%s -------\n",__FUNCTION__);

#if 0
    if ( settings->player_stop_script ) {
        char buf[255];
        snprintf( buf, sizeof(buf), "%s %d", settings->player_stop_script, rtsp_config->rtp_port );
        DEBUG( "system call: %s", buf );
        system( buf );
    } else {
        DEBUG( "would stop player here, but -S parameter not specified" );
    }
#endif

}

void player_start( rtsp_config_t* rtsp_config )
{
    settings_t* settings = (settings_t*)rtsp_config->caller_context;
	char buf[255];
	miracast_cmd_t cmd;

	NOTICE( "-----%s -------\n",__FUNCTION__);

	cmd.cmd = MIRACAST_CMD_PLAY;
	cmd.param = rtsp_config->rtp_port;

	if(message_channel_fd > 0){
		send(message_channel_fd, (void *)&cmd, sizeof(cmd), 0);
	}

#if 0
	sprintf(buf,"%s%s %d","sh ","/mnt/udisk/rtp_test.sh",rtsp_config->rtp_port);

	NOTICE( "---------------------------\n");
	NOTICE( "-----Start RTP Demo -------\n");
	NOTICE( "---------------------------\n");
	printf("%s\n",buf);

	system(buf);
#endif

}

void player_pause( rtsp_config_t* rtsp_config )
{
	settings_t* settings = (settings_t*)rtsp_config->caller_context;
	miracast_cmd_t cmd;

	NOTICE( "-----%s -------\n",__FUNCTION__);

	cmd.cmd = MIRACAST_CMD_PAUSE;
	cmd.param = 0;

	if(message_channel_fd > 0){
		send(message_channel_fd, (void *)&cmd, sizeof(cmd), 0);
	}
	
#if 0
    DEBUG( "entering" );
    settings_t* settings = (settings_t*)rtsp_config->caller_context;

	NOTICE( "-----%s -------\n",__FUNCTION__);

    if ( settings->player_paused ) {
        NOTICE( "already in PAUSE state; ignoring" );
    } else {
        rtsplib_pause( rtsp_config->rtsp_mode, rtsp_config->hRtspLib );
        settings->player_paused = 1;
    }
#endif
}

void player_unpause( rtsp_config_t* rtsp_config )
{
    DEBUG( "entering" );
    settings_t* settings = (settings_t*)rtsp_config->caller_context;

	NOTICE( "-----%s -------\n",__FUNCTION__);

    if ( settings->player_paused ) {
        rtsplib_play( rtsp_config->rtsp_mode, rtsp_config->hRtspLib );
        settings->player_paused = 0;
        NOTICE( "send PLAY command to source to unpause" );
    } else {
        NOTICE( "ignoring unpause command because not currently in paused state" );
    }
}

void teardown_rtsp( rtsp_config_t* rtsp_config )
{	
    rtsplib_teardown( rtsp_config->rtsp_mode, rtsp_config->hRtspLib );
    DEBUG( "sent RTSP teardown" );
	NOTICE( "-----%s -------\n",__FUNCTION__);

	send_out_to_player();

	NOTICE( "-----%s -------\n",__FUNCTION__);
}

/* Implement the callbacks for the library
 * Use syslog for messages
 * Open a UDP socket and listen on it for user commands
 */

void set_up_logging( void )
{
    //openlog( "sample_rtsp", LOG_PERROR, LOG_USER );
    //setlogmask( LOG_UPTO(LOG_DEBUG) );
}

void signal_callback_handler( int signum )
{
	pid_t wait_pid;
	int status;
	
    NOTICE( "Caught signal %d", signum );

#if 0
	/**
	* p2p has been invoked.
	*/
	if(p2p_pid>0){
		kill((int)p2p_pid,SIGKILL);
		wait_pid = wait(&status);
		if(wait_pid == p2p_pid){
			printf("[%s]: Kill the P2P\n",__FUNCTION__);
		}
	}
    exit( signum );
#endif

}

void register_signal_handler()
{
    signal( SIGINT, signal_callback_handler );
}

void affinity_file_test( void )
{
    char* secret = rtsp_affinity_get_shared_secret();
    printf( "read secret from disk: %s\n", secret );
}

void print_usage( char** argv )
{
    const char* usage =
        "%s\n"
        "   -s  <RTSP source IP address>                    required\n"
        "   -p  <RTSP source port number>                   required\n"
        "   -u  <UDP port number for receiving RTP data>    default %d\n"
        "   -w  use Wi-Fi Display-compliant RTSP\n"
        "   -r  use RDS-compliant RTSP\n"
        "   -E  use hard-coded sample Sony EDID\n"
        "   -a  run affinity test vector check and exit\n"
        "   -f  test reading the affinity key file from disk and exit\n"
        "   -c  <named pipe filename for commands>\n"
        "   -P  <path to script or command to run on RTSP PLAY>\n"
        "   -T  <path to script or command to run on RTSP TEARDOWN>\n"
        "";
    printf( usage, argv[0], RTP_PORT_DEFAULT );
}

void process_command_line
    ( int argc
    , char** argv
    , settings_t* settings
    )
{
    int c;

    while ((c = getopt(argc, argv, "s:p:P:T:u:afwrEc:")) != -1) {
        switch (c) {
            case 's':
                snprintf
                    ( settings->source_ip_address,
                      sizeof(settings->source_ip_address), "%s",
                      optarg );
                break;
            case 'p':
                settings->source_port = (uint16_t)atoi( optarg );
                break;
            case 'P':
                settings->player_start_script = strdup( optarg );
                break;
            case 'T':
                settings->player_stop_script = strdup( optarg );
                break;
            case 'u':
                settings->rtp_port = (uint16_t)atoi( optarg );
                break;
            case 'a':
                rtsp_affinity_test_vector();
                exit( 0 );
            case 'f':
                affinity_file_test();
                exit( 0 );
            case 'w':
                settings->rtsp_mode = RTSP_MODE_WFD;
                break;
            case 'r':
                settings->rtsp_mode = RTSP_MODE_RDS;
                break;
            case 'E':
                settings->use_sample_edid = 1;
                break;
            case 'c':
                settings->named_pipe_filename = strdup( optarg );
                break;
            case '?':
                if ( optopt == 's' || optopt == 'p' || optopt == 'u' ) {
                    ERROR( "Option -%c requires an argument.", optopt );
                } else {
                    ERROR ( "Unknown option character `\\x%x'.", optopt);
                }
                print_usage( argv );
                exit( 1 );
            default:
                abort();
        }
    }

    if ( strlen( settings->source_ip_address ) == 0 ) {
        print_usage( argv );
        ERROR( "must specify -s" );
        abort();
    } else if ( settings->source_port == 0 ) {
        print_usage( argv );
        ERROR( "must specify -p" );
        abort();
    } else if ( settings->rtsp_mode == 0 ) {
        print_usage( argv );
        ERROR( "must specify -r or -w" );
        abort();
    }
}

/**
 * @brief This function will only get called if the primary
 * get_parameter is unable to fulfill the request
 */
long handle_rtsp_get_parameter_wfd( rtsp_config_t* rtsp_config, WFD_PARAMETER_STRUCT* p )
{
    const int rtsp_mode = rtsp_config->rtsp_mode;
    char* param_name = (char*)rtsplib_param_name( rtsp_mode, p->nId );

    DEBUG( "entering %s", param_name );

    if ( rtsplib_param_is_vendor_extension( rtsp_mode, p->nId ) ) {
        CRITICAL( "need to handle vendor extension %s", p->szValue );
    } else {
        CRITICAL( "request for unimplemented parameter: %s", param_name );
    }

    return rtsplib_error_code( rtsp_mode, "NO_ERROR" );
}

/**
 * @brief Handles GET PARAMETER requests from the RTSP source. For many
 * parameters, will try and respond automatically using values stored in
 * rtsp_config. If this is not possible, will invoke a callback handler.
 */
long handle_rtsp_get_parameter( rtsp_config_t* rtsp_config, void* p )
{
    const int rtsp_mode = rtsp_config->rtsp_mode;
    long rc = -1;

    if ( rtsp_mode == RTSP_MODE_RDS ) {
        DEBUG( "implement vendor extension here as needed" );
    } else if ( rtsp_mode == RTSP_MODE_WFD ) {
        DEBUG( "implement vendor extension here as needed" );
    }

    if ( rc == -1 ) {
        // Couldn't automatically handle this request; call the
        // registered callback handler
        DEBUG( "need to implement registered callback handler" );
        return -1;
    }

    CRITICAL( "%s(%d): should never get here", __func__, __LINE__ );
    return 1;
}

long handle_rtsp_set_parameter_rds( rtsp_config_t* rtsp_config, PARAMETER_STRUCT* p, RDSRTSPNotifications id )
{
    const int rtsp_mode = rtsp_config->rtsp_mode;
    const char* param_name = rtsplib_param_name( rtsp_mode, p->nId );

    DEBUG( "param_name=%s", param_name );

    switch ( p->nId ) {
        case rds_overscan_comp:
            DEBUG( "overscan compensation received is: x=%d, y=%d",
                    p->u.overscanComp.nX, p->u.overscanComp.nY );
            break;

        case rds_init_intel_hdshk:
        case rds_cfrm_intel_hdshk:
            return rtsp_affinity_check( rtsp_config->hRtspLib, p );

        default:
            CRITICAL( "Request to set unimplemented parameter: %s", param_name );
            break;
    }

    return rtsplib_error_code( rtsp_mode, "NO_ERROR" );
}

long handle_rtsp_set_parameter_wfd( rtsp_config_t* rtsp_config,
        WFD_PARAMETER_STRUCT* p, RDSRTSPNotifications id )
{
    const int rtsp_mode = rtsp_config->rtsp_mode;
    const char* param_name = rtsplib_param_name( rtsp_mode, p->nId );

    DEBUG( "param_name=%s", param_name );

    if ( strcmp( param_name, "VENDOR_EXTENSION" ) == 0 ) {
        // Default action is to just save whatever the source sends
        // This should be modified as necessary
        rtsp_param_set( rtsp_config, p->szParam, p->szValue );
    }

    switch ( p->nId ) {
        default:
            CRITICAL( "%s: unsupported parameter! %s (szValue=%s)", __func__, param_name,
                    p->szValue );
    }

    return rtsplib_error_code( rtsp_mode, "NO_ERROR" );
}

static long handle_rtsp_set_parameter( rtsp_config_t* rtsp_config, void* p, long id )
{
    const int rtsp_mode = rtsp_config->rtsp_mode;
    long rc = -1;

    if ( rtsp_mode == RTSP_MODE_RDS ) {
        rc = handle_rtsp_set_parameter_rds( rtsp_config, (PARAMETER_STRUCT*)p, id );
    }

    if ( rtsp_mode == RTSP_MODE_WFD ) {
        rc = handle_rtsp_set_parameter_wfd( rtsp_config, (WFD_PARAMETER_STRUCT*)p, id );
    }

    return rc;
}

static long secondary_notification_callback( int id, void* szValue, void* pContext )
{
    rtsp_config_t* rtsp_config = (rtsp_config_t*)pContext;
    settings_t* settings = (settings_t*)rtsp_config->caller_context;
    const int rtsp_mode = rtsp_config->rtsp_mode;
    const long no_error = rtsplib_error_code( rtsp_mode, "NO_ERROR" );
    const char* notif_name = rtsplib_notification_name( rtsp_mode, id );

    DEBUG( "id=%d, name=%s", id, notif_name );

    if ( id == rtsplib_notification_code( rtsp_mode, "PLAY" ) ) {
		
		PLAY_STRUCT* p = (PLAY_STRUCT*)szValue;
		
		NOTICE( "RTSP_PLAY: %s, RTP port=%d, RTCP port=%d", p->szValue,
			p->usRTPPort, p->usRTCPPort );
		
		player_start( rtsp_config );

	/**
        if ( settings->player_paused ) {
            NOTICE( "Resuming PLAY from PAUSE state" );
            settings->player_paused = 0;
        } else {
            PLAY_STRUCT* p = (PLAY_STRUCT*)szValue;
            NOTICE( "RTSP_PLAY: %s, RTP port=%d, RTCP port=%d", p->szValue,
                    p->usRTPPort, p->usRTCPPort );
            player_start( rtsp_config );
        }
     */
        return no_error;
    }

    if ( id == rtsplib_notification_code( rtsp_mode, "PAUSE" ) ) {
        NOTICE( "received RTSP PAUSE from source");
        //settings->player_paused = 1;
        player_pause(rtsp_config);
        return no_error;
    }

    if ( id == rtsplib_notification_code( rtsp_mode, "GET_PARAMETER" ) ) {
        return handle_rtsp_get_parameter( rtsp_config, szValue );
    }

    if ( id == rtsplib_notification_code( rtsp_mode, "GET_PARAMETER_RESP" ) ) {
        return handle_rtsp_set_parameter( rtsp_config, szValue, id );
    }

    if ( id == rtsplib_notification_code( rtsp_mode, "SET_PARAMETER" ) ) {
        return handle_rtsp_set_parameter( rtsp_config, szValue, id );
    }

    CRITICAL( "%s(%d): unimplemented case %d (%s)", __func__, __LINE__, id, notif_name );

    DEBUG( "leaving" );
    return no_error;
}

static void cmd_handler
    ( rtsp_config_t* rtsp_config
    , const char* cmd_in
    )
{
    typedef void(*kb_func)( rtsp_config_t* rtsp_config );
    struct {
        const char* cmd;
        kb_func func;
        const char* help;
    } *cmdp, cmds[] = {
        { "p"  , player_pause            , "Pause playback" },
        { "u"  , player_unpause          , "Unpause (resume) playback" },
        { "t"  , teardown_rtsp           , "Teardown RTSP connection" },
        { NULL , NULL                    , NULL }
    };

    if ( ! cmd_in ) {
        return;
    }

    if ( cmd_in[0] == '?' ) {
        // display help
        printf( "Console menu:\n" );
        for ( cmdp=cmds; cmdp->cmd != NULL; cmdp++ ) {
            printf("%2s  %s\n", cmdp->cmd, cmdp->help );
        }
    }

    for ( cmdp=cmds; cmdp->cmd != NULL; cmdp++ ) {
        if ( strncasecmp(cmdp->cmd, cmd_in, strlen(cmdp->cmd) ) == 0 ) {
            cmdp->func( rtsp_config );
        }
    }
}

/**
 * Displays a console menu to the user and accepts keyboard input.
 * Invokes handlers to fulfill the requested keyboard commands.
 *
 * @returns None
 */
static void* keyboard_process
    ( void* voidp ///< void pointer to sample_apps_var_t, passed in from thread_start()
    )
{
    rtsp_config_t* rtsp_config = (rtsp_config_t*)voidp;
    char cmd_in[100];

    while ( 1 ) {
        cmd_handler( rtsp_config, "?" ); // display console menu

        char* ret = fgets( cmd_in, sizeof(cmd_in), stdin );
        if ( ret == NULL ) {
            DEBUG( "process started in background; disabling keyboard thread" );
            return NULL;
        } else {
            cmd_handler( rtsp_config, cmd_in );
        }
    }

    assert( 0 ); // should never get here
}

/**
 * Process that reads commands from a named pipe and processes them.
 * Implemented for SIGMA CAPI automation.
 *
 * @returns NULL
 */
static void* named_pipe_process
    ( void* voidp ///< void pointer to sample_apps_var_t, passed in from thread_start()
    )
{
    settings_t* settings = (settings_t*)voidp;
    const char* path = settings->named_pipe_filename;
    const int poll_interval_in_ms = 500;

    if ( path == NULL ) {
        return NULL;
    }

    // Create named pipe
    mode_t prev_umask = umask( 000 );
    int rc = mkfifo( path, 0666 );
    umask( prev_umask );
    if ( rc == -1 && errno != EEXIST ) {
        DEBUG( "unable to create named pipe: %s", path );
        return NULL;
    }

    int fd = open( path, O_RDONLY | O_NONBLOCK );
    if ( fd == -1 ) {
        DEBUG( "unable to open named pipe: %s", path );
        return NULL;
    }

    DEBUG( "monitoring named pipe %s for commands", path );
    char buf[100];
    while ( 1 ) {
        int numread = read( fd, buf, sizeof(buf) );
        if ( numread > 0 ) {
            buf[numread] = 0; // read() does not guarantee null termination
            DEBUG( "read from pipe: %s", buf );
            cmd_handler( settings->rtsp_config, buf );
        }

        usleep( poll_interval_in_ms * 1000 );
    }


    // This thread will never exit, but putting a return value to keep the compiler happy
    return NULL;
}

static void set_default_settings( settings_t* s )
{
    memset( s, 0, sizeof(settings_t) );

    s->rtp_port = RTP_PORT_DEFAULT;
}

static void set_sample_edid( settings_t* s, rtsp_handle h )
{
	FILE *fp;
	uint8_t* edid = NULL;
	long edidlen = 0;
	long readnr = 0;

	fp = fopen("/mnt/vram/edidinfo.bin","r");
	if(fp){
		/** read the saved edid information */
		fseek(fp, 0, SEEK_END);
		edidlen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if(edidlen > 0){
			edid = (uint8_t*)malloc(edidlen);
			if(edid){
				readnr = fread(edid, sizeof(uint8_t),edidlen,fp);
				if(readnr == edidlen){
					fclose(fp);
					rtsp_set_edid_and_connector( h, edid, edidlen, RTSP_CONNECTOR_HDMI );
					free(edid);
					printf(">>>>>>>>>>>> set dynamic edid ok\n");
					return;
				}
				free(edid);
				edid = NULL;
			}
		}
		fclose(fp);
	}
	
    if ( s->use_sample_edid ){
        edid = rtsp_get_sample_edid( &edidlen );
        rtsp_set_edid_and_connector( h, edid, edidlen, RTSP_CONNECTOR_HDMI );
    }
}

void start_keyboard_thread
    ( rtsp_config_t* rtsp_config
    )
{
    static thread_data_t t;
    memset( &t, 0, sizeof(t) );
    t.data = rtsp_config;
    start_thread( &t, keyboard_process );
}

void start_named_pipe_thread
    ( settings_t* settings
    , rtsp_config_t* rtsp_config
    )
{
    static thread_data_t t;
    memset( &t, 0, sizeof(t) );
    settings->rtsp_config = rtsp_config;
    t.data = settings;
    start_thread( &t, named_pipe_process );
}

static int __get_source_ip_address(char *ipaddr,int size)
{
	FILE *fp;
	char buf[128]={0};

	if(ipaddr == NULL){
		return -1;
	}

	fp = fopen("/tmp/sourceip.log","r");
	if(fp == NULL){
		printf(">>>>>>>>>> open /tmp/sourceip.log error\n");
		return -1;
	}

	printf(">>>>>>>>>> %d,%s\n",__LINE__,__FUNCTION__);
	fgets(buf, 128, fp);
	printf(">>>>>>>>>>buf=%s,bufsize=%d,size=%d\n",buf,strlen(buf),size);
	if((strlen(buf) <= 0) || (strlen(buf)>size)){
		fclose(fp);
		return -1;
	}
	strcpy(ipaddr, buf);

	return 0;

}

static int __get_rtsp_port()
{
	FILE *fp;
	char buf[128]={0};
	char *ptr;
	int port = -1;

	fp = fopen("/tmp/port.log","r");
	if(fp == NULL){
		return -1;
	}

	while(!feof(fp)){
		fgets(buf, 128, fp);
		ptr = strstr(buf, "Port=");
		if(ptr){
			ptr += 5;
			port = atoi(ptr);
			break;
		}
	}

	fclose(fp);

	return port;
}

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>



/**
* @brief Function for execute a shell script.
* 
* @note The last arg for arglist must be NULL.
*/
static int exec_shell_script(char *script,char *arglist,...)
{
#define INITIAL_ARGV_MAX 1024

	const char *argv[INITIAL_ARGV_MAX];
	pid_t pcid;
	pid_t wait_pid;
	int execstat;
	int status;
	va_list args;
	unsigned int i;

	/**
	* process the args.
	*/
	argv[0] = "sh";
	argv[1] = script;
	argv[2] = arglist;

	va_start (args, arglist);

	i = 2;
	while (argv[i] != NULL)
	{
		i++;
		argv[i] = va_arg (args, const char *);
	}
	
	va_end (args);
	
	/**
	* exec sh.
	*/
	pcid = fork();
	if(pcid == -1){
		printf("[%s]: fork error\n",__FUNCTION__);
		return -1;
	}
	else if(pcid == 0){
		execstat = execv("/bin/sh", (char *const *)argv);
		if(execstat == -1){
			printf("[%s]: execl error\n",__FUNCTION__);
			exit(1);
		}
	}
	else{
		p2p_pid = pcid;
		wait_pid = wait(&status);
		if(wait_pid == -1){
			printf("[%s]:wait for child error\n",__FUNCTION__);
			return -1;
		}
		if(WIFEXITED(status)){
			printf("[%s]:normal termination\n",__FUNCTION__);
		}	
		if(WIFSIGNALED(status)){
			printf("[%s]:signal exit\n",__FUNCTION__);
		}
		if(WIFSTOPPED(status)){
			printf("[%s]:stopped\n",__FUNCTION__);
		}
		return 0;
	}
}

pthread_t ctrl_thread_id=-1;
enum{
	SIGMA_CMD_NONE=-1,
	SIGMA_CMD_PLAY=0,
	SIGMA_CMD_PAUSE=1,
	SIGMA_CMD_RESUME=2,
	SIGMA_CMD_STOP=3,
	SIGMA_CMD_TEARDOWN=4,
};
static void *_rtsp_ctrl_msg_loop(void *arg)
{
	fd_set rfd_set;
	int ret;
	int cmd = SIGMA_CMD_NONE;
	int cmdsize;
	rtsp_config_t *prtsp_config=(rtsp_config_t *)arg;

	while(1){
		cmd = SIGMA_CMD_NONE;
		FD_ZERO(&rfd_set);
		FD_SET(message_channel_fd, &rfd_set);
		ret = select(message_channel_fd+1,&rfd_set,NULL,NULL,NULL);
		if(ret>0){		
			if(FD_ISSET(message_channel_fd,&rfd_set)){
				cmdsize = read(message_channel_fd,&cmd,sizeof(int));
				if(cmdsize == sizeof(int)){
					switch(cmd){
						case SIGMA_CMD_PLAY:
							printf(">>> RTSP engine cmd: play\n");
							rtsplib_play(prtsp_config->rtsp_mode, prtsp_config->hRtspLib);
						break;
						case SIGMA_CMD_PAUSE:
							printf(">>> RTSP engine cmd: pause\n");
							rtsplib_pause(prtsp_config->rtsp_mode, prtsp_config->hRtspLib);
						break;
						case SIGMA_CMD_TEARDOWN:
							printf(">>> RTSP engine cmd: teardown\n");
							teardown_rtsp(prtsp_config);
						break;
					}
				}
			}
		}
	}
}

static int isHdcpEnable(){
	if(!access(MIRACAST_HDCPDISABLE_PATH, 0)){
		unlink(MIRACAST_HDCPDISABLE_PATH);
		return 0;
	}

	return 1;
}

#ifndef WIDI_TEST
int main
    ( int argc
    , char** argv
    )
{
    rtsp_config_t rtsp_config;
    int cancel_request = 0;
    long rc;
	char sh_command[64];
	char sourceip[20]={0};
	int rtsp_port=0;
	settings_t settings;
	struct sockaddr_un un,uns;
	int err,cnt;

	set_up_logging();
    NOTICE( "WiDi RTSP Sink Sample (compiled %s %s)", __DATE__, __TIME__ );

	
	/**
	* setup a communication channel.
	*/
	un.sun_family = AF_UNIX;
    strcpy(un.sun_path, "/tmp/miracast_client");
	if ((message_channel_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
		printf("[%s]: socket create error\n",__FUNCTION__);
		exit(1);
	}

	/** before bind, must make sure the path file not exist */
	unlink(un.sun_path);
	
	//size = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
	if (bind(message_channel_fd, (struct sockaddr *)&un, sizeof(un)) < 0){
		printf("[%s]: bind error\n",__FUNCTION__);
		close(message_channel_fd);
		exit(1);
	}

	cnt = 0;

	while(cnt < 10){
		memset(&uns, 0, sizeof(uns));
	    uns.sun_family = AF_UNIX;
	    strcpy(uns.sun_path, "/tmp/miracast_server");
		err = connect(message_channel_fd, (struct sockaddr *)&uns, sizeof(uns));
	    if ( err < 0) {
	        printf("[%s]: connect error\n",__FUNCTION__);
			sleep(1);
			cnt++;
	    }
		else{
			break;
		}
	}

	if(cnt >= 10){
		printf("[%s]: final connect error\n",__FUNCTION__);
		close(message_channel_fd);
		exit(1);
	}

	

	register_signal_handler();

	/**
	**************************************************************
	* Step 1. Do the device discovery via P2P.
	**************************************************************
	*/
	NOTICE( "Miracast do P2P connection\n");
	//sprintf(sh_command,"sh /am7x/case/scripts/miracast_p2p.sh wlan0 1 1 PBC 00000000 6 6");
	//system(sh_command);
	//exec_shell_script("/am7x/case/scripts/miracast_p2p.sh","wlan0","1","1","PBC","00000000","6","6",NULL);

	if(__get_source_ip_address(sourceip,sizeof(sourceip))!=0){
		NOTICE( "Miracast could not get source ip address\n");
		//send_p2p_fail_to_player();
		if(message_channel_fd > 0){
			close(message_channel_fd);
			message_channel_fd = -1;
		}
		exit(1);
	}
	NOTICE( "Miracast source ip address : %s\n",sourceip);

	rtsp_port = __get_rtsp_port();
	if(rtsp_port <=0 ){
		rtsp_port = 7236;
	}

	/**
	**************************************************************
	* Step 2. Run the RTSP.
	**************************************************************
	*/
   
    //NOTICE( "Press Ctrl+C to abort..." );
    set_default_settings( &settings );

	/**
	* set our setting parameters.
	*/
	settings.rtsp_mode = RTSP_MODE_WFD;
	strcpy(settings.source_ip_address,sourceip);
	settings.source_port = (uint16_t)rtsp_port;
	settings.use_sample_edid = 1;
	
    //process_command_line( argc, argv, &settings );

    rtsp_init( &rtsp_config, settings.rtsp_mode, settings.rtp_port, secondary_notification_callback, &settings );
    set_sample_edid( &settings, &rtsp_config );
	/**
	* LPCM 48K 16bits 2 channels. see table 5-21.
	*/
	//rtsp_set_audio_codecs(&rtsp_config,0x3,0xf,0x7,1);
	rtsp_set_audio_codecs(&rtsp_config,2,0,0,1);
	/**
	* video CEA support 0x11101011
	*/
	#if (defined(MODULE_CONFIG_MIRACAST_FULL_HD) && (MODULE_CONFIG_MIRACAST_FULL_HD == 0))
	uint32_t cea_support = 0x8C7F;
	uint32_t vesa_support = 0x3C7FFF;
	uint32_t hh_support = 0xfff;
	#else
	uint32_t cea_support = 0x1deff;
	uint32_t vesa_support = 0x53c7fff;
	uint32_t hh_support = 0xfff;
	#endif
	rtsp_set_video_formats(&rtsp_config,cea_support,vesa_support,hh_support,0,1);

    NOTICE( "RTSP library is running in %s mode", rtsp_config.rtsp_mode == RTSP_MODE_RDS ? "RDS" : "WFD" );

	/**
	* HDCP2.O TCP port.
	*/
#if MODULE_CONFIG_HDCP_ENABLE==1
	if(isHdcpEnable()){
		printf("[%s] [%d] -- HDCP ENABLE\n", __FILE__, __LINE__);
		rtsp_set_hdcp2_port(&rtsp_config,10123);
	}else{
		printf("[%s] [%d] -- HDCP DISABLE\n", __FILE__, __LINE__);
		rtsp_set_hdcp2_port(&rtsp_config,0);
	}
#else
	rtsp_set_hdcp2_port(&rtsp_config,0);
#endif

    //start_keyboard_thread( &rtsp_config );
    //start_named_pipe_thread( &settings, &rtsp_config );

	if (pthread_create(&ctrl_thread_id, NULL, _rtsp_ctrl_msg_loop, &rtsp_config) != 0){
		printf("[%s]: rtsp loop msg thread create error\n",__FUNCTION__);
	}

    rtsp_request_t requests = { 0, 0, 0 };
    rc = rtsp_sink_run_session( &rtsp_config, settings.source_ip_address, settings.source_port, &cancel_request, &requests );
    if ( rc == RTSP_SOCKET_NOT_CONNECTED && rtsp_config.teardown_received == 1 ) {
        NOTICE( "teardown received, exiting RTSP library" );
    } else {
        NOTICE( "RTSP session exited with return code %ld", rc );
    }

	/**
	**************************************************************
	* Step 3. Miracast out, should release the resouces.
	**************************************************************
	*/

	if(ctrl_thread_id != -1){
		pthread_cancel(ctrl_thread_id);
		pthread_join(ctrl_thread_id, NULL);
	}
	
	send_out_to_player();
	if(message_channel_fd > 0){
		close(message_channel_fd);
		message_channel_fd = -1;
	}
	
    return 0;
}
#endif
