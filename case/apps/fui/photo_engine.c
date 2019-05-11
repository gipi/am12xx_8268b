#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "swf_types.h"
#include "swfdec.h"
#include "sys_conf.h"
#include "swf_ext.h"
#include "photo_engine.h"
#include "image_decode.h"
#include "load_image_engine.h"
//#include "Apps_vram.h"
#include "act_media_info.h"
#include "apps_vram.h"
#include "system_info.h"
#include "ezcast_public.h"

/******IO Interface*******/
extern void *fui_os_fopen(char *path, char *mode);
extern int fui_os_fclose(void *fp);
extern long fui_os_fread(void *fp, unsigned char *buf, unsigned long nbytes);
extern long fui_os_fwrite(void *fp, unsigned char *ptr, unsigned long nbytes);
extern long fui_os_fseek_set(void *fp, long offset);
extern long fui_os_fseek_cur(void *fp, long offset);
extern long fui_os_fseek_end(void *fp, long offset);
extern long fui_os_ftell(void *fp);
extern void *fui_os_malloc(int size);
extern void fui_os_free(void * pfree);

extern unsigned long fui_get_bus_address(unsigned long logicaddr);
/*************************/
typedef struct pic_decode_result_s{
	unsigned char is_valid;
	IMG_DECODE_INFO_S imginfo;
}pic_decode_result_t;

pic_decode_result_t pic_decode_result;

static unsigned char * decode_buffer=NULL;
static unsigned int decode_bufsize=0;
static void *target=NULL;
static int decode_w = 0,decode_h = 0;

static int decode_mode;
char is_faceinstall=0;
#if EZCAST_ENABLE
char isIconShowing = 0;
#endif

#define EXIF_TMPBUF_LEN 128
static char exif_tmp_buf[EXIF_TMPBUF_LEN];

#ifdef ROTATE_SENSOR
int sensor_stat=SENSOR_ROTATE_H;	
int sensor_en = 0;
#endif

/**
* for face detection
*/

#ifdef _FACE_DETECTION_EN

typedef struct face_func_switch_info{
	unsigned short face_detection:1;
	unsigned short face_beautify:1;
	unsigned short dynamic_lighting:1;
	unsigned short color_enhancement:1;
	unsigned short edge_enhancement:1;
}face_func_switch_info_t;

typedef enum face_func_switch_enum{
	FUNC_FACEDETECTION_ENABLE,
	FUNC_FACEBEAUTIFY_ENABLE,
	FUNC_DYNAMICLIGHTING_ENABLE,
	FUNC_COLORENHANCE_ENABLE,
	FUNC_EDGEENHANCE_ENABLE,
	FUNC_CLEAR_ALL
}face_func_switch_e;

enum{
	POINT_X0=0,
	POINT_Y0=1,
	POINT_X1=2,
	POINT_Y1=3,
};

static FACE_INIT_DATA faceinit;
GLOBAL_DATA_T   face_global_data;
extern FACE_INFO FaceInformation;
face_func_switch_info_t facefuncinfo={0,0,0,0,0};

#endif /**  end _FACE_DETECTION_EN */


PE_ROTATE_STATE image_rotate_state=PE_ROTATE_LEFT_360;

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

static int picDecID=-1;

static int __rotate_cmd_as2midware(int angle)
{
	int cmd=0;
	switch(angle){
		case PROTATE_NONE:
			cmd = BG_IMG_ROT_NONE;
		break;

		case PROTATE_90:
			cmd = BG_IMG_ROT_RIGHT_90;
		break;

		case PROTATE_180:
			cmd = BG_IMG_ROT_180;
		break;

		case PROTATE_270:
			cmd = BG_IMG_ROT_LEFT_90;
		break;

		default:
			printf("%s,%d:Angle Err angle=%d\n",__FILE__,__LINE__,angle);
		break;
	}
	return cmd;
}

static int __rotate_cmd_midware2as(int angle)
{
	int cmd=0;
	switch(angle){
		case BG_IMG_ROT_NONE:
			cmd = PROTATE_NONE;
		break;

		case BG_IMG_ROT_RIGHT_90:
			cmd = PROTATE_90;
		break;

		case BG_IMG_ROT_180:
			cmd = PROTATE_180;
		break;

		case BG_IMG_ROT_LEFT_90:
			cmd = PROTATE_270;
		break;

		default:
			printf("%s,%d:Angle Err angle=%d\n",__FILE__,__LINE__,angle);
		break;
	}
	return cmd;
}

/**
* id ---> a unique identification for a picture
* for decode bitmap or thumb
*/
int get_bitmap_status(int id)
{
	int ret=0;
	int status=1;
	 IMG_DECODE_INFO_S imginfo;
	/** FIXME: add get status */
	ret = img_get_dec_result_by_userid(id,&imginfo);
	if(ret==-1){
		status = 1;
	}
	else{
		status = ret;
	}
	return status;
}

/**
* for decode full
*
*/
int get_pic_dec_status()
{
	int status;
	printf("%s,%d:Sorry this funciton is useless\n",__func__,__LINE__);
	status=get_bitmap_status(picDecID);
	return status;
}

/**
* @brief get the decode mode
*
*/

int get_image_dec_mode()
{
	int mode=0;

	/** FIXME: add get mode */
	mode = get_photo_disp_ratio();
	return mode;
}

unsigned int convert_rorate_status(PE_ROTATE_STATE rotate){	
	unsigned int rtn=0;

	/** FIXME: please finish convert */
	
	switch(rotate){
		case PE_ROTATE_LEFT_90:
			
		break;
		
		case PE_ROTATE_LEFT_180:
			
		break;
		
		case PE_ROTATE_LEFT_270:
			
		break;
		
		case PE_ROTATE_LEFT_360:
			
		break;	

		default:
			
		break;
	}
	return rtn;

}
int is_photo_stream =0 ;

static int pe_open(void * handle){	
	int rtn;
	SWFEXT_FUNC_BEGIN(handle);
	
	//printf("%s,%d:~~~~~~~OPEN IAMGE DEC~~~~~~ ",__func__,__LINE__);
	/** create imge decode thread**/
	is_photo_stream = 0;
	rtn = image_thread_create();
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}


static int pe_decode_image(void *handle)
{
	printf("%s%d:\n",__func__,__LINE__);
	char src[512];
	int rtn=0;	
	SWFEXT_FUNC_BEGIN(handle);
	strcpy(src,Swfext_GetString());

	printf("%s%d:file===%s\n",__func__,__LINE__,src);
	int width=0,height=0;
	width = Swfext_GetNumber();
	height = Swfext_GetNumber();
	printf("pe_decode_image_width=%d,pe_decode_image_height=%d\n",width,height);
	is_photo_stream = 1;


#if 0
	printf("lcd_width=%d,lcd_height=%d\n",lcd_width,lcd_height);
	lcd_width = system_get_screen_para(CMD_GET_SCREEN_WIDTH);
	lcd_height = system_get_screen_para(CMD_GET_SCREEN_HEIGHT);
	lcd_width = 800;
	printf("lcd_width=%d,lcd_height=%d\n",lcd_width,lcd_height);
#endif

	if(decode_buffer){
		if(decode_bufsize<width*height*2){
			SWF_Free(decode_buffer);
			decode_buffer=NULL;
			decode_buffer=SWF_Malloc(width*height*2);
			if(decode_buffer){
				decode_bufsize=width*height*2;
			}
			else{
				decode_bufsize=0;
			}
		}
	}
	else{
		decode_buffer=SWF_Malloc(width*height*2);
		if(decode_buffer){
			decode_bufsize=width*height*2;
		}
		else{
			decode_bufsize=0;
		}
	}
	printf("%s,%d:decode_buffer is 0x%x\n",__FILE__,__LINE__,decode_buffer);
#if 1
	if(decode_buffer){
		int ret=0;
		int filesize=0;
		int decode_image_id= 0x123456;
		//int mode = get_image_dec_mode();
		//printf("%s,%d:decode mode is %d\n",__FILE__,__LINE__,mode);
		//mode = DEC_MODE_FULL_SCREEN;
		IMG_DECODE_INFO_S imginfo;
		if(img_dec_send_cmd(BG_IMG_DEC_FULL2BUF,DEC_MODE_LBOX,(void*)src,decode_buffer,width,height,decode_image_id)==-1){
			rtn = 0;
			goto PE_DECODE_IMAGE_OUT;
		}
			

		while(1){//wait until the decoding is completed
			ret = img_get_dec_result_by_userid(decode_image_id,&imginfo);
			if(ret==-1){
				rtn = 0;
				goto PE_DECODE_IMAGE_OUT;
			}
			else if(ret == 0){
				_img_thread_sleep(100);
				continue;
			}
			else
				break;
		}
#if 0
		//add for dubug
		FILE*fp;
		if((fp=fopen("/mnt/udisk/imagetest.yuv","wb")))
		{
			printf("open file ok\n");
			fwrite(decode_buffer,sizeof(char),width*height*2,fp);
			fsync(fileno(fp));
			fclose(fp);
		}
#endif				

	}
	else{
		rtn = -1;
	}
	
PE_DECODE_IMAGE_OUT:
	if(decode_buffer)
		SWF_Free(decode_buffer);
#endif	
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
	
}
#include "display.h"
extern void *deinst;
 static DE_config ds_conf;
static int set_de_defaultcolor()
{
       
	de_get_config(deinst,&ds_conf,DE_CFG_ALL);
	ds_conf.input.enable=0;
	de_set_Config(deinst,&ds_conf,DE_CFG_ALL);
	
	return 0;
}


