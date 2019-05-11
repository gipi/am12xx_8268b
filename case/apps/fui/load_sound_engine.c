#include "swf_ext.h"
#include "fileselector.h"
#include "audio_common.h"


extern struct _audio_decode_info audio_decode_info[AUDIO_ENGINE_MAX];
extern audio_dec_open	a_open;
extern audio_dec_cmd	a_cmd;
extern audio_dec_close	a_close;
//void * audio_engine=NULL;
char cur_file[AUDIO_FULL_PATH_SIZE];


static void* find_available_handle(void)
{
	int i;
	struct _audio_decode_info *decode_info=NULL;
	for(i=0;i<AUDIO_ENGINE_MAX;i++)
	{
		decode_info=&(audio_decode_info[i]);
		if(decode_info->audio_engine==NULL){
			decode_info->audio_engine=a_open(NULL);
			break;
		}
	}
	if(decode_info->audio_engine==NULL) 
	{
		printf("open audio engine err\n");		
		return 0;
	}
	return &(decode_info->audio_engine);
}
static int judge_available_handle(void** engine)
{
	int i;
	struct _audio_decode_info *decode_info=NULL;
	for(i=0;i<AUDIO_ENGINE_MAX;i++)
	{
		decode_info=&(audio_decode_info[i]);
		if(decode_info->audio_engine==*engine){
			return 0;
		}
	}
	return -1;
}
static int ls_open(void * handle)
{
	void **engine=NULL;
	int n;
	int arg;
	SWFEXT_FUNC_BEGIN(handle);	
	n = Swfext_GetParamNum();
	if(n>0){		
		arg=Swfext_GetNumber();
	}
	else
		goto exit;	
	engine=find_available_handle();
	if(*engine)
	{
		((struct _audio_decode_info *)(engine))->owner=arg;
	}
exit:
	Swfext_PutNumber((int)engine);
	SWFEXT_FUNC_END();	
}
static int ls_get_status(void * handle)
{
	int n;
	int busy=0; 
	void **engine=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	n = Swfext_GetParamNum();
	if(n>0){		
		engine=(void **)Swfext_GetNumber();
		if(*engine==NULL){
			goto exit;
		}
	}
	else
		goto exit;
	
	if(judge_available_handle(engine)){
		goto exit;
	}
	
	a_cmd(*engine,GET_PLAYER_STATUS,(int)(&(((struct _audio_decode_info *)(engine))->player_status)));
	if(((struct _audio_decode_info *)(engine))->player_status.status==PLAYER_PLAYING)
		busy=1;
	
exit:
	printf("get_status:%d\n",busy);
	Swfext_PutNumber(busy);
	SWFEXT_FUNC_END();	
}

static int ls_close(void * handle)
{
	int n;
	void **engine=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	printf("close\n");
	n = Swfext_GetParamNum();
	if(n>0){		
		engine=(void **)Swfext_GetNumber();
		if(*engine==NULL){
			goto exit;
		}
	}
	else
		goto exit;

	if(judge_available_handle(engine)){
		goto exit;
	}
	
	a_cmd(*engine, STOP,0);
	a_close(*engine);
	memset((struct _audio_decode_info *)(engine),0,sizeof(struct _audio_decode_info));


exit:
	
	SWFEXT_FUNC_END();	
}


