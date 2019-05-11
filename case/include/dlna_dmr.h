#ifndef DLNA_DMR_H
#define DLNA_DMR_H

enum
{
	DLNA_DMR_UUID_SIZE  = 37,   // 36+1
	DLNA_DMR_TITLE_SIZE = 256,
	//DLNA_DMR_MIME_SIZE  = 512,  // content-type, source_type
	DLNA_DMR_IPV4_SIZE  = 32,
}DLNA_DMR_CONSTANT_VALUE;
/*
typedef enum{
	DMR_PLAYER_DEFAULT =      0x00,
	DMR_PLAYER_PLAY =         0x01,    // Play by Upnp/Dlna
	DMR_PLAYER_STOP =         0x02,    // Stop by "dlna_dmr_cmd"
	DMR_PLAYER_STOP_PLAYING = 0x15,    // StopPlaying by "dlna_dmr_play_fail"
}DMR_PLAYER_STATUS;
*/
typedef struct dmr_info_st {
	int dmr_action;                       // Play, Pause, Stop, etc
	//char source_type[DLNA_DMR_MIME_SIZE]; // MIME_TYPE, Content-Type
	int  dmr_on;                          // indicate if it is running
	int dmr_playing;                      // NO use
	char* uri;                            // media uri
	char uuid[DLNA_DMR_UUID_SIZE];        // actions's uuid
	//DMR_PLAYER_STATUS player_status;      // player status
	char client_ipv4[DLNA_DMR_IPV4_SIZE]; // DLNA Client/DMC/DLNA APP ip v4
} dmr_info_t;

typedef struct dmr_start_arg {
	char dmr_title[DLNA_DMR_TITLE_SIZE];
	char dmr_uuid[DLNA_DMR_UUID_SIZE];
	
	long (*get_player_total_time)(void);
	long (*get_player_play_time)(void);
	int (*get_ready_for_dmr)(char*);
	int (*seek_play_for_dmr)(long);
	int (*get_volume_for_dmr)(void);
	int (*set_volume_for_dmr)(int);
	int (*get_mute_for_dmr)(void);
	int (*set_mute_for_dmr)(int);
	void (*set_dlna_dmr_info)(char*,char*,char*,char*);

} dmr_start_arg_t;

typedef enum{
	DEFCMD =            0x00,
	PLAY =              0x01,
	STOP =              0x02,
	PAUSE =             0x03,
	REPLAY_ONE =        0x04,
	REPLAY_ALL =        0x05,
	SET_SEEK_TIME =     0x04,
	FAST_FORWARD =      0x06,
	FAST_BACKWARD =     0x07,
	CANCEL_FF =         0x08,
	CANCEL_FB =         0x09,
	DLNA_DMR_CLOSE =    0x0A,
	GET_MEDIA_INFO =    0x13,
	GET_PLAYER_STATUS = 0x14,
	READY_TO_PLAY =     0x21,
}DMR_COMMAND;

int dlna_dmr_start_run(void *arg);
int dlna_dmr_exit(void *arg);
int dlna_dmr_cmd(void *arg);
char* dlna_dmr_acquire_uri(void *arg);
int dlna_dmr_configure_status(void *arg);
int dlna_dmr_check_media_type(void *arg);
void dlna_dmr_cb(void* data);
//extern long get_total_time_for_dmr();
//extern long get_play_time_for_dmr();
//extern  int seek_play_for_dmr(long seek_time);
char * dlna_dmr_check_media_title(void);
void dlna_dmr_play_fail(void);
long long dlna_dmr_get_file_length(void);
char *dlna_dmr_get_subtitle_url(void);

#endif

