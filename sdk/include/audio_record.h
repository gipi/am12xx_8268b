#ifndef __AUDIO_RECORD_H__
#define __AUDIO_RECORD_H__

#define ENC_SET_AUDIO_PARAM 0x1
#define ENC_SET_AUDIO_GAIN	0x2
#define ENC_SET_RECORD_TIME 0x3
#define ENC_START_RECORDING 0x4
#define ENC_STOP_RECORDING 	0x5
#define ENC_GET_STATUS		0x6
#define ENC_PAUSE_RECORDING 	0x7
#define ENC_CONTINUE_RECORDING		0x8
#define LINEIN_SET_DELAYMS		0x9
#define LINEIN_GET_DELAYMS		0xa
#define LINEIN_INC_DELAYMS		0xb
#define LINEIN_DEC_DELAYMS		0xc
#define LINEIN_GET_TIMES		0xd


typedef enum
{
	RECORDER_STOPPED,
    RECORDER_RECORDING,
    RECORDER_TERMINATED
    
} RECORDER_STATUS_t;
typedef struct{
	unsigned int time;						//当前的录音时间
	RECORDER_STATUS_t status;					//当前codec的状态
	
	unsigned int err_no;						//在出错状态时返回的错误号
}record_status_t;	
typedef struct _file_iocontext_s
{	
	
	void *handle;
	long (*write)(void *, unsigned char *, unsigned long);
	long (*seek_set)(void *, long);
	long (*seek_cur)(void *, long);
	long (*seek_end)(void *, long);
	long (*tell)(void *);
}file_iocontext_s;
typedef struct
{
	int PCMSampleRate;		// pcm 采样率, kHz
	
} aeDspInit_t;
int audioEncCmd(void *handle,unsigned int cmd,unsigned int param);
int audioEncClose(void *handle,void *param);
void *audioEncOpen(void *param);


typedef struct{
	unsigned int delay;
	unsigned int adc_addr;
	unsigned int adc_len;
	unsigned int dac_addr;
	unsigned int dac_len;
	int reserved;
}linein_input_t;	


int audioLineInCmd(void *handle,unsigned int cmd,unsigned int param);
int audioLineInClose(void *handle);
void *audioLineInOpen(linein_input_t *param);
//void *audioLineInOpen(void *param);


#endif
