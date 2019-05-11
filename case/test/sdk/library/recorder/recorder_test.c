#include "audio_record.h"
#define OPTIONAL
_file_iocontext_s f_io;
record_status_t status;
/////////fill f_io struct


////////////

main(){
	void* handle;
	handle=audioEncOpen(0);
	audioEncCmd(handle,ENC_SET_AUDIO_PARAM,8000);//set sample rate,8000,44100 and so on
	#ifdef OPTIONAL
	audioEncCmd(handle,ENC_SET_AUDIO_GAIN,5);//0~7
	audioEncCmd(handle,ENC_SET_RECORD_TIME,60);//total record time second
	#endif
	audioEncCmd(handle,ENC_START_RECORDING,&f_io);
	#ifdef OPTIONAL
	audioEncCmd(handle,ENC_GET_STATUS,&status);
	#endif
	audioEncCmd(handle,ENC_STOP_RECORDING,0);
	audioEncClose(handle,0);
}