static int ls_play(void * handle)
{
	char * file;
	int n;
	int offset=0;
	int ret=0;
	play_param_t play_param;
	void **engine=NULL;

	SWFEXT_FUNC_BEGIN(handle);


	n = Swfext_GetParamNum();
	if(n>0){		
		engine=(void **)Swfext_GetNumber();
		//if(n>1)
		//	offset=Swfext_GetNumber();
		if(n>1){
			((struct _audio_decode_info *)(engine))->s=Swfext_GetNumber();
		}
		if(*engine==NULL){
			goto exit;
		}

	}
	else
		goto exit;

	if(judge_available_handle(engine)){
		goto exit;
	}

	if(offset>0)
		a_cmd(*engine,SET_SEEK_TIME,offset);
	
	play_param.mode=NORMAL_PLAY;
	play_param.param=0;
	ret=a_cmd(*engine,PLAY,(unsigned int)&play_param);
	if(ret)
	{
		a_close(*engine);
		memset((struct _audio_decode_info *)(engine),0,sizeof(struct _audio_decode_info));
	}
	
exit:	
	SWFEXT_FUNC_END();	
	
}
static int ls_set_file(void * handle)
{
	char * file;
	int n;
	int err_set=0;
	void **engine=NULL;
	struct _audio_decode_info *info=NULL;
	a_set_file_s aSetfile;

	SWFEXT_FUNC_BEGIN(handle);

	n = Swfext_GetParamNum();
	if(n == 0){
		err_set=-1;
		goto exit;
	}
	else{
		file = Swfext_GetString();
		engine=(void **)Swfext_GetNumber();
		if(*engine==NULL){
			goto exit;
		}
	}

	if(judge_available_handle(engine))
	{
		err_set=-1;
		goto out;
	}

	//ae_info("n %d music open file %s,filelen=%d\n",n,file,strlen(file));
	if(strlen(file)>768)
	{
		err_set = -1;
		goto out;
	}
	strcpy(cur_file,file);
	
	//if(audio_engine==NULL){
		//ae_info("music engine null\n");
	//}
	//else{
	info=(struct _audio_decode_info *)(engine);
		_audio_set_setfile_para(&(info->file.file_iocontext),file);
		aSetfile.f_io = &(info->file.file_iocontext);
		aSetfile.pInfo = &(info->media_info);
		err_set= a_cmd(*engine,(unsigned int)SET_FILE,(int)(&aSetfile));
			
	//}

	
out:	
	//printf("err_set==%d\n",err_set);
	if(err_set!=0){
		//audio_play_cmd(audio_engine,GET_PLAYER_STATUS,(unsigned int)&player_status);
		//err_set = player_status.err_no;
		a_cmd(*engine, STOP,0);
		a_close(*engine);
		memset((struct _audio_decode_info *)(engine),0,sizeof(struct _audio_decode_info));

	}
	else{
		a_cmd(*engine,GET_MEDIA_INFO,(int)(&(info->media_info)));	
		printf("%s,%d:total time is %d\n",__FILE__,__LINE__,info->media_info.total_time);
		printf("%s,%d:sample rate is %d\n",__FILE__,__LINE__,info->media_info.sample_rate);
		printf("%s,%d:bitrate is %d\n",__FILE__,__LINE__,info->media_info.bitrate);
		printf("%s,%d:channels is %d\n",__FILE__,__LINE__,info->media_info.channels);
	}
exit:	
	printf("set_file:%d\n",err_set);
	Swfext_PutNumber(err_set);
	
	SWFEXT_FUNC_END();	
}
static int ls_stop(void * handle)
{
	int n=0;
	void **engine=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	
	n = Swfext_GetParamNum();
	if(n == 0){
		goto exit;
	}
	else{
		engine=(void **)Swfext_GetNumber();
		if(*engine==NULL){
			goto exit;
		}
	}

	if(judge_available_handle(engine))
	{
		goto exit;
	}
	a_cmd(*engine,STOP,0);
exit:
	SWFEXT_FUNC_END();	
}
static int ls_get_total_time(void * handle)
{
	int total_time = 0;
	int n;
	void **engine=NULL;	
	SWFEXT_FUNC_BEGIN(handle);

	n = Swfext_GetParamNum();
	if(n>0){		
		engine=(void **)Swfext_GetNumber();
		if(*engine==NULL){
			goto exit;
		}
	}
	else
		goto exit;

	if(judge_available_handle(engine)){
		goto exit;
	}

	total_time = ((struct _audio_decode_info *)(engine))->media_info.total_time;
	printf("get totaltime %d\n",total_time);	
exit:
	Swfext_PutNumber(total_time);
	
	SWFEXT_FUNC_END();	
}


static int ls_get_current_time(void * handle)
{
	int crrent_time = 0;
	int n;
	void **engine=NULL;	
	SWFEXT_FUNC_BEGIN(handle);

	n = Swfext_GetParamNum();
	if(n>0){		
		engine=(void **)Swfext_GetNumber();
		if(*engine==NULL){
			goto exit;
		}
	}
	else
		goto exit;

	if(judge_available_handle(engine)){
		goto exit;
	}
	
	a_cmd(*engine,GET_PLAYER_STATUS,(int)(&(((struct _audio_decode_info *)(engine))->player_status)));
	crrent_time = ((struct _audio_decode_info *)(engine))->player_status.cur_time_ms;	
exit:	
	Swfext_PutNumber(crrent_time);
	
	SWFEXT_FUNC_END();	
}


static int ls_get_singer(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: get singer!!! */
	Swfext_PutString("unknown singer");

	SWFEXT_FUNC_END();	
}


static int ls_get_album(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: get album!!! */
	Swfext_PutString("unknown album");

	SWFEXT_FUNC_END();		
}
int swfext_sound_register(void)
{
	SWFEXT_REGISTER("ls_Open", ls_open);
	SWFEXT_REGISTER("ls_Close", ls_close);
	SWFEXT_REGISTER("ls_Play", ls_play);
	SWFEXT_REGISTER("ls_Stop", ls_stop);
	SWFEXT_REGISTER("ls_SetFile", ls_set_file);
	SWFEXT_REGISTER("ls_CurTime", ls_get_current_time);
	SWFEXT_REGISTER("ls_TotalTime", ls_get_total_time);
	SWFEXT_REGISTER("ls_Album", ls_get_album);
	SWFEXT_REGISTER("ls_Singer", ls_get_singer);
	SWFEXT_REGISTER("ls_Get_status", ls_get_status);
	return 0;
}