static int pe_start_bmp(void * handle){	
	int rtn=1;
	SWFEXT_FUNC_BEGIN(handle);
	
	printf("%s,%d:~~~~~~~OPEN IAMGE DEC~~~~~~ ",__func__,__LINE__);
	start_bmp();
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
static int pe_exit_bmp(void * handle){
	
	int ret_close = 1;
	SWFEXT_FUNC_BEGIN(handle);
	
	/** FIXME: something should do here */
	exit_bmp();
	Swfext_PutNumber(ret_close);
	
	SWFEXT_FUNC_END();
}
int bmp_dec_finished=0;
char path[512];
int bmp_height;
int bmp_width;
void  *bmp_dec(void *arg)
{
   	int rtn=0;
	int input_pix_fmt=0;
	int input_width=0;
	int input_height=0;
	int output_pip_x=0;
	int output_pip_y=0;
	if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE)
	{
		
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
	}
    //   wait_dec_bmp();
        set_de_defaultcolor();
	/**
	* set fui to sleep.
	*/		 
   
	if(decode_buffer){
		if(decode_bufsize<bmp_width*bmp_height*4){
			SWF_Free(decode_buffer);
			decode_buffer=NULL;
			decode_buffer=SWF_Malloc(bmp_width*bmp_height*4);
			if(decode_buffer){
				decode_bufsize=bmp_width*bmp_height*4;
			}
			else{
				decode_bufsize=0;
			}
		}
	}
	else{
		decode_buffer=SWF_Malloc(bmp_width*bmp_height*4);
		if(decode_buffer){
			decode_bufsize=bmp_width*bmp_height*4;
		}
		else{
			decode_bufsize=0;
		}
	}
	printf("%s,%d:decode_buffer is 0x%x\n",__FILE__,__LINE__,decode_buffer);
	if(decode_buffer){	
		FILE*fp;
	            //  FILE*fp1;	  
		if(fp=fopen(path,"rb")){
			printf("open file ok\n");
			fread(decode_buffer,sizeof(char),bmp_width*bmp_height*4,fp);
			fclose(fp);
			/*fp1=fopen("/mnt/udisk/1.bin","wb");
			fwrite(decode_buffer,sizeof(char),width*height*4,fp1);
			fclose(fp1);*/
		 }
		de_get_config(deinst,&ds_conf,DE_CFG_ALL);
		printf("%s,%d:ds_conf.input.pix_fmt is %d\n",__FILE__,__LINE__,ds_conf.input.pix_fmt);
		
		#if 1
		  //  pre_pix=ds_conf.input.pix_fmt;
		   //  pre_colorspace= ds_conf.colorspace;
		input_pix_fmt=ds_conf.input.pix_fmt;
		input_width=ds_conf.input.width;
		input_height=ds_conf.input.height;
		output_pip_x=ds_conf.output.pip_x;
		output_pip_y=ds_conf.output.pip_y;
		ds_conf.input.pix_fmt=PIX_FMT_RGB32;
		ds_conf.input.width = bmp_width;//ds_conf.dev_info[1].width;
		ds_conf.input.height= bmp_height;//ds_conf.dev_info[1].height;
		ds_conf.output.pip_x=(ds_conf.dev_info[1].width-bmp_width)/2;
		ds_conf.output.pip_y=(ds_conf.dev_info[1].height-bmp_height)/2;

		printf("------------------------Display info------------------\n");
		printf("ds_conf.dev_info[1].width = %d ds_conf.dev_info[1].height = %d\n",ds_conf.dev_info[1].width,ds_conf.dev_info[1].height);
		printf("ds_conf.input.width = %d ds_conf.input.height = %d\n",ds_conf.input.width,ds_conf.input.height);
		printf("pip_x=%d\n",ds_conf.output.pip_x);
		printf("pip_y=%d\n",ds_conf.output.pip_y);
		printf("display_mode=%d\n",ds_conf.output.display_mode);
		printf("output_mode=%d\n",ds_conf.output.output_mode);
		printf("pip_width=%d\n",ds_conf.output.pip_width);
		printf("pip_height=%d\n",ds_conf.output.pip_height);
		printf("dar_width=%d\n",ds_conf.output.dar_width);
		printf("dar_height=%d\n",ds_conf.output.dar_height);
		printf("----------------------------------------------------\n");
		#endif
		ds_conf.input.enable = 1;
		//ds_conf.input.default_color= 0xff00;
		ds_conf.input.img = (unsigned long*)decode_buffer;
		int i;
		unsigned int* tempAddr=ds_conf.input.img;
		printf("@@@@@@@@@@@%d %d 0x%x\n",ds_conf.dev_info[1].width,ds_conf.dev_info[1].height,ds_conf.input.img);
		/*for (i=0;i<ds_conf.dev_info[1].width*ds_conf.dev_info[1].height/3;i++)
		{
			*tempAddr = 0xff0000;
			tempAddr++;
		}
		for (i=0;i<ds_conf.dev_info[1].width*ds_conf.dev_info[1].height/3;i++)
		{
			*tempAddr = 0xff00;
			tempAddr++;
		}
		for (i=0;i<ds_conf.dev_info[1].width*ds_conf.dev_info[1].height/3;i++)
		{
			*tempAddr = 0xff;
			tempAddr++;
		}*/
		printf("-----------------------\n");
		ds_conf.input.bus_addr = fui_get_bus_address((unsigned long)decode_buffer);
		printf("ds_conf.input.bus_addr=0x%x\n",ds_conf.input.bus_addr);
		de_set_Config(deinst,&ds_conf,DE_CFG_IN);
		printf("%s,%d:ds_conf.input.pix_fmt is %d\n",__FILE__,__LINE__,ds_conf.input.pix_fmt);
	
		while(1){//wait until the decoding is completed

			if(bmp_dec_finished==1){
				rtn = 0;
				goto PE_DECODE_BMP_OUT;
		      	}
			else if(bmp_dec_finished == 0){
			//	OSSleep(100);
			//   printf("tttttttttttttttttttttttttttttt\n");
				_img_thread_sleep(100);
				continue;
			  }
			else
				break;
	        }
			
		printf("%s,%d:ds_conf.input.pix_fmt is %d bmp_dec_finished=%d\n",__FILE__,__LINE__,ds_conf.input.pix_fmt,bmp_dec_finished);
	
	}
	else{
		rtn = -1;
	}
	
PE_DECODE_BMP_OUT:
	/**
	* cannot malloc memory for video, just go out.
	*/
	 
	if(decode_buffer)
		SWF_Free(decode_buffer);
	de_get_config(deinst,&ds_conf,DE_CFG_ALL);
	ds_conf.input.enable = 0;

	printf("%s,%d:ds_conf.input.pix_fmt is %d\n",__FILE__,__LINE__,ds_conf.input.pix_fmt);
	// ds_conf.input.default_color= 0x000000;//
	ds_conf.input.pix_fmt=input_pix_fmt;
       	ds_conf.input.width= input_width;
	ds_conf.input.height= input_height;
       	ds_conf.output.pip_x=  output_pip_x;
       	ds_conf.output.pip_y=output_pip_y;
	         
	de_set_Config(deinst,&ds_conf,DE_CFG_IN);
	printf("%s,%d:ds_conf.input.pix_fmt is %d \n",__FILE__,__LINE__,ds_conf.input.pix_fmt);
	printf("%s,%d:ds_conf.input.pix_fmt is  %d bmp_dec_finished=%d\n",__FILE__,__LINE__,ds_conf.input.pix_fmt,bmp_dec_finished);
	if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0){
		printf("%s,%d:ds_conf.input.pix_fmt is %d\n",__FILE__,__LINE__,ds_conf.input.pix_fmt);
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	}
	bmp_thread_exit();

}
static int pe_decode_bmp(void *handle)
{
	printf("%s%d:\n",__func__,__LINE__);
	//char *src;
	int rtn=0;	
	int pre_colorspace=0;
	SWFEXT_FUNC_BEGIN(handle);
	strcpy(path,Swfext_GetString());
     // src=Swfext_GetString();
	//memcpy(src);
	printf("%s%d:file===%s\n",__func__,__LINE__,path);
	int width=0,height=0;
	width = Swfext_GetNumber();
	height = Swfext_GetNumber();
	bmp_height=height;
	bmp_width=width;
	printf("pe_decode_image_width=%d,pe_decode_image_height=%d\n",width,height);
	creat_bmp_thread();  
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int pe_attach(void * handle){
	int w,h;
	SWFEXT_FUNC_BEGIN(handle);
	target   = Swfext_GetObject();
	int width=0,height=0;
	width = Swfext_GetNumber();
	height = Swfext_GetNumber();
	printf("%s,%d:Attach @@@@@@@@",__func__,__LINE__);
#if 0
	printf("lcd_width=%d,lcd_height=%d\n",lcd_width,lcd_height);
	width = system_get_screen_para(CMD_GET_SCREEN_WIDTH);
	height = system_get_screen_para(CMD_GET_SCREEN_HEIGHT);
	printf("lcd_width=%d,lcd_height=%d\n",lcd_width,lcd_height);
#endif
	printf("pe_attach_width=%d,pe_attach_height=%d\n",width,height);

	SWF_AttachBitmap(target,(unsigned int*)decode_buffer,width,height,width,height,width,SWF_BMP_FMT_YUV422,NULL);
	
	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();
}

static int pe_detach(void * handle){
	
	SWFEXT_FUNC_BEGIN(handle);
	int clone;
	void *target=NULL;

	printf("%s,%d:Detatch Do nothing",__func__,__LINE__);
	target = Swfext_GetObject();
	clone = Swfext_GetNumber();

	if(target != NULL){
		SWF_DetachBitmap(target,clone);		
	}
	
	if(decode_buffer)
	{
		SWF_Free(decode_buffer);
		decode_buffer = NULL;
		decode_bufsize = 0;
	}
	
	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();
}

static int pe_show(void * handle){
	
	SWFEXT_FUNC_BEGIN(handle);
	
	printf("%s,%d:Show  Do nothing",__func__,__LINE__);
	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();
}

/**
* @brief
		prototype: "StopDecode(int cmd)"
* @param
	cmd stop command: 
		0 indicates just stopping current task
		1 indicates stopping all decode tasks
*/
static int pe_stop_decode(void * handle){

	int cmd=0;
	SWFEXT_FUNC_BEGIN(handle);
	cmd = Swfext_GetNumber();
	if(cmd!=0){
		/** FIXME: [clear all pending tasks] */
	}
	img_dec_io_ctrl(BG_IMG_DEC_STOP,NULL);
	SWFEXT_FUNC_END();
}


/**
* @brief "GetStatus" actually returns the status of the photo
* which has been called by "Show". 
*       
*/

static int pe_get_status(void * handle){
	
	SWFEXT_FUNC_BEGIN(handle);
	
	printf("%s,%d:Sorry get Staus is usefull",__func__,__LINE__);
	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();
}

static int pe_close(void * handle){
	
	int ret_close = 1;
	SWFEXT_FUNC_BEGIN(handle);
	
	/** FIXME: something should do here */
	//printf("%s,%d:~~~~~~~~~~Close IMAGE DEC~~~~~~",__func__,__LINE__);
	ret_close=image_thread_exit();
	is_photo_stream = 0;
	Swfext_PutNumber(ret_close);
	
	SWFEXT_FUNC_END();
}


static int pe_get_play_mode(void * handle){
	
	int mode = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	/** FIXME: [finish it]*/
	printf("%s,%d:Get play mode Do nothing",__func__,__LINE__);
	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();
}

static int pe_set_play_mode(void * handle){
	
	int mode = 0;

	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	printf("%s,%d:Get play mode Do nothing",__func__,__LINE__);
	mode= Swfext_GetNumber(0);

	SWFEXT_FUNC_END();
}


static int pe_get_music_enable(void * handle){
	
	int en = 0;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	en = get_photo_bk_music_en();
	Swfext_PutNumber(en);
	
	SWFEXT_FUNC_END();
}

static int pe_set_music_enable(void * handle){
	
	int en = 0;

	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	en= Swfext_GetNumber();
	set_photo_bk_music_en(en);

	SWFEXT_FUNC_END();
}

static int pe_get_play_ratio(void * handle){
	
	int ratio=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	ratio = get_image_dec_mode();
	Swfext_PutNumber(ratio);
	
	SWFEXT_FUNC_END();
}

static int pe_set_play_ratio(void * handle){
	
	int ratio=0;

	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	ratio = Swfext_GetNumber();
	set_photo_disp_ratio(ratio);
	
	SWFEXT_FUNC_END();
}


static int pe_set_autorotationExif_en(void *handle)
{
	int auto_rotation=0;

	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	auto_rotation = Swfext_GetNumber();
	set_photo_autorotation_exif_en(auto_rotation);
	
	SWFEXT_FUNC_END()
}

static int pe_get_autorotationExif_en(void*handle)
{
	int auto_rotation=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	auto_rotation = get_photo_autorotation_exif_en();
	Swfext_PutNumber(auto_rotation);
	
	SWFEXT_FUNC_END();
}


static int pe_set_autorotationAdhere_en(void *handle)
{
	int auto_rotation=0;

	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	auto_rotation = Swfext_GetNumber();
	set_photo_autorotation_adhere_en(auto_rotation);
	
	SWFEXT_FUNC_END()
}

static int pe_get_autorotationAdhere_en(void*handle)
{
	int auto_rotation=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	auto_rotation = get_photo_autorotation_adhere_en();
	Swfext_PutNumber(auto_rotation);
	
	SWFEXT_FUNC_END();
}

static int pe_set_background_effect(void *handle)
{
	int bkg_effect=0;

	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	bkg_effect = Swfext_GetNumber();
	set_photo_background_effect(bkg_effect);
	
	SWFEXT_FUNC_END();
}

static int pe_get_background_effect(void *handle)
{
	int bkg_effect=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	bkg_effect = get_photo_background_effect();
	Swfext_PutNumber(bkg_effect);
	
	SWFEXT_FUNC_END();
}

static int pe_get_slideshow_time(void * handle){
	
	int time=10;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	time = get_photo_slideshow_interval();
	Swfext_PutNumber(time);
	
	SWFEXT_FUNC_END();
}

static int pe_set_slideshow_time(void * handle){
	
	int time=10;

	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	time= Swfext_GetNumber();
	set_photo_slideshow_interval(time);
	SWFEXT_FUNC_END();
}
/*add by richard 04142012*/
static int pe_get_slideshow_mode(void * handle){
	
	int mode=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	mode= get_photo_slideshow_mode();
	Swfext_PutNumber(mode);
	
	SWFEXT_FUNC_END();
}

static int pe_set_slideshow_mode(void * handle){
	
	int mode=0;

	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	mode= Swfext_GetNumber();
	set_photo_slideshow_mode(mode);
	SWFEXT_FUNC_END();
}
/*add by richard 04142012*/
static int pe_get_slideshow_effect(void * handle){
	
	int effect=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	effect = get_photo_slideshow_effect();
	printf("[%s %d] effect ================= %d\n",__FILE__,__LINE__,effect);
	Swfext_PutNumber(effect);
	
	SWFEXT_FUNC_END();
}

static int pe_set_slideshow_effect(void * handle)
{
	int effect=0;

	SWFEXT_FUNC_BEGIN(handle);

	effect= Swfext_GetNumber();
	
	printf("[%s %d] effect ================= %d\n",__FILE__,__LINE__,effect);
	set_photo_slideshow_effect(effect);

	SWFEXT_FUNC_END();
}

static int pe_get_decoded_pic_info(void *handle)
{
	int info=0,cmd=0;
	int rtn=-1;
	SWFEXT_FUNC_BEGIN(handle);

	cmd = Swfext_GetNumber();
	if(pic_decode_result.is_valid){
		switch(cmd){
			case PE_GET_SRC_WIDTH:
				rtn = pic_decode_result.imginfo.src_width;
				break;
			case PE_GET_SRC_HEIGHT:
				rtn = pic_decode_result.imginfo.src_height;
				break;
			case PE_GET_SCALE_RATE:
				rtn = pic_decode_result.imginfo.cur_scale_rate;
				break;
			case PE_GET_ACTUAL_WIDTH:
				rtn = pic_decode_result.imginfo.img_actual_wid;
				break;
			case PE_GET_ACTUAL_HEIGHT:
				rtn = pic_decode_result.imginfo.img_actual_wid;
				break;
			default:
				printf("%s,%d: cmd Error!cmd=%d\n",__FILE__,__LINE__,cmd);
		}
	}
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int pe_get_decoded_err(void *handle){
	
	int rtn;
	unsigned int param_nr=0;
	char *filename;
	
	SWFEXT_FUNC_BEGIN(handle);

	param_nr = Swfext_GetParamNum();

	/** FIXME: [get decode error status]*/
	if(param_nr == 0){
		/** get decoder stat that by "Show" */
		printf("%s,%d:Sorry please input filename\n",__func__,__LINE__);
		rtn = DECODED_ERROR;
	}
	else{
		/** get decoder stat that by "LoadClip" */
		filename = Swfext_GetString();
		//printf("%s,%d:name=%s\n",__func__,__LINE__,filename);
		rtn = img_get_dec_result(filename,&pic_decode_result.imginfo);
		pic_decode_result.is_valid = 0;
		if(rtn==-1){
			printf("%s,%d:Crazy file name can't be found\n",__func__,__LINE__);
			rtn = DECODED_NOT_SUPPORT;
		}
		else if(rtn==0)
			rtn = NO_DECODED;
		else if(rtn==1){
			rtn = pic_decode_result.imginfo.result;
			pic_decode_result.is_valid = 1;
		}
		
	}
	//printf("[~~~~~~~~]%s,%d:Get Decode Result=%d,isready=%d\n",__func__,__LINE__,rtn,imginfo.img_ready);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}


static int pe_get_clock_enable(void * handle){
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	Swfext_PutNumber(0);
	printf("%s,%d:Get clock enable  Do nothing",__func__,__LINE__);
	
	SWFEXT_FUNC_END();
}

static int pe_set_clock_enable(void * handle){
	
	int en=0;

	SWFEXT_FUNC_BEGIN(handle);
	en = Swfext_GetNumber();
	printf("%s,%d:Set clock enable  Do nothing",__func__,__LINE__);

	SWFEXT_FUNC_END();
}

static int pe_attach_face(void *handle)
{
#ifdef  _FACE_DETECTION_EN

	SWFEXT_FUNC_BEGIN(handle);

	faceinit.faceHeapAddr = 0;
	/** 1.5M space for face detection */
	faceinit.faceHeapSize = (unsigned int)0x180000;
	faceinit.faceHeapAddr = (unsigned int)((char*)SWF_Malloc(faceinit.faceHeapSize));

	if((char*)faceinit.faceHeapAddr == NULL){
		printf("face detection malloc failed\n");
		Swfext_PutNumber(0);
	}
	else{
		is_faceinstall=1;
		Swfext_PutNumber(1);
	}
	
	SWFEXT_FUNC_END();
	
#else

	SWFEXT_FUNC_BEGIN(handle);

	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();
	
#endif
}

static int pe_detach_face(void *handle)
{
#ifdef _FACE_DETECTION_EN

	SWFEXT_FUNC_BEGIN(handle);

	if((char*)faceinit.faceHeapAddr!=NULL){
		SWF_Free((char*)faceinit.faceHeapAddr);
		is_faceinstall =0;
		faceinit.faceHeapAddr = 0;
	}
	
	Swfext_PutNumber(1);
	
	SWFEXT_FUNC_END();
	
#else
	SWFEXT_FUNC_BEGIN(handle);

	Swfext_PutNumber(1);

	SWFEXT_FUNC_END();
#endif
}

static int pe_set_face_func_switch(void *handle)
{
#ifdef _FACE_DETECTION_EN
	int whichFunc;

	SWFEXT_FUNC_BEGIN(handle);
	
	whichFunc = Swfext_GetNumber();
	
	switch(whichFunc){
		case FUNC_CLEAR_ALL:
			memset(&facefuncinfo,0,sizeof(face_func_switch_info_t));
			break;
		case FUNC_FACEDETECTION_ENABLE:
			facefuncinfo.face_detection = 1;
			break;
		case FUNC_FACEBEAUTIFY_ENABLE:
			facefuncinfo.face_beautify = 1;
			break;
		case FUNC_DYNAMICLIGHTING_ENABLE:
			facefuncinfo.dynamic_lighting = 1;
			break;
		case FUNC_COLORENHANCE_ENABLE:
			facefuncinfo.color_enhancement = 1;
			break;
		case FUNC_EDGEENHANCE_ENABLE:
			facefuncinfo.edge_enhancement = 1;
			break;
		default:
			break;
	}
	SWFEXT_FUNC_END();
#else
	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();
#endif 
}

static int pe_get_face_func_switch(void *handle)
{
#ifdef _FACE_DETECTION_EN
	int funcstatus;

	SWFEXT_FUNC_BEGIN(handle);
	
	funcstatus =*(int*)&facefuncinfo;
 	Swfext_PutNumber(funcstatus);
	
	SWFEXT_FUNC_END();
#else
	SWFEXT_FUNC_BEGIN(handle);

	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();
#endif 
}

static int pe_detect_face(void *handle)
{
#ifdef _FACE_DETECTION_EN
	int type=0;
	char * filename=NULL;
	FACE_INFO faceinfo;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	type = Swfext_GetParamType();
	if(type == SWFDEC_AS_TYPE_NULL){
		printf("Error,filename ==NULL\n");
		Swfext_PutNumber(0);
	}
	else{
		filename = Swfext_GetString();
		
		/** FIXEME: [add face detection here]*/
		
		Swfext_PutNumber(0);
	}
	SWFEXT_FUNC_END();
#else
	SWFEXT_FUNC_BEGIN(handle);

	Swfext_PutNumber(0);

	SWFEXT_FUNC_END();
#endif
}

static int pe_get_face_rect(void *handle)
{
#ifdef _FACE_DETECTION_EN
	int i,point;

	SWFEXT_FUNC_BEGIN(handle);
	
	i = Swfext_GetNumber();
	point = Swfext_GetNumber();
	
	if(FaceInformation.FaceNumber!=0&&FaceInformation.FaceNumber>i){
		switch(point){
			case POINT_X0:
				Swfext_PutNumber(FaceInformation.Rect[i].x0);
				break;
			case POINT_X1:
				Swfext_PutNumber(FaceInformation.Rect[i].x1);
				break;
			case POINT_Y0:
				Swfext_PutNumber(FaceInformation.Rect[i].y0);
				break;
			case POINT_Y1:
				Swfext_PutNumber(FaceInformation.Rect[i].y1);
				break;
			default:
				printf("%s,%d:Error Point number\n",__FILE__,__LINE__);
				Swfext_PutNumber(0);		
		}
	}
	SWFEXT_FUNC_END();
#else
	SWFEXT_FUNC_BEGIN(handle);

	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();
#endif
}


static int pe_prepare_for_delete(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [may do something]*/
	img_dec_io_ctrl(BG_IMG_DEC_STOP,NULL);

	
	SWFEXT_FUNC_END();
}

/**
* @brief for write EXIF information to encoded file
*/
char *tmp_buffer=NULL;

char exifinfo[74] = 
{
	0xFF, 0xE1, 0x00, 0x48, 0x45, 0x78, 0x69, 0x66, 0x00, 0x00, 
	0x49, 0x49, 0x2a, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x69,
	0x87, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x90, 0x02, 0x00, 0x14,
	0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define TMP_BUFFER_SIZE (10*1024)

static int write_exif(char *filename,char *data, int data_size, int offset)
{
	int move_size;
	FILE *fptr;
	int file_len;

	fptr = fopen(filename, "rw");
	if(fptr == NULL){
		return -1;
	}

	fseek(fptr,0,SEEK_END);
	file_len = ftell(fptr);
	
	if(offset > file_len){
		fclose(fptr);
		return -1;
	}

	if(data_size > TMP_BUFFER_SIZE)
	{
		int tmp_size = data_size-TMP_BUFFER_SIZE;
		
		if (0 != fseek(fptr, file_len, SEEK_SET))
		{
			printf("fseek error!\n");
			fclose(fptr);
			return -1;
		}
		
		while(tmp_size > 0)
		{
			int write_size = TMP_BUFFER_SIZE;

			if(write_size > tmp_size){
				write_size = tmp_size;
			}

			fwrite(tmp_buffer, sizeof(char), write_size, fptr);

			tmp_size -= write_size;
		}
		
	}


	move_size = file_len - offset;
	
	while(move_size > 0)
	{
		int seek_pos = offset + move_size - TMP_BUFFER_SIZE;
		int read_size = TMP_BUFFER_SIZE;
		
		if(seek_pos < offset)
		{
			seek_pos = offset;
			read_size = move_size;
		}
		
		if (0 != fseek(fptr, seek_pos, SEEK_SET))
		{
			printf("fseek error!\n");
			fclose(fptr);
			return -1;
		}
		
		//if(S_FS_FRead(tmp_buffer, read_size, fptr) != read_size)
		if(fread(tmp_buffer, sizeof(char),read_size, fptr) != read_size)
		{
			printf("fread error!\n");
			fclose(fptr);
			return -1;
		}
		
		if (0 != fseek(fptr, seek_pos+data_size, SEEK_SET))
		{
			printf("fseek error!\n");
			fclose(fptr);
			return -1;
		}
		
		fwrite(tmp_buffer, sizeof(char), read_size, fptr);
		
		move_size -= read_size;
	}


	if (0 != fseek(fptr, offset, SEEK_SET))
	{
		printf("fseek error!\n");
		fclose(fptr);
		return -1;
	}

	fwrite(data, sizeof(char), data_size, fptr);
	fclose(fptr);

	return 0;
}

static int convert_dateinfo_to_str(char *str,int year,int month,int day)
{
	/** YYYY:MM:DD HH:MM:SS0x00 */
	if(str == NULL){
		return -1;
	} 
	/** should keep every byte smaller than 10 */
	str[0] = 0x30 + (year/1000)%10;
	str[1] = 0x30 + (year%1000)/100;
	str[2] = 0x30 + (year%100)/10;
	str[3] = 0x30 + (year%10);
	str[4]  = ':';
	str[5]  = 0x30 + (month/10)%10;
	str[6]  = 0x30 + (month%10);
	str[7]  = ':';
	str[8]  = 0x30 + (day/10)%10;
	str[9]  = 0x30 + (day%10);
	str[10] = ' ';
	str[11] = 0x30;
	str[12] = 0x30;
	str[13] = ':';
	str[14] = 0x30;
	str[15] = 0x30;
	str[16] = ':';
	str[17] = 0x30;
	str[18] = 0x30;
	str[19] = 0x0;
	
}


/**
* @brief check some status before resize copy.
*/
static int pe_check_resize_copy(void * handle)
{
	char src[256],dst[256];
	char *p,*prev;
	FILE *fhandle = NULL;
	DIR *hdir = NULL;
	int rtn=0;
	unsigned long long size=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** 
	* get valid destination path 
	*/
	strcpy(src,Swfext_GetString());
	strcpy(dst,Swfext_GetString());

	/** 
	* path for copy error 
	*/
	hdir = opendir(dst);
	if(hdir == NULL){
		// rtn = FS_ERR_Path;
		goto CHECK_RC_OUT;
	}
	closedir(hdir);

	/**
	* refactoring the path
	*/
	p = src;
	do{
		p++;
		prev = p;
		p = strstr(p,"/");
	}while(p!=NULL);
	strcat(dst,prev);

	/** 
	* check if file exists 
	*/
	fhandle = fopen(dst,"r");
	if(fhandle){
		/** rtn = FS_ERR_FileExist;*/
		fclose(fhandle);
		goto CHECK_RC_OUT;
	}
	
	/** 
	* check if disk full,currently if size < 300K,
	* we consider it full 
	*/
	

	// rtn = FS_ERR_OK;
	
CHECK_RC_OUT:

	/**
	* translate the return value
	*/
	//Swfext_PutNumber(TranslateFSError(rtn));
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}

unsigned char * __do_extract_actual_pic(const unsigned char*buffer,int buf_width,int buf_height,IMG_DECODE_INFO_S* imginfo)
{
	int actual_width = imginfo->img_actual_wid;
	int actual_height = imginfo->img_actual_hei;
	int start_x = imginfo->img_start_x;
	int start_y = imginfo->img_start_y;
	unsigned char * pic_buffer=NULL;
	unsigned int offset=0;
	unsigned int buf_offset=0;
	pic_buffer = (unsigned char *)SWF_Malloc(actual_width*actual_height*2);
	if(pic_buffer){
		int i=0;
		buf_offset = start_x*2+buf_width*2*start_y;
		for(i=0;i<actual_height;i++){
			memcpy(pic_buffer+offset,buffer+buf_offset+i*buf_width*2,actual_width*2);
			offset +=actual_width*2;
		}
	}
	else
		printf("%s,%d:Malloc Failed!\n",__FILE__,__LINE__);
	return pic_buffer;
}

/**
@brief call this function to resize copy the photo
@param[in] src: the file full path which will be copied
@param[in] dst : the file name which will be saved
@return
	1 	: success 
	0	: failed
**/
int __do_resize_copy(const char *src,const char* dst)
{
	unsigned char* decbuffer = NULL;
	int rtn=0;
	int lcd_width=0,lcd_height=0;
	printf("lcd_width=%d,lcd_height=%d\n",lcd_width,lcd_height);
	lcd_width = system_get_screen_para(CMD_GET_SCREEN_WIDTH);
	lcd_height = system_get_screen_para(CMD_GET_SCREEN_HEIGHT);
	decbuffer=(unsigned char*)SWF_Malloc(lcd_width*lcd_height*2);
	if(decbuffer){
		int ret=0;
		int filesize=0;
		int resize_copy_id= 0x123456;
		int mode = get_image_dec_mode();
		IMG_DECODE_INFO_S imginfo;
		if(img_dec_send_cmd(BG_IMG_DEC_FULL2BUF,mode,(void*)src,decbuffer,lcd_width,lcd_height,resize_copy_id)==-1){
			rtn = 0;
			goto DO_RESIZE_COPY_OUT;
		}
			

		while(1){//wait until the decoding is completed
			ret = img_get_dec_result_by_userid(resize_copy_id,&imginfo);
			if(ret==-1){
				rtn = 0;
				goto DO_RESIZE_COPY_OUT;
			}
			else if(ret == 0){
				_img_thread_sleep(100);
				continue;
			}
			else
				break;
		}
		
		if(jpeg_encode(dst,(void*)decbuffer,NULL,NULL,lcd_width*2,4,imginfo.img_start_x,imginfo.img_start_y,
			imginfo.img_actual_wid,imginfo.img_actual_hei,3,&filesize,1)!=0){
			printf("%s,%d:Resize Copy Error!\n",__FILE__,__LINE__);
			rtn = 0;
			goto DO_RESIZE_COPY_OUT;
		}
		else{
			printf("%s,%d:Resize Copy OK Size=0x%x!\n",__FILE__,__LINE__,filesize);
			rtn = 1;
		}	
	}
	else
		rtn = 0;

DO_RESIZE_COPY_OUT:
	if(decbuffer)
		SWF_Free(decbuffer);
	return rtn;
}

static int pe_do_resize_copy(void * handle)
{
	unsigned int param_nr;
	char src[128],dst[128];
	char *p,*prev;
	DIR *hdir = NULL;
	int rtn=0;
	
	unsigned char flag=0;
	unsigned int encSize=0;
	char datainfo[20];
	int i;

	
	SWFEXT_FUNC_BEGIN(handle);

	/* 
	* check param validation
	*/
	param_nr = Swfext_GetParamNum();
	if(param_nr != 2){
		rtn=0;
		goto RESIZE_COPY_OUT;
	}

	/** 
	* get valid destination path 
	*/
	strcpy(src,Swfext_GetString());
	strcpy(dst,Swfext_GetString());

	hdir = opendir(dst);
	if(hdir == NULL){
		rtn=0;
		printf("dst not dir\n");
		goto RESIZE_COPY_OUT;
	}
	closedir(hdir);

	p = src;
	do{
		p++;
		prev = p;
		p = strstr(p,"/");
	}while(p!=NULL);
	strcat(dst,prev);

	/** FIXME: [do resize copy]*/
	printf("%s,%d:src=%s,dst=%s\n",__FILE__,__LINE__,src,dst);
	rtn = __do_resize_copy(src,dst);
	
RESIZE_COPY_OUT:
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}



/**
* @brief prototype is "StoreRotation(file,rotation)";
*/
static int pe_store_rotation(void * handle)
{
	unsigned int nparam;
	char *filename=NULL;
	int angle=0;
	int rtn=0;
	int rotation;

	
	SWFEXT_FUNC_BEGIN(handle);

	/** check param validation */
	nparam = Swfext_GetParamNum();
	if(nparam != 2){
		goto STORE_ROTATION_OUT;
	}

	/** get valid destination path */
	filename = Swfext_GetString();
	angle = Swfext_GetNumber();
	rotation = __rotate_cmd_as2midware(angle);
	if(img_store_photo_exif(filename,SET_EXIF_ROTATION,rotation)==0){
		rtn = 1;
	}
	
STORE_ROTATION_OUT:
	
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

/**
* @brief prototype is "GetRotation(file)";
*/
static int pe_get_rotation(void * handle)
{
	unsigned int nparam;
	char * filename=NULL;
	int angle = 0;
	PHOTO_DB_T db;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** check param validation */
	nparam = Swfext_GetParamNum();
	if(nparam != 1){
		goto STORE_ROTATION_OUT;
	}

	// get valid destination path
	filename = Swfext_GetString();
	
	if(img_read_photo_exif(filename,&db)==0){
		angle = __rotate_cmd_midware2as(db.rotation);
	}
	
STORE_ROTATION_OUT:
	
	Swfext_PutNumber(angle);
	SWFEXT_FUNC_END();
}


/**
* @brief prototype is "StoreEffect(file,effect)";
*/
static int pe_store_effect(void * handle)
{
	unsigned int nparam;
	char filename[128];
	int rtn=0;
	int effect;

	
	SWFEXT_FUNC_BEGIN(handle);

	// check param validation
	nparam = Swfext_GetParamNum();
	if(nparam != 2){
		goto STORE_EFFECT_OUT;
	}

	// get valid destination path
	strcpy(filename,Swfext_GetString());
	effect = Swfext_GetNumber();

	if(img_store_photo_exif(filename,SET_EXIF_EFFECT,effect)==0){
		rtn = 1;
	}
	
STORE_EFFECT_OUT:
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int pe_store_process_effect(void * handle)
{
	int effect;

	SWFEXT_FUNC_BEGIN(handle);
	
	effect = Swfext_GetNumber();

	/** FIXME: [do something]*/

	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}

/**
* @brief photo zoom via decoder.
*/
static int pe_zoom(void * handle)
{
	char *file;
	int index;
	int cmd,newcmd;
	
	SWFEXT_FUNC_BEGIN(handle);

	file = Swfext_GetString();
	cmd = Swfext_GetNumber();
	
	/** FIXME: [do zoom via decoder]*/

	index = (int)img_dec_req_query(QUE_CMD_FILENAME,file);
	if(index == -1){
		goto PZOOM_OUT;
	}
	switch(cmd){
		case PE_RATIO_ZOOMOUT:
			newcmd = BG_IMG_ZOOM_OUT;
			break;
		case PE_RATIO_ZOOMIN:
			newcmd = BG_IMG_ZOOM_IN;
			break;
		case PE_RATIO_RESIZE:
			newcmd = BG_IMG_ZOOM_RESET;
			break;
		case PE_RATIO_MOVE_LEFT:
			newcmd = BG_IMG_MOVE_LEFT;
			break;
		case PE_RATIO_MOVE_RIGHT:
			newcmd = BG_IMG_MOVE_RIGHT;
			break;
		case PE_RATIO_MOVE_UP:
			newcmd = BG_IMG_MOVE_UP;
			break;
		case PE_RATIO_MOVE_DOWN:
			newcmd = BG_IMG_MOVE_DOWN;
			break;
		default:
			goto PZOOM_OUT;
			break;
	}
	printf("%s,%d:cmd=%d,index=%d\n",__func__,__LINE__,newcmd,index);
	img_dec_req_reset_deccmd(newcmd,index,NULL);
	
PZOOM_OUT:
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}

/**
* @brief rotate the photo via decoder.
*
*/
static int pe_rotate(void * handle)
{
	int angle;
	char *file;
	int index;
	int cmd;
	
	SWFEXT_FUNC_BEGIN(handle);

	if(Swfext_GetParamNum() != 2){
		goto ROTATE_OUT;
	}

	file = Swfext_GetString();
	angle = Swfext_GetNumber();
	
	/** FIXME: [do rotation via decode]*/
	index =  (int)img_dec_req_query(QUE_CMD_FILENAME,file);
	if(index == -1){
		goto ROTATE_OUT;
	}
	//cmd = IMG_ROT_RIGHT_90;
	cmd = __rotate_cmd_as2midware(angle);
	printf("%s,%d:cmd=%d,index=%d\n",__func__,__LINE__,cmd,index);
	img_dec_req_reset_deccmd(cmd,index,(void *)angle);
ROTATE_OUT:
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}

/**
* @brief enable/disable auto rotate
*/
static int pe_set_auto_rotate(void * handle)
{
	int cmd;
	
	SWFEXT_FUNC_BEGIN(handle);
	
#ifdef ROTATE_SENSOR	
	cmd = Swfext_GetNumber();
	sensor_en = !!cmd;
#endif

	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();
}

static void _pe_init_media_info_input(M_INFO_INPUT_S *input){
	input->p_read = fui_os_fread;
	input->p_write = fui_os_fwrite;
	input->p_seek_cur = fui_os_fseek_cur;
	input->p_seek_set = (void*)fui_os_fseek_set;
	input->p_seek_end = fui_os_fseek_end;
	input->p_tell = fui_os_ftell;
	input->p_free = fui_os_free;
	input->p_malloc = fui_os_malloc;
}

static int pe_get_exif_info(void *handle)
{
	M_INFO_INPUT_S pic_input;
	char *filepath=NULL;
	int cmd=0;
	MEDIA_INFO_CMD get_info_cmd=-1;
	MEDIA_INFO_RET ret=0;
	IMG_INFO_S img_Info;
	SWFEXT_FUNC_BEGIN(handle);
	_pe_init_media_info_input(&pic_input);		
	filepath = Swfext_GetString();
	cmd = Swfext_GetNumber();
	pic_input.file_handle = (void*)fopen(filepath,"rb");
	if(pic_input.file_handle !=NULL){
		memset(exif_tmp_buf,0,EXIF_TMPBUF_LEN);
		memset(&img_Info,0,sizeof(IMG_INFO_S));
		switch(cmd){
			case EXIF_TIME:
				img_Info.date_time_org_enable = 1;
				get_info_cmd = M_PHOTO;
				break;
			case EXIF_RESOLUTION:
				img_Info.img_hei_enable = 1;
				img_Info.img_wid_enable = 1;
				get_info_cmd = M_PHOTO;
				break;
			case EXIF_USERTAG:
				get_info_cmd = M_PHOTO_USR_TAG;
				break;
		}
		if(get_info_cmd==M_PHOTO){
			ret= GetMediaInfo(&pic_input,get_info_cmd, &img_Info);
			switch(cmd){
				case EXIF_TIME:
					
					printf("img_Info.iie_date_time_original.year===%d\n",__FILE__,__LINE__,img_Info.iie_date_time_original.year);
					printf("img_Info.iie_date_time_original.month===%d\n",__FILE__,__LINE__,img_Info.iie_date_time_original.month);
					printf("img_Info.iie_date_time_original.day===%d\n",__FILE__,__LINE__,img_Info.iie_date_time_original.day);
					sprintf(exif_tmp_buf,"%04d%02d%02d",img_Info.iie_date_time_original.year,\
										img_Info.iie_date_time_original.month,\
										img_Info.iie_date_time_original.day);
					break;
				case EXIF_RESOLUTION:
					sprintf(exif_tmp_buf,"%d*%d",img_Info.main_img_width,img_Info.main_img_height);
					break;	
			}
		}
		else
			printf("%s,%d:EXIF DO Not Support Now!\n",__FILE__,__LINE__);

		fclose(pic_input.file_handle);
	}
	else
		printf("%s,%d:Open File failed:%s\n",__FILE__,__LINE__,filepath);
	printf("exif_tmp_buf:%s\n",__FILE__,__LINE__,exif_tmp_buf);
	Swfext_PutString(exif_tmp_buf);
	SWFEXT_FUNC_END();
}


static INT32S SetPhotoAlbumName(void * handle)
{
    struct sysconf_param sys_cfg_data;
	
	INT8U albumIndex;
	char *albumname;
	SWFEXT_FUNC_BEGIN(handle);
	albumIndex = Swfext_GetNumber();
	apps_vram_read(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	albumname = Swfext_GetString();
	strcpy(sys_cfg_data.albumName[albumIndex], albumname);
	printf("%s %d (sys_cfg_data.albumName[%d])= %s\n",__FILE__,__LINE__,albumIndex,(sys_cfg_data.albumName[albumIndex]));
	apps_vram_write(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	apps_vram_store(VRAM_ID_SYSPARAMS);
	SWFEXT_FUNC_END();
}

static INT32S GetPhotoAlbumName(void * handle)
{
	struct sysconf_param sys_cfg_data;
	int albumIndex;
	SWFEXT_FUNC_BEGIN(handle);
	albumIndex = Swfext_GetNumber();
	apps_vram_read(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	printf("%s %d (sys_cfg_data.albumName[%d])= %s\n",__FILE__,__LINE__,albumIndex,(sys_cfg_data.albumName[albumIndex]));
	Swfext_PutString(sys_cfg_data.albumName[albumIndex]);
	
	SWFEXT_FUNC_END();
}

static INT32S GetPhotoAlbumSum(void * handle)
{
	struct sysconf_param sys_cfg_data;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	apps_vram_read(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	printf("%s %d (sys_cfg_data.albumSum)= %d\n",__FILE__,__LINE__,(sys_cfg_data.albumSum));
	Swfext_PutNumber(sys_cfg_data.albumSum);
	SWFEXT_FUNC_END();
}

static INT32S SetPhotoAlbumSum(void * handle)
{
	struct sysconf_param sys_cfg_data;

	SWFEXT_FUNC_BEGIN(handle);
	apps_vram_read(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	sys_cfg_data.albumSum= Swfext_GetNumber();
	apps_vram_write(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	apps_vram_store(VRAM_ID_SYSPARAMS);
	SWFEXT_FUNC_END();
}

static INT32S GetPhotoAlbumStatus(void * handle)
{
	struct sysconf_param sys_cfg_data;
	int albumIndex;
	SWFEXT_FUNC_BEGIN(handle);
	albumIndex = Swfext_GetNumber();
	apps_vram_read(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	printf("%s %d (sys_cfg_data.albumStatus[%d])= %d\n",__FILE__,__LINE__,albumIndex,(sys_cfg_data.albumStatus[albumIndex]));
	Swfext_PutNumber(sys_cfg_data.albumStatus[albumIndex]);
	
	SWFEXT_FUNC_END();
}

static INT32S SetPhotoAlbumStatus(void * handle)
{
	struct sysconf_param sys_cfg_data;
	INT8U albumIndex;
	INT8U albumstatus;

	SWFEXT_FUNC_BEGIN(handle);
	albumIndex = Swfext_GetNumber();
	apps_vram_read(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	albumstatus = Swfext_GetNumber();
	sys_cfg_data.albumStatus[albumIndex] = albumstatus;
	printf("%s %d (sys_cfg_data.albumStatus[%d])= %d\n",__FILE__,__LINE__,albumIndex,(sys_cfg_data.albumStatus[albumIndex]));
	apps_vram_write(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	apps_vram_store(VRAM_ID_SYSPARAMS);
	SWFEXT_FUNC_END();
}

static INT32S InitPhotoAlbum(void * handle)
{
	struct sysconf_param sys_cfg_data;
	int i = 0;
	SWFEXT_FUNC_BEGIN(handle);
	apps_vram_read(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	
	sys_cfg_data.albumSum=0;
	for(i = 0; i<16; i++)
	{
    	sys_cfg_data.albumStatus[i]=0;
        sys_cfg_data.albumName[i][0]='\0';
    }	
	apps_vram_write(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param));
	apps_vram_store(VRAM_ID_SYSPARAMS);
	SWFEXT_FUNC_END();
}

#if EZCAST_ENABLE
// @brief  Check if moble upload a picture, and move it to another file for show it.
static int pe_check_help_picture(void *handle){
	int pic_status = 0;
	char cmd[64];
	
	SWFEXT_FUNC_BEGIN(handle);
	//printf("\t%d\n", access(pic_path, 0));
	if(!access(PIC_PATH, 0)){
		EZCASTLOG("Have a new user manual jpeg\n");
		pic_status = 1;
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "mv %s %s", PIC_PATH, SHOW_PIC_PATH);
		system(cmd);
	}else{
		pic_status = 0;
	}
//printf("pic_status = %d\n", pic_status);
	Swfext_PutNumber(pic_status);
	SWFEXT_FUNC_END();
}

// @brief  Show help picture in EZCast function, it can only show one jpeg picture.
static int pe_show_help_picture(void *handle){
	int width, height;
	char cmd[64];
	
	SWFEXT_FUNC_BEGIN(handle);
	if(!access(SHOW_PIC_PATH, F_OK)){
		if(ezGetResolution(&width, &height) == 0){
			ezCustomerShowOnlineHelp(SHOW_PIC_PATH, 0, (height*180)/1080, width, height);
		}
		unlink(SHOW_PIC_PATH);
	}
	
	SWFEXT_FUNC_END();
}

static int pe_clean_help_picture(void *handle){
	
	SWFEXT_FUNC_BEGIN(handle);
	ezCustomerCleanOnlineHelp();
	//printf("pic_status = %d\n", pic_status);
	SWFEXT_FUNC_END();
}

static int coordinate_check(const int x, const int y, const int w, const int h, const int cmp_w, const int cmp_h){
	if(x < 0 || x > cmp_w)
		return -1;
	if(y < 0 || y > cmp_h)
		return -1;
	if(w <= 0 || w > cmp_w)
		return -1;
	if(h <= 0 || h > cmp_h)
		return -1;

	return 0;
}

int ezcastShowTextInDisplay(char *textstring, int fontsize, EZCASTARGB_t *color, int x, int y){
	int cur_w, cur_h;	// cur_w and cur_h is the width and height of the current resolution;

	if(textstring != NULL){
		if(coordinate_check(x, y, 1, 1, 1920, 1080) < 0){
			EZCASTWARN("The coordinate error!!![x: %d, y: %d]\n", x, y);
			return -1;
		}
		cur_w=screen_output_data.screen_output_width;
		cur_h=screen_output_data.screen_output_height;
		if(cur_h == 720){//will be fixed later for VGA resolution richardyu 080814
			//printf("[%s] [%d] -- original fontsize: %d\n", __func__, __LINE__, fontsize);
			fontsize += 1;
		}
		EZCASTLOG("show text in display, res_w = %d, res_h = %d, \\\n\t\textstring = %s, x=%d, y=%d, fontsize=%d, A=%d, R=%d, G=%d, B=%d\n", \
			cur_w, cur_h, textstring, x, y, fontsize, color->A, color->R, color->G, color->B);
		int ratio_w = (cur_w*1000)/1920;
		int ratio_h = (cur_h*1000)/1080;
		int ratio = ratio_w;
		int offset_x = 0, offset_y = 0;
		if(ratio_w<ratio_h){
			ratio = ratio_w;
			offset_x = 0;
			offset_y = (cur_h-((1080*ratio)/1000))/2;
		}else{
			ratio = ratio_h;
			offset_x = (cur_w-((1920*ratio)/1000))/2;
			offset_y = 0;
		}
		ezCastShowText(textstring, (fontsize*ratio)/1000, color, (ratio*x)/1000+offset_x, (ratio*y)/1000+offset_y);
		isIconShowing = 1;
		return 0;
	}

	EZCASTWARN("textstring error!!\n");
	return -1;
}

// @brief  Show text in EZCast function.
static int pe_show_text_in_display(void *handle){
	int x, y, fontsize, color;
	char *textstring;
	
	SWFEXT_FUNC_BEGIN(handle);
	textstring = Swfext_GetString();
	fontsize = Swfext_GetNumber();
	color = Swfext_GetNumber();
	x = Swfext_GetNumber();
	y = Swfext_GetNumber();
	#if(!EZMUSIC_ENABLE)
	ezcastShowTextInDisplay(textstring, fontsize, (EZCASTARGB_t *)&color, x, y);
	#endif
	SWFEXT_FUNC_END();
}

int ezcastShowIconInDisplay(char *path, int x, int y, int w, int h){
	int cur_w, cur_h;	// cur_w and cur_h is the width and height of the current resolution;

	if(path != NULL && !access(path, F_OK)){
		if(coordinate_check(x, y, w, h, 1920, 1080) < 0){
			EZCASTWARN("The coordinate error!!![x: %d, y: %d, w: %d, h: %d]\n", x, y, w, h);
			return -1;
		}
		cur_w=screen_output_data.screen_output_width;
		cur_h=screen_output_data.screen_output_height;
		if(cur_h == 720){
			//printf("[%s] [%d] -- original w: %d, h: %d\n", __func__, __LINE__, w, h);
			w += 3;
			h += 2;
		}
		EZCASTLOG("show icon in display, res_w = %d, res_h = %d, \\\n\t\tpath = %s, x=%d, y=%d, w=%d, h=%d\n", \
			cur_w, cur_h, path, x, y, w, h);
		int ratio_w = (cur_w*1000)/1920;
		int ratio_h = (cur_h*1000)/1080;
		int ratio = ratio_w;
		int offset_x = 0, offset_y = 0;
		if(ratio_w<ratio_h){
			ratio = ratio_w;
			offset_x = 0;
			offset_y = (cur_h-((1080*ratio)/1000))/2;
		}else{
			ratio = ratio_h;
			offset_x = (cur_w-((1920*ratio)/1000))/2;
			offset_y = 0;
		}
		EZCASTLOG("ratio: %d, offset_x: %d, offset_y: %d\n", ratio, offset_x, offset_y);
		int display_x = (ratio*x)/1000+offset_x;
		display_x = PROOFREED_COORDINATE(display_x, cur_w);
		int display_y = (ratio*y)/1000+offset_y;
		display_y = PROOFREED_COORDINATE(display_y, cur_h);
		int display_w = (ratio*w)/1000;
		display_w = PROOFREED_COORDINATE(display_w, cur_w);
		int display_h = (ratio*h)/1000;
		display_h = PROOFREED_COORDINATE(display_h, cur_h);
		EZCASTLOG("display_x: %d, display_y: %d, display_w: %d, display_h: %d\n", display_x, display_y, display_w, display_h);
		ezCustomerShowOverlayImg(path, display_x, display_y, display_w, display_h);
		isIconShowing = 1;
		return 0;
	}

	EZCASTWARN("path error!!\n");
	return -1;
}
	
void ezcastCleanIconRect(int x, int y, int w, int h){
	int cur_w, cur_h;	// cur_w and cur_h is the width and height of the current resolution;

	if(coordinate_check(x, y, w, h, 1920, 1080) < 0){
		EZCASTWARN("The coordinate error!!![x: %d, y: %d, w: %d, h: %d]\n", x, y, w, h);
		return;
	}
	cur_w=screen_output_data.screen_output_width;
	cur_h=screen_output_data.screen_output_height;
	if(cur_h == 720){
		//printf("[%s] [%d] -- original w: %d, h: %d\n", __func__, __LINE__, w, h);
		w += 4;
		h += 2;
	}
	EZCASTLOG("clean icon in display, res_w = %d, res_h = %d, \\\n\t\tx=%d, y=%d, w=%d, h=%d\n", \
		cur_w, cur_h, x, y, w, h);
	
	int ratio_w = (cur_w*1000)/1920;
	int ratio_h = (cur_h*1000)/1080;
	int ratio = ratio_w;
	int offset_x = 0, offset_y = 0;
	if(ratio_w<ratio_h){
		ratio = ratio_w;
		offset_x = 0;
		offset_y = (cur_h-((1080*ratio)/1000))/2;
	}else{
		ratio = ratio_h;
		offset_x = (cur_w-((1920*ratio)/1000))/2;
		offset_y = 0;
	}
	int display_x = (ratio*x)/1000+offset_x;
	display_x = PROOFREED_COORDINATE(display_x, cur_w);
	int display_y = (ratio*y)/1000+offset_y;
	display_y = PROOFREED_COORDINATE(display_y, cur_h);
	int display_w = (ratio*w)/1000;
	display_w = PROOFREED_COORDINATE(display_w, cur_w);
	int display_h = (ratio*h)/1000;
	display_h = PROOFREED_COORDINATE(display_h, cur_h);
	ezCustomerCleanOverlayRect(display_x, display_y, display_w, display_h);
}

void ezcastCleanIcon(){
	if(isIconShowing == 1){
		isIconShowing = 0;
		ezCustomerCleanOverlayImg();
		EZCASTLOG("Clean all icon and test!!!\n");
	}
}

// @brief  Show picture in EZCast function, it can show more than one jpeg/png pictures .
static int pe_show_icon_in_display(void *handle){
	int x, y, w, h;
	char *path;
	
	SWFEXT_FUNC_BEGIN(handle);
	path = Swfext_GetString();
	x = Swfext_GetNumber();
	y = Swfext_GetNumber();
	w = Swfext_GetNumber();
	h = Swfext_GetNumber();
	#if(!EZMUSIC_ENABLE)
	ezcastShowIconInDisplay(path, x, y, w, h);
	#endif
	SWFEXT_FUNC_END();
}

static int pe_clean_icon(void *handle){
	
	SWFEXT_FUNC_BEGIN(handle);
	ezcastCleanIcon();
	SWFEXT_FUNC_END();
}

static int pe_icon_clean_rect(void *handle){
	int x, y, w, h;

	SWFEXT_FUNC_BEGIN(handle);
	x = Swfext_GetNumber();
	y = Swfext_GetNumber();
	w = Swfext_GetNumber();
	h = Swfext_GetNumber();
	#if(!EZMUSIC_ENABLE)
	ezcastCleanIconRect(x, y, w, h);
	#endif
	SWFEXT_FUNC_END();
}
#endif
#if 1
typedef struct _bmp_header_
{
	/* 00h */	unsigned short BM;			
	/* 02h */	unsigned long file_size;	
	/* 06h */	unsigned short reserved0;	
	/* 08h */	unsigned short reserved1;	
	/* 0Ah */	unsigned long data_offset;	
	/* 0Eh */	unsigned long header_size;	
	/* 12h */	unsigned long width;		
	/* 16h */	long height;				
	/* 1Ah */	unsigned short planes;
	/* 1Ch */	unsigned short num_of_bits;
	/* 1Eh */	unsigned long compression;
	/* 22h */	unsigned long bmp_data_size;
	/* 26h */	unsigned long hor_resolution;
	/* 2Ah */	unsigned long ver_resolution;
	/* 2Eh */	unsigned long palette_colors;
	/* 32h */	unsigned long important_colors;
}__attribute__((packed)) BMP_HEADER;
 #define PP_PIX_FMT_RGB16_5_5_5							0x040001U
#define PP_PIX_FMT_RGB16_5_6_5							0x040002U
#define PP_PIX_FMT_BGR16_5_5_5							0x040003U
#define PP_PIX_FMT_BGR16_5_6_5							0x040004U
#define PP_PIX_FMT_RGB32								0x041001U
#define PP_PIX_FMT_BGR32								0x041002U
static int output_bmp_RGB16_RGB32(char *fname, unsigned char *img, int width, int height, int step, int pix_fmt)
{
	void *fptr = NULL;
	BMP_HEADER hdr;
	unsigned long red_mask, green_mask, blue_mask;
	int byte_per_pixel;
	int line_size;
	char padding_data[4] = {0, 0, 0, 0};
	int padding_size = 0;
	int file_size, header_size, bmp_data_size;
	unsigned long time_sec;
	int i;

	switch(pix_fmt)
	{
	case PP_PIX_FMT_RGB32:
		byte_per_pixel = 4;
		red_mask	= 0x00ff0000;
		green_mask	= 0x0000ff00;
		blue_mask	= 0x000000ff;
		break;
	case PP_PIX_FMT_BGR32:
		byte_per_pixel = 4;
		red_mask	= 0x000000ff;
		green_mask	= 0x0000ff00;
		blue_mask	= 0x00ff0000;
		break;
	case PP_PIX_FMT_RGB16_5_6_5:
		red_mask	= 0x0000f800;
		green_mask	= 0x000007e0;
		blue_mask	= 0x0000001f;
		byte_per_pixel = 2;
		break;
	case PP_PIX_FMT_RGB16_5_5_5:
		red_mask	= 0x00007c00;
		green_mask	= 0x000003e0;
		blue_mask	= 0x0000001f;
		byte_per_pixel = 2;
		break;
	case PP_PIX_FMT_BGR16_5_6_5:
		red_mask	= 0x0000001f;
		green_mask	= 0x000007e0;
		blue_mask	= 0x0000f800;
		byte_per_pixel = 2;
		break;
	case PP_PIX_FMT_BGR16_5_5_5:
		red_mask	= 0x0000001f;
		green_mask	= 0x000003e0;
		blue_mask	= 0x00007c00;
		byte_per_pixel = 2;
		break;
	default:
		OSprintf("unsupported output pixel format!\n");
		return -1;
	}
	line_size = width*byte_per_pixel;
	if(line_size%4)
		padding_size = 4-(line_size%4);
	bmp_data_size = (line_size+padding_size)*height;
	header_size = sizeof(BMP_HEADER)+12;
	file_size = ((header_size+3)/4)*4+bmp_data_size;
	if(header_size%4)
		file_size += 4-(header_size%4);
	// create BMP file header //////////////////////////////////////////////////////////////////////////
	hdr.BM					= 0x4d42;
	hdr.file_size			= file_size;
	hdr.reserved0			= 0;
	hdr.reserved1			= 1;
	hdr.data_offset			= header_size;
	hdr.header_size			= 40;
	hdr.width				= width;
	hdr.height				= -height;
	hdr.planes				= 1;
	hdr.num_of_bits			= byte_per_pixel*8;
	hdr.compression			= 3;
	hdr.bmp_data_size		= bmp_data_size;
	hdr.hor_resolution		= 0;
	hdr.ver_resolution		= 0;
	hdr.palette_colors		= 0;
	hdr.important_colors	= 0;
	//////////////////////////////////////////////////////////////////////////
	printf("BM =0x%x , filesize=0x%x,,size=%d,%d\n",hdr.BM,hdr.file_size,sizeof(unsigned short int),sizeof(unsigned short));
	printf("BMP head=%d   addr1=0x%x,addr2=0x%x]]]",sizeof(BMP_HEADER),&hdr.BM,&hdr.file_size);

	{
		int len_n = sizeof(BMP_HEADER);
		char *bmp_head=(char*)&hdr;
		int j=0;
		for(j=0;j<len_n;j++){
			printf("0x%02x ",*(bmp_head+j));
		}
	}
	
	//create new BMP file
	//OSGetTime(&time_sec, NULL);
	//OSsprintf(file_name, "%s_%08X.bmp", fname, time_sec);
	printf("filename=%s\n",fname);
	fptr = fopen(fname, "wb");
	if(fptr)
	{
		unsigned char *p = img;

		//write BMP file header
		fwrite(&hdr, 1, sizeof(BMP_HEADER), fptr);
		fwrite(&red_mask, 1, 4, fptr);
		fwrite(&green_mask, 1, 4, fptr);
		fwrite(&blue_mask, 1, 4, fptr);

		//write BMP pixel data
		for(i=0;i<height;i++)
		{
			fwrite(p, 1, line_size, fptr);
			p += step;
			if(padding_size)
				fwrite(padding_data, 1, padding_size, fptr);
		}

		if(header_size%4)
			fwrite(padding_data, 1, 4-(header_size%4), fptr);

		//close file
		fclose(fptr);
	}
	else
		printf("Open File bmp Failed!\n");
	return 0;
}




static int _pe_set_default_fops(io_layer_t  *io_layer)
{
	io_layer->img_fread = fui_os_fread;
	io_layer->img_fseek_set = fui_os_fseek_set;
	io_layer->img_fseek_cur = fui_os_fseek_cur;
	io_layer->img_fseek_end = fui_os_fseek_end;
	io_layer->img_ftell = fui_os_ftell;
	io_layer->img_malloc = (void*)SWF_Malloc;
	io_layer->img_realloc =NULL;
	io_layer->img_free = SWF_Free;
	io_layer->lp_get_bus_addr = fui_get_bus_address;
	return 0;
}

static char* _pe_gif_get_parser_buf(int format,int w,int h)
{
	char * buffer=NULL;
	if(format==SWF_BMP_FMT_YUV422){
		buffer = (char*)SWF_Malloc(w*h*2);
		if(buffer==NULL){
			pe_info("Malloc Pic Parser Buf Failed!");
		}
	}
	else if(format==SWF_BMP_FMT_ARGB){
		buffer = (char*)SWF_Malloc(w*h*4);
		if(buffer==NULL){
			pe_info("Malloc Pic Parser Buf Failed!");
		}
	}
	else
		pe_info("Sorry, The Format is not support!");
	return buffer;
}

static void _pe_gif_free_parser_buf(char*buf)
{
	if(buf!=NULL){
		SWF_Free(buf);
		buf = NULL;
	}
}

static int _pe_gif_parser_status(int id)
{
	return 1;
}


static loadimg_gifres_t *_pe_load_gif_start(char* filename,void* target, int w, int h)
{
	img_info_t img_info;
	io_layer_t io_layer;
	loadimg_gifres_t * gifres=NULL;
	int ret=0;

	io_layer.handle = fui_os_fopen(filename,"rb");
	_pe_set_default_fops(&io_layer);

	if(io_layer.handle==NULL){
		pe_err("Open fileError: %s",filename);
		return gifres;
	}
	else
		pe_info("Handle=0x%x",io_layer.handle);

	img_info.file_format = FILE_FORMAT_DYNAMIC_GIF;
	img_info.format = LP_FMT_RGB_32;
	img_info.out_pic_width = w;
	img_info.out_pic_height = h;
	img_info.out_pic_pos_x = 0;
	img_info.out_pic_pos_y =0 ;
	img_info.buf_width = w;
	img_info.buf_height = h;
	img_info.buf_info.buf_y.buf=(unsigned char*) _pe_gif_get_parser_buf((int)SWF_BMP_FMT_ARGB,w,h);
	if(img_info.buf_info.buf_y.buf==NULL)
		goto LOAD_GIF_START_END;
	
	img_info.buf_info.buf_y.size = w*h*4;
	img_info.buf_info.buf_y.bus_addr = fui_get_bus_address((unsigned long)img_info.buf_info.buf_y.buf);

	img_info.buf_info.buf_uv.buf= NULL;
	img_info.buf_info.buf_v.buf= NULL;
	

	loadimg_dec_open();

	gifres = loadimg_get_gifres(&io_layer);

	if(gifres==NULL)//get buffer error
		return gifres;
	
	ret = loadimg_gif_load_start(&io_layer,&img_info,gifres);
	if(ret==-1){
		//should release the resource
		pe_info("Release Space!");
		
		_pe_gif_free_parser_buf((char*)img_info.buf_info.buf_y.buf);
		fui_os_fclose(io_layer.handle);
		loadimg_free_gifres(&io_layer,gifres);
		
		gifres=NULL;
	}
	else{
		gifres->output_buffer = img_info.buf_info.buf_y.buf;
		gifres->target = target;
		gifres->target_w = w;
		gifres->target_h = h;
		gifres->file_handle = io_layer.handle;
		
		SWF_AttachBitmap(target,(unsigned int*)img_info.buf_info.buf_y.buf,w,h,w,h,w,SWF_BMP_FMT_ARGB,_pe_gif_parser_status);
	}
LOAD_GIF_START_END:
	return gifres;

}

static int _pe_gif_get_next_frame(loadimg_gifres_t * gifres)
{
	int ret=0;
	//static char whichframe=1;
	io_layer_t io_layer;
	img_info_t img_info;
	int w=0,h=0;
	char filename[32]="";
	
	_pe_set_default_fops(&io_layer);

	w=gifres->target_w;
	h=gifres->target_h;
	
	img_info.file_format = FILE_FORMAT_DYNAMIC_GIF;
	img_info.format = LP_FMT_RGB_32;
	img_info.out_pic_width = w;
	img_info.out_pic_height = h;
	img_info.out_pic_pos_x = 0;
	img_info.out_pic_pos_y =0 ;
	img_info.buf_width = w;
	img_info.buf_height = h;
	img_info.buf_info.buf_y.buf=gifres->output_buffer;

	
	img_info.buf_info.buf_y.size = w*h*4;
	img_info.buf_info.buf_y.bus_addr = fui_get_bus_address((unsigned long)img_info.buf_info.buf_y.buf);

	img_info.buf_info.buf_uv.buf= NULL;
	img_info.buf_info.buf_v.buf= NULL;

	ret = loadimg_gif_get_next_frame(&io_layer,&img_info,gifres);

	SWF_DetachBitmap(gifres->target,0);

	w=gifres->target_w;
	h=gifres->target_h;

	//sprintf(filename,"%s%d%s","/mnt/udisk/",whichframe,".bmp");
	//pe_info("filename=%s",filename);
	//whichframe++;
	//output_bmp_RGB16_RGB32(filename,gifres->output_buffer,w,h,w*4,PP_PIX_FMT_RGB32);

	SWF_AttachBitmap(gifres->target,(unsigned int*)gifres->output_buffer,w,h,w,h,w,SWF_BMP_FMT_ARGB,_pe_gif_parser_status);

	return ret;
}

static int _pe_load_gif_end(loadimg_gifres_t * gifres)
{
	io_layer_t io_layer;
	int ret=0;
	_pe_set_default_fops(&io_layer);
	
	ret = loadimg_gif_load_end(&io_layer,NULL,gifres);
	if(gifres!=NULL){
		
		SWF_DetachBitmap(gifres->target,0);
		
		if(gifres->file_handle!=NULL){
			pe_info("release handle!file_handle=0x%x",gifres->file_handle);
			fui_os_fclose(gifres->file_handle);
			gifres->file_handle =NULL;
		}
		if(gifres->output_buffer!=NULL){
			pe_info("release buffer!\n");
			_pe_gif_free_parser_buf(gifres->output_buffer);
			gifres->output_buffer = NULL;
		}
		loadimg_free_gifres(&io_layer,gifres);
		gifres=NULL;
	}
	
	loadimg_dec_close();
	pe_info("GIF END!\n");
	return ret;
}

static int pe_gif_start_load(void * handle)
{
	loadimg_gifres_t * gifres=NULL;
	char *giffilepath=NULL;
	void *target=NULL;
	int w=0,h=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	giffilepath = Swfext_GetString();
	target   = Swfext_GetObject();
	w = Swfext_GetNumber();
	h = Swfext_GetNumber();
		
	pe_info("path=%s,w=%d,h=%d\n",giffilepath,w,h);
	gifres = _pe_load_gif_start(giffilepath,target,w,h);


	pe_info("gifres=0x%x",gifres);
	Swfext_PutNumber((int)gifres);
	SWFEXT_FUNC_END();
}

static int pe_gif_get_next_frame(void *handle)
{
	loadimg_gifres_t * gifres=NULL;
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);

	gifres = (loadimg_gifres_t*)Swfext_GetNumber();
	pe_info("gifres=0x%x",gifres);
	
	ret = _pe_gif_get_next_frame(gifres);
		
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

static int pe_gif_end_load(void *handle)
{
	loadimg_gifres_t * gifres=NULL;

	int ret=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	gifres = (loadimg_gifres_t*)Swfext_GetNumber();
	pe_info("gifres=0x%x",gifres);

	ret = _pe_load_gif_end(gifres);
	
	Swfext_PutNumber(ret);
	
	SWFEXT_FUNC_END();
}

static int pe_gif_get_status(void* handle)
{
	loadimg_gifres_t * gifres=NULL;
	int cmd=0;
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);

	gifres = (loadimg_gifres_t*)Swfext_GetNumber();
	cmd = Swfext_GetNumber();
	
	pe_info("gifres=0x%x",gifres);

	if(gifres!=NULL){
		switch(cmd){
			case GET_FRAME_DELAYTIME:
				ret = (int)gifres->curframe_delay;
				break;
			case GET_FRAME_DEC_RET:
				ret = (int)gifres->curframe_decret;
				break;
			default:
				printf("%s,%d:Sorry Not support yet\n",__FILE__,__LINE__);
		}
	}
	pe_info("cmd =%d, ret=%d",cmd,ret);
	Swfext_PutNumber(ret);
	
	SWFEXT_FUNC_END();
}

#endif

void pe_init_global_para()
{
	memset(&pic_decode_result,0,sizeof(pic_decode_result_t));
}

int swfext_pe_register(void)
{
	pe_init_global_para();
	SWFEXT_REGISTER("pe_Open", pe_open);
	SWFEXT_REGISTER("pe_Close", pe_close);
	SWFEXT_REGISTER("pe_Show", pe_show);
	SWFEXT_REGISTER("pe_GetStatus", pe_get_status);
	SWFEXT_REGISTER("pe_Attach", pe_attach);
	SWFEXT_REGISTER("pe_Detach", pe_detach);
	SWFEXT_REGISTER("pe_StopDecode", pe_stop_decode);
	SWFEXT_REGISTER("pe_GetDecodedErr",pe_get_decoded_err);
	SWFEXT_REGISTER("pe_GetPlayMode", pe_get_play_mode);
	SWFEXT_REGISTER("pe_SetPlayMode", pe_set_play_mode);
	SWFEXT_REGISTER("pe_GetMusicEnable", pe_get_music_enable);
	SWFEXT_REGISTER("pe_SetMusicEnable", pe_set_music_enable);
	SWFEXT_REGISTER("pe_GetPlayRatio", pe_get_play_ratio);
	SWFEXT_REGISTER("pe_SetPlayRatio", pe_set_play_ratio);
	SWFEXT_REGISTER("pe_GetSlideShowTime", pe_get_slideshow_time);
	SWFEXT_REGISTER("pe_SetSlideShowTime", pe_set_slideshow_time);
	SWFEXT_REGISTER("pe_GetSlideShowEffect", pe_get_slideshow_effect);
	SWFEXT_REGISTER("pe_SetSlideShowEffect", pe_set_slideshow_effect);
/*add by richard 04142012*/	
	SWFEXT_REGISTER("pe_GetSlideShowMode", pe_get_slideshow_mode);
	SWFEXT_REGISTER("pe_SetSlideShowMode", pe_set_slideshow_mode);
/*add by richard 04142012*/
	

	SWFEXT_REGISTER("pe_SetBackGroundEffect", pe_set_background_effect);
	SWFEXT_REGISTER("pe_GetBackGroundEffect", pe_get_background_effect);

	SWFEXT_REGISTER("pe_SetAutoRotationEnByExif", pe_set_autorotationExif_en);
	SWFEXT_REGISTER("pe_GetAutoRotationEnByExif", pe_get_autorotationExif_en);	

	SWFEXT_REGISTER("pe_SetAutoRotationEnByAdhere", pe_set_autorotationAdhere_en);
	SWFEXT_REGISTER("pe_GetAutoRotationEnByAdhere", pe_get_autorotationAdhere_en);	
	
	SWFEXT_REGISTER("pe_GetClockEnable", pe_get_clock_enable);
	SWFEXT_REGISTER("pe_SetClockEnable", pe_set_clock_enable);
	SWFEXT_REGISTER("pe_AttachFace",pe_attach_face);
	SWFEXT_REGISTER("pe_DetachFace",pe_detach_face);
	SWFEXT_REGISTER("pe_DetectFace",pe_detect_face);
	SWFEXT_REGISTER("pe_GetFaceRect",pe_get_face_rect);
	SWFEXT_REGISTER("pe_setFaceFuncSwitch",pe_set_face_func_switch);
	SWFEXT_REGISTER("pe_getFaceFuncSwitch",pe_get_face_func_switch);
	SWFEXT_REGISTER("pe_PrepareForDel", pe_prepare_for_delete);
	SWFEXT_REGISTER("pe_ResizeCopy", pe_do_resize_copy);
	SWFEXT_REGISTER("pe_StoreRotation", pe_store_rotation);
	SWFEXT_REGISTER("pe_GetRotation", pe_get_rotation);
	SWFEXT_REGISTER("pe_StoreEffect", pe_store_effect);
	SWFEXT_REGISTER("pe_PSEffect", pe_store_process_effect);
	SWFEXT_REGISTER("pe_Rotate", pe_rotate);
	SWFEXT_REGISTER("pe_SetAutoRotate", pe_set_auto_rotate);
	SWFEXT_REGISTER("pe_Zoom", pe_zoom);
	SWFEXT_REGISTER("pe_CheckResizeCopy", pe_check_resize_copy);

	SWFEXT_REGISTER("pe_getExitInfo", pe_get_exif_info);
	SWFEXT_REGISTER("pe_getDecodedPicInfo",pe_get_decoded_pic_info);

	SWFEXT_REGISTER("pe_SetPhotoAlbumName", SetPhotoAlbumName);
	SWFEXT_REGISTER("pe_GetPhotoAlbumName", GetPhotoAlbumName);
	SWFEXT_REGISTER("pe_GetPhotoAlbumSum", GetPhotoAlbumSum);
	SWFEXT_REGISTER("pe_SetPhotoAlbumSum", SetPhotoAlbumSum);
	SWFEXT_REGISTER("pe_GetPhotoAlbumStatus", GetPhotoAlbumStatus);
	SWFEXT_REGISTER("pe_SetPhotoAlbumStatus", SetPhotoAlbumStatus);
	SWFEXT_REGISTER("pe_InitPhotoAlbum", InitPhotoAlbum);


	SWFEXT_REGISTER("pe_gifLoadStart", pe_gif_start_load);
	SWFEXT_REGISTER("pe_gifGetNextFrame", pe_gif_get_next_frame);
	SWFEXT_REGISTER("pe_gifLoadEnd", pe_gif_end_load);
	SWFEXT_REGISTER("pe_gifGetStatus", pe_gif_get_status);
	//add for dlna
	SWFEXT_REGISTER("pe_DecodeImage",pe_decode_image);
#if EZCAST_ENABLE
	SWFEXT_REGISTER("pe_check_help_picture",pe_check_help_picture);
	SWFEXT_REGISTER("pe_show_help_picture",pe_show_help_picture);
	SWFEXT_REGISTER("pe_clean_help_picture",pe_clean_help_picture);
	SWFEXT_REGISTER("pe_show_icon_in_display",pe_show_icon_in_display);
	SWFEXT_REGISTER("pe_show_text_in_display",pe_show_text_in_display);
	SWFEXT_REGISTER("pe_clean_icon",pe_clean_icon);
	SWFEXT_REGISTER("pe_icon_clean_rect",pe_icon_clean_rect);
#endif
	SWFEXT_REGISTER("pe_Decodebmp",pe_decode_bmp);
	SWFEXT_REGISTER("pe_Bmpexit",pe_exit_bmp);//pe_decode_image);//
	SWFEXT_REGISTER("pe_Bmpstart",pe_start_bmp);
	
	return 0;
}


