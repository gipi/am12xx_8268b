#include "audio_common.h"
#include "stream_api.h"

struct _audio_decode_info audio_decode_info[AUDIO_ENGINE_MAX];
void* adllhandle = NULL;
audio_dec_open	a_open	=	NULL;
audio_dec_cmd	a_cmd	=	NULL;
audio_dec_close	a_close	=	NULL;


//add new
static int _check_file_exist(const char *filename)
{
	return access(filename, 0); 
}
//end of added

int _audio_clear_setfile_para(file_iocontext_s *p_fio)
{
	if(p_fio->handle!=NULL){
		
		printf("%s%dClose FileHandle=0x%x\n",__func__,__LINE__,p_fio->handle);
		
		if(_check_file_exist(p_fio->file_name) != -1){
			_audio_fclose(p_fio->handle);
		}
		else{
			am_stream_audio_close(p_fio->handle);
		}
		p_fio->handle = NULL;
	}
	if(p_fio){
		p_fio->read =NULL;
		p_fio->seek_cur = NULL;
		p_fio->seek_end = NULL;
		p_fio->seek_set = NULL;
		p_fio->tell = NULL;
		p_fio->get_file_length = NULL;
		if(p_fio->file_name){
			free(p_fio->file_name);
			p_fio->file_name = NULL;
		}
		return 1;
	}
}



int _audio_set_setfile_para(file_iocontext_s *p_fio,char*filename)
{
	if(p_fio->handle!=NULL){
		printf("%s%dClose FileHandle=0x%x\n",__func__,__LINE__,p_fio->handle);
		
		if(_check_file_exist(p_fio->file_name) != -1){
			_audio_fclose(p_fio->handle);
		}
		else{
			am_stream_audio_close(p_fio->handle);
		}
		p_fio->handle = NULL;
	}

	if(p_fio->file_name){
		printf("%s%d Free filename=%s\n",__func__,__LINE__,p_fio->file_name);
		free(p_fio->file_name);
		p_fio->file_name = NULL;
	}
	
	p_fio->file_name = strdup(filename);
	
	if(_check_file_exist(filename)!=-1){	
		
		printf("file in local filesystem\n");
	
		p_fio->handle = _audio_fopen(filename,"rb");
		if(p_fio->handle==NULL)
		{	
			printf("%s,%d:Open File Failed!\n",__FILE__,__LINE__);
			
			return -1;
		}

		p_fio->read = (void*)_audio_read_packet;
		p_fio->seek_cur = _audio_file_seek_cur;
		p_fio->seek_end = _audio_file_seek_end;
		p_fio->seek_set = _audio_file_seek_set;
		p_fio->tell = _audio_get_pos;
		p_fio->get_file_length = NULL;
		return 1;
	}
	else{
		printf("not local filesystem file\n");

		int file_format;
		p_fio->handle = am_stream_audio_open(filename,NULL,&file_format);
		if(p_fio->handle==NULL)
		{	
			printf("%s,%d:Open File Failed!\n",__FILE__,__LINE__);
			return -1;
		}
		
		p_fio->read = (void*)am_stream_audio_read;
		p_fio->seek_cur = am_stream_audio_seek_cur;
		p_fio->seek_end = am_stream_audio_seek_end;
		p_fio->seek_set = am_stream_audio_seek_set;
		p_fio->tell = am_stream_audio_get_pos;
		p_fio->get_file_length = am_stream_audio_get_filesize;
		return 2;
	}
	return 1;
}

int _audio_dll_open(void)
{
	//return 0;

	int ret=0;
	adllhandle=(void*)OSDLOpen("libmusic.so");
	if(NULL==adllhandle)
	{
		OSprintf("can not open libmusic.so\n");
		return 0;
	}

	a_open = (void*)OSDLGetProcAddr(adllhandle,"audioDecOpen");
	a_cmd  = (void*)OSDLGetProcAddr(adllhandle,"audioDecCmd");
	a_close= (void*)OSDLGetProcAddr(adllhandle,"audioDecClose");

	if( (NULL==a_open) ||(NULL==a_cmd) ||(NULL==a_close))
	{
		OSprintf("can not load func\n");
		return 0;
	}
	/*init mutex*/
	/*audio_mutex =OSCreateMutex();
	if(audio_mutex==NULL)
	{
		printf("audio_mutex create failed:%d \n",ret);
		goto init_api_error;
	}*/

	return 0;


init_api_error:
	
	if(adllhandle){
		OSDLClose(adllhandle);
		adllhandle = NULL;
	}
	
dll_open_error:
	
	return -1;
}
int _audio_dll_close()
{
/*	if(audio_engine && a_close){
		a_close(audio_engine);
	}
*/
	int ret;
	a_open = NULL;
	a_cmd = NULL;
	a_close = NULL;

	if(adllhandle){
		OSDLClose(adllhandle);
		adllhandle = NULL;
	}
	//OSDestroyMutex(audio_mutex);
	return 0;
}
