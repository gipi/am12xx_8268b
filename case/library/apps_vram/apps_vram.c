#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "apps_vram.h"

#define GLOVALVALUE_INFO_OFFSET	48	//cfg file data offset
/**
* a mutex that makes the access protection for 
* sysconfig write and store.
*/
pthread_mutex_t apps_vram_sysconfig_lock;

/**
* sdram copy of the system config parameters.
*/
static struct sysconf_param apps_vram_sysconf;


#if 0
	#define vramdbg_info(fmt, arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
	#define vramdbg_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
	#define vramdbg_info(fmt, arg...)
	#define vramdbg_err(fmt,arg...)
#endif

/**
* @brief initialize the application level vram data. 
* @return 0-success and others-failed. 
*/
int apps_vram_init()
{
	int err;
	struct vram_param_t vparam;
	
	/**
	* read the sysconfig from media to sdram.
	*/
	vparam.task_id = VRAM_ID_SYSPARAMS;
	vparam.sub_type = SYS_PARAM_SUBID;
	vparam.target_name = SYS_PARAM_NAME;
	vparam.pbuf = (unsigned char *)&apps_vram_sysconf;
	vparam.offset = 0;
	vparam.length = sizeof(struct sysconf_param);
	err = app_read_vram(&vparam);
	if(err < 0){
		vramdbg_err("apps_vram_init error: [read sysconf error %d]\n",err);
		return -1;
	}

	err = pthread_mutex_init(&apps_vram_sysconfig_lock,NULL);
	if(err != 0){
		vramdbg_err("apps_vram_init error: [sysconf lock init error %d]\n",err);
		return -1;
	}

	return 0;
}

/**
* @brief release the application level vram manager. 
*    this function will be used only in the case that
*    the application system being shut down.
* @return 0-success and others-failed. 
*/
int apps_vram_release()
{
	pthread_mutex_destroy(&apps_vram_sysconfig_lock);
	
	return 0;
}


/**
* @brief store the vram content from sdram to disk that 
*    pointed by "pvram".
* @param vid: vram identification.
* @param pdata: data buffer pointer.
* @param size: size of the data buffer.
* @return 0-success and others-failed. 
*/
int apps_vram_store(int vid)
{
	struct vram_param_t vparam;
	int err;
	
	if(vid == VRAM_ID_SYSPARAMS){
		
		pthread_mutex_lock(&apps_vram_sysconfig_lock);

		vparam.task_id = VRAM_ID_SYSPARAMS;
		vparam.sub_type = SYS_PARAM_SUBID;
		vparam.target_name = SYS_PARAM_NAME;
		vparam.pbuf = (unsigned char *)&apps_vram_sysconf;
		vparam.offset = 0;
		vparam.length = sizeof(struct sysconf_param);
		err = app_write_vram(&vparam);
		if(err < 0){
			printf("apps_vram_store error %d\n",err);
			pthread_mutex_unlock(&apps_vram_sysconfig_lock);
			return -1;
		}
		pthread_mutex_unlock(&apps_vram_sysconfig_lock);
		printf("apps_vram_store OK!\n");
		return 0;
	}
	
	return -1;
}


/**
* @brief read the vram content that already in sdram.
* @param vid: vram identification.
* @param pdata: data buffer pointer.
* @param size: size of the data buffer.
* @return 0-success and others-failed. 
*/
int apps_vram_read(int vid, void *pdata, long size)
{
	if(pdata == NULL){
		return -1;
	}

	if((vid == VRAM_ID_SYSPARAMS) && (size == sizeof(struct sysconf_param))){
		memcpy(pdata,(void *)&apps_vram_sysconf,size);
		vramdbg_info("apps_vram_read OK!\n");
		return 0;
	}

	vramdbg_err("apps_vram_read Error!\n");
	return -1;
}


/**
* @brief store the vram content to sdram.
* @param vid: vram identification.
* @param pdata: data buffer pointer.
* @param size: size of the data buffer.
* @return 0-success and others-failed. 
*/
int apps_vram_write(int vid, void *pdata, long size)
{
	if(pdata == NULL){
		return -1;
	}

	if((vid == VRAM_ID_SYSPARAMS) && (size == sizeof(struct sysconf_param))){
		pthread_mutex_lock(&apps_vram_sysconfig_lock);
		memcpy((void *)&apps_vram_sysconf,pdata,size);
		pthread_mutex_unlock(&apps_vram_sysconfig_lock);
		vramdbg_info("apps_vram_write OK!\n");
		return 0;
	}

	vramdbg_err("apps_vram_read Error!\n");
	return -1;
}

static char globalvalue_cfgfile[]="/am7x/case/data/global_value.bin";

static void __print_global_value(global_value_t *global_val)
{
	int i;
	for(i=0;i<GLOVAL_VALUE_NUMS_NUM;i++){
		printf("Num[%d]=%d ",i,global_val->nums[i]);
	}
	printf("\n");
	for(i=0;i<GLOVAL_VALUE_STRS_NUM;i++){
		printf("Str[%d]=%s",i,global_val->strs[i]);
	}
}
static int _get_global_value(struct sysconf_param *sys_default)
{
	FILE* fp;
	int i,type;
	int len_read=0;
	int size_read=0;
	fp = fopen(globalvalue_cfgfile,"rb");
	sys_default->is_gloval_valid = 0;
	if(fp==NULL)
	{
		printf("read %s error,please check whether it is exist\n",globalvalue_cfgfile);
		return -1;
	}
	fseek(fp,GLOVALVALUE_INFO_OFFSET,SEEK_SET);
	size_read = sizeof(global_value_t);
	len_read = fread(&sys_default->glo_val,sizeof(char),size_read,fp);
	//printf("Len_read=%d,size_read=%d",len_read,size_read);
	if(len_read!=size_read)//read normal kbd cfgfile
	{
		printf("Read Data Err!\n");
		return -1;
	}
	sys_default->is_gloval_valid = 1;
	fclose(fp);
	__print_global_value(&sys_default->glo_val);
	return 0;
}

