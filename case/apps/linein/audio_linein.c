#include <string.h>
#include <stdio.h>
#include "audio_linein.h"

extern int mode;
extern int VolumeDown;


void *AudioLineInStart(linein_input_t *input)
{
	void* handle;

	handle=audioLineInOpen(input);
	if(NULL==handle)
		return NULL;

	if(VolumeDown)
		audioLineInCmd(handle,ENC_SET_AUDIO_GAIN,0xf);	//means volume down half

	audioLineInCmd(handle,ENC_SET_AUDIO_PARAM,mode);
	
	audioLineInCmd(handle,ENC_START_RECORDING,0);

	return handle;
}

int AudioLineInStop(void *handle)
{
	audioLineInCmd(handle,ENC_STOP_RECORDING,0);

	audioLineInClose(handle);

	return 0;
}

void AudioLineInSetDelay(void *handle,unsigned int delay)
{
	audioLineInCmd(handle,LINEIN_SET_DELAYMS,delay);
}

unsigned int AudioLineInGetDelay(void *handle)
{
	unsigned int delay=0;
	audioLineInCmd(handle,LINEIN_GET_DELAYMS,(unsigned int)&delay);
	return delay;
}

void AudioLineInIncDelay(void *handle)
{
	audioLineInCmd(handle,LINEIN_INC_DELAYMS,0);
}

void AudioLineInDecDelay(void *handle)
{
	audioLineInCmd(handle,LINEIN_DEC_DELAYMS,0);
}




