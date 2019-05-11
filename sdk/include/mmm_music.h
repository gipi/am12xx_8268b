#ifndef MMM_MUSIC_H
#define MMM_MUSIC_H

#include "sys_buf.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define ATTR_VISIBLE    __declspec(dllexport)
#else
#define ATTR_VISIBLE	__attribute__ ((visibility("default")))
#endif

typedef void *AUDIO_DEC_INST;



enum DAC_STAUE_E{
	ALREADY_SET_DAC_PARA = 1,
	SET_DAC_PARA_ERROR = 2
};

enum AUDIO_DECODE_E{
        AUDIO_DECODE_OK                       =   0,
        AUDIO_DECODE_ERR		             			=  -1,
        AUDIO_NOTSUPPORT_BLURAYHD							=  -2,
        AUDIO_NOTSUPPORT_AC3HD                =  -3,
        AUDIO_NOTSUPPORT_DTSHD           			=  -4,
        AUDIO_SETDAC_ERR											=	 -10    
        
        };


typedef enum{
	MP3 = 1 ,
	WMA ,
	APE ,
	OGG ,
	WAV ,
	FLA ,
	ASF ,
	AMR ,
	AC3,
	AAC,
	FLAC,
	DTS,
	COOK,
	RA_144,
	RA_288,
	ATRAC3,
	SIPR,
	M4A,
} AUDIO_TYPE_E;


//typedef  AUDIO_TYPE_E music_type_t;
#define music_type_t AUDIO_TYPE_E;

typedef struct audio_decoder_s{
	int (*init)(struct audio_decoder_s *plugin,void *param);
	int (*decode_data)(struct audio_decoder_s *plugin);
	int (*get_err)(struct audio_decoder_s *plugin);
	unsigned int (*get_time)(struct audio_decoder_s *plugin);
	int (*sync)(struct audio_decoder_s *plugin, unsigned int cmd);        
	int (*dispose)(struct audio_decoder_s *plugin);           
}audio_decoder_t;

typedef struct _mem_entry{
	unsigned long  physic_address;	
	unsigned long  logic_address;
	unsigned long  request_size;
}mem_entry;

typedef struct AUDIO_DEC_CONTEXT{	
	void * task_syn;
	void * pheap;	
	int dsp;
	int dsp_spdif;
	int data_rem;		
	unsigned char 	*addr;
	mem_entry dac_buf;
	//struct mem_dev dac_buf;
	//mem_entry spdif_buf;
	struct mem_dev spdif_buf;
	audio_decoder_t *dec;
	int sample_rate;
    int bakSampleRate;
	int num_of_channel;	
    int bakChannel;
	int precision;//for wav only
	AUDIO_TYPE_E	music_type;//=MP3;
	char	player_para;//=GET_MUSIC_INFO;	
	char dac_status;//if set dac para will change to =READY_SET_DAC_PARA
	int ogg_end;
	unsigned char  	audio_in[1024*20];
	int codec_id;
	void * is;//For Video state
	char av_fifo_enable;
	int play_dev;
	
} AUDIO_DEC_CONTEXT;

typedef struct ret_info{
	unsigned int pcm_size;
	int sample_rate;
	int num_of_channel;
	int bitrate;
	int frame;
	int total_frame;
	int samples;
	int end_ret;//for ogg
	int page_flag;//for ogg
	int ret;//for wma
	int byte_used;
}audio_ret_info_t;

typedef enum{ 
	GET_MUSIC_INFO ,
	MUSIC_PLAY ,
	MUSIC_RESTORE ,	
	MUSIC_END ,
	MUSIC_SEEK,
	MUSIC_SWITCH
} player_para_t;

typedef struct{   
	audio_decoder_t audio_plugin; 
	int Length_src;
	void *src_addr;
	void *dst_addr;
	int cur_pos;
	audio_ret_info_t audio_ret;
	void* private_data;
}demo_t;

typedef struct{
	char *input;
	char *output;
}audio_plugio_t;

typedef struct _audio_header_info_t
{
	int sample_rate; ///< samples per second
    int channels;    ///< number of audio channels
	int block_align;
	int bitrate;
	int bits_per_coded_sample;
	unsigned int format_tag;
	unsigned char *extradata;
    int extradata_size;
    int avg_bytes_per_second;
	int audio_speed;
}audio_header_info_t;
#define AD_IOCS_INIT            0x4b01 // init, only once per decode task for certain format
#define AD_IOCS_START           0x4b02 // start at diffrent mode, start vs. stop
#define AD_IOCT_SYN_STOPWR		0x4b36
#define AD_IOCG_SEEK			0x4b37
#define AD_IOCT_FINISH			0x4b22
#define AD_IOCT_CONTINUE		0x4b2b	// resume dsp interrupt
#define AD_IOCT_STOPWR			0x4b2c	// stop write
#define AD_IOCT_STARTWR			0x4b2d	// start write
#define AD_IOCT_RESETWR			0x4b2e	// clean the buffer
#define AD_IOCT_STOP            0x4b2f //
#define AD_IOCT_PAUSEWR			0x4b32
#define AD_IOCG_STATUS          0x4b42 // »ñÈ¡½âÂë×´Ì¬
#define AD_IOCG_REMAIN          0x4b43 

#define ACT_SAMPLE_96K		0x0
#define ACT_SAMPLE_48K		0x1
#define ACT_SAMPLE_44K		0x9
#define ACT_SAMPLE_32K		0x2
#define ACT_SAMPLE_24K		0x3
#define ACT_SAMPLE_22K		0xb
#define ACT_SAMPLE_16K		0x4
#define ACT_SAMPLE_12K		0x5
#define ACT_SAMPLE_11K		0xd
#define ACT_SAMPLE_8K		0x6


#define Earphone		0x1
#define HDMI			0x2
#define I2S				0x4
#define SPDIF2			0x8
#define SPDIF51			0x10


typedef enum _audio_play_dev_e
{
	PlayDev_Earphone=Earphone,				//only earphone output
	PlayDev_HDMI=HDMI,						//only HDMI	output
	PlayDev_I2S=I2S,						//only I2s output
	PlayDev_SPDIF2=SPDIF2,					//only SPDIF 2 channel PCM output
	PlayDev_SPDIF51=SPDIF51,				//SPDIF DTS and AC3 5.1 channel ,other 2 channel PCM output
	PlayDev_Earphone_HDMI=(Earphone|HDMI),	//earphone and HDMI output at the same time
	PlayDev_I2S_HDMI=(I2S|HDMI),			//I2S and HDMI output at the same time
	PlayDev_Earphone_SPDIF2=(Earphone|SPDIF2),	//earphone and SPDIF2 output at the same time
	PlayDev_Earphone_SPDIF51=(Earphone|SPDIF51)	//earphone and PlayDev_SPDIF51 output at the same time
}audio_play_dev_e;

#ifdef __cplusplus
}
#endif

#endif