static int _set_sysconfig_default(struct sysconf_param *sys_default)
{
	rtc_time_t tmp_rtc;
	memset(&tmp_rtc,0,sizeof(rtc_time_t));
	sys_default->local_language_id = SYSDEFAULT_LOCAL_LANGUAGE;
	sys_default->sys_volume = SYSDEFAULT_VOLUME;
	sys_default->is_udisk_dirty = SYSDEFAULT_IS_UDISK_DIRTY;

	sys_default->backlight_strength = SYSDEFAULT_BACKLIGHT_STRENGTH;

	sys_default->brightness = SYSDEFAULT_COLOR_BIRGHTNESS;
	sys_default->constrast = SYSDEFAULT_COLOR_CONSTRAST;
	sys_default->saturation = SYSDEFAULT_COLOR_SATURATION;
	sys_default->hue = SYSDEFAULT_COLOR_HUE;
	sys_default->sharpness = SYSDEFAULT_COLOR_SHARPNESS;


	sys_default->photo_display_ratio = SYSDEFAULT_PHOTO_DISP_RATIO; 
	sys_default->photo_background_music_en = SYSDEFAULT_PHOTO_BKMUSIC_EN;
	sys_default->photo_background_effect= SYSDEFAULT_PHOTO_BACKGROUND_EFFECT;
	sys_default->photo_slideshow_interval = SYSDEFAULT_PHOTO_SLIDE_INTERVAL;
	sys_default->photo_slideshow_mode = SYSDEFAULT_PHOTO_SLIDE_MODE;
	sys_default->photo_slideshow_effect = SYSDEFAULT_PHOTO_SLIDE_EFFECT;
	sys_default->photo_clockshow_en = SYSDEFAULT_PHOTO_CLOCKSHOW_EN;
	sys_default->photo_autorotation_exif_en = SYSDEFAULT_PHOTO_AUTOROTATION_EN;
	sys_default->photo_autorotation_adhere_en = SYSDEFAULT_PHOTO_AUTOROTATION_EN;
	sys_default->music_effect = SYSDEFAULT_MUSIC_EFFECT;
	sys_default->music_playback_mode = SYSDEFAULT_MUSIC_PLAY_MODE;
	sys_default->audio_autoplay = SYSDEFAULT_AUDIO_AUTOPLAY_MODE;

	sys_default->video_display_ratio = SYSDEFAULT_VIDEO_DISP_RATIO;
	sys_default->video_playback_mode = SYSDEFAULT_VIDEO_PLAY_MODE;
	sys_default->video_autoplay = SYSDEFAULT_VIDEO_AUTOPLAY_MODE;

	sys_default->calendar_clock_mode = SYSDEFAULT_CALENDAR_CLOCK_MODE;

	sys_default->auto_power_on_enable = SYSDEFAULT_AUTO_POWER_ON_ENABLE;
	memcpy(&sys_default->power_on_time,&tmp_rtc,sizeof(rtc_time_t));
	
	sys_default->auto_power_off_enable = SYSDEFAULT_AUTO_POWER_OFF_ENABLE;
	memcpy(&sys_default->power_off_time,&tmp_rtc,sizeof(rtc_time_t));

	sys_default->auto_power_freq = SYSDEFAULT_AUTO_POWER_FREQ_ONCE;

	sys_default->printer_enable = SYSDEFAULT_PRINTER_ENABLE;

	sys_default->hdmi_mode = SYSDEFAULT_SCREEN_OUTPUT_MODE;
	sys_default->cvbs_mode = SYSDEFAULT_SCREEN_OUTPUT_MODE;
	sys_default->ypbpr_mode = SYSDEFAULT_SCREEN_OUTPUT_MODE;

	sys_default->output_enable = SYSDEFAULT_OUTPUT_MODE;

	sys_default->ebook_text_color = SYSDEFAULT_EBOOK_TEXT_COLOR;
	sys_default->ebook_text_size= SYSDEFAULT_EBOOK_TEXT_SIZE;

	sys_default->auto_screen_off_enable = SYSDEFAULT_AUTO_SCREEN_ON_ENABLE;
	sys_default->realtek_wifi_concurrent_softap_ssid_init_flag=0;
	sys_default->realtek_softap_ssid_init_flag=0;
	sys_default->ralink_softap_ssid_init_flag=0;
	sys_default->last_ui=0;
	memset(&sys_default->dmr_title,0,sizeof(sys_default->dmr_title));
	memset(&sys_default->dmr_uuid,0,sizeof(sys_default->dmr_uuid));
	
	memset(&sys_default->alarm_info_head,0,sizeof(alarm_info_head_t));
	_get_global_value(sys_default);
	return 0;
}

/**
* @brief set the vram to default value.
* @return 0-success and others-failed. 
*/
int apps_vram_set_default()
{
	int err;
	
	/**
	* set sysconfig to default value.
	*/
	pthread_mutex_lock(&apps_vram_sysconfig_lock);
	memset(&apps_vram_sysconf, 0, sizeof(apps_vram_sysconf));
	_set_sysconfig_default(&apps_vram_sysconf);
	pthread_mutex_unlock(&apps_vram_sysconfig_lock);
	err = apps_vram_store(VRAM_ID_SYSPARAMS);
	if(err < 0){
		vramdbg_err("apps_vram_store %d\n",err);
		err = -1;
	}
	
	return err;
}



