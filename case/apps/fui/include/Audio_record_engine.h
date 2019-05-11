#ifndef SWFEXT_AUDIO_RECORD_ENGINE_H
#define SWFEXT_AUDIO_RECORD_ENGINE_H

#ifdef MODULE_CONFIG_AUDIO_RECORD

#define		MAX_NAME_SIZE			256

typedef enum{
	REC_IDLE=0,  //< record engine state: idle
	REC__RECORDING=1, //< record engine state: recording
	REC_PAUSE=2,//< record engine state: pause
	REC_STOPPED=3,//< record engine state: stop
	REC_ERROR = 4//< record engine state: error
}RECORDER_STATE;

#endif	/** MODULE_CONFIG_AUDIO_RECORD */

#endif
