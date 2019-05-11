/*
********************************************************************************
*                       linux213x                
*         actions plugin and engine interface structure
*                (c) Copyright 2002-2003, Actions Co,Ld.                       
*                        All Right Reserved                               
*
* File   : act_plugin.h 
* by	 : IP/FW/AL	
* Version: 1> v1.00     first version     02.24.2006
********************************************************************************
*/
#ifndef ACT_PLUGIN_H
# define	ACT_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

/*定义了不同的插件类型常量，这些常量用于插件信息的type字段*/
#define		PLUGIN_DEMUX		0x01			//plugin for demuxer
#define		PLUGIN_AUDIO_DECODER	0x02			//plugin for audio decoder
#define		PLUGIN_VIDEO_DECODER	0x03			//plugin for video decoder
#define 	PLUGIN_IMAGE_DECODER	0x04			//plugin for img decoder
#define 	PLUGIN_MUX		0x05			//plugin for muxer
#define		PLUGIN_AUDIO_ENCODER	0x06			//plugin for audio encoder
#define		PLUGIN_VIDEO_ENCODER	0x07			//plugin for video encoder
#define 	PLUGIN_IMAGE_ENCODER	0x08			//plugin for img encoder
#define		PLUGIN_IPP		0x09

/*constant for skip*/
#define 	SKIP_FORWARD			0x01
#define		SKIP_BACKWARD			0x02

/*constant for  ab play*/
#define		SET_AP					0x01
#define 	RETURN_TO_A				0x02

/*structure for buffer element*/
typedef struct buf_ele_s{
	struct buf_ele_s  *next; 
	unsigned int type;
	unsigned int len;								/*length of buffer*/
	unsigned int size;								/*length of content*/
	unsigned char *buf;                     		/*user address of the buffer*/
	unsigned char *phy_buf;							/*physical address of this buffer*/	
	void    *source;
	void 	*mem;
}buf_element_t;

typedef struct{
		char *extension;
		unsigned int file_len;
}file_info_t;								/*get file information*/

/*disk manager seek*/
#define		DSEEK_SET		0x01
#define		DSEEK_END		0x02
#define		DSEEK_CUR		0x03

typedef struct stream_input_s{
		int (*read)(struct stream_input_s *input,unsigned int buf,unsigned int len);
		int (*write)(struct stream_input_s *input,unsigned int buf,unsigned int len);
		int (*seek)(struct stream_input_s *input,int offset,int original);		
		int (*tell)(struct stream_input_s *input);
		int (*get_file_info)(struct stream_input_s *input,file_info_t *info);
		int (*dispose)(struct stream_input_s *input);
		int (*testfile)(struct stream_input_s *input);
}stream_input_t;

typedef struct dec_port_s{
		buf_element_t *(*get_buf)(struct dec_port_s *port,unsigned int len);
		int (*put_buf)(struct dec_port_s *port,buf_element_t *buf);
		int (*block)(struct dec_port_s *port);
		int (*reset)(struct dec_port_s *port);			
		int (*dispose)(struct dec_port_s *port);
}dec_port_t;

/*下面的数据结构是有关不同插件输入输出的数据结构，在执行插件的open函数时传入*/
/*不同的插件输入输出有不同的格式，插件的编写者根据插件的类型选择相应的结构*/
typedef struct{
		stream_input_t *input;
		dec_port_t *ao;
		dec_port_t *vo;
}demux_plugio_t;

typedef struct{
		stream_input_t *ain;
		dec_port_t *ao;
}audiodec_plugio_t;

typedef struct{
		stream_input_t *vin;
		dec_port_t *vo;
		void *clock;
}videodec_plugio_t;

typedef struct{
		stream_input_t *input;
}imagedec_plugio_t;

typedef struct{
		stream_input_t *ain;
		stream_input_t *vin;
		stream_input_t *output;	
}mux_plugio_t;
 
typedef struct{
		stream_input_t *ain;
		dec_port_t *aout;
}audioenc_pluginio_t;

typedef struct{
		stream_input_t *vin;
		dec_port_t *vout;
}videoenc_pluginio_t;

typedef struct{
		stream_input_t *output;
}imageenc_plugio_t;


#define		FILE_NOT_SPPORT		0x01
#define		FILE_IS_DAMAGED		0x02
	
/*该结构记录了不同插件的信息，中间件在完成插件加载时使用*/
typedef struct{
		char type;
		char extension[10];
		void *(*open)(void *plugio);		/*open插件时需要告诉当前插件的输入输出*/
		int (*get_file_info)(stream_input_t *input,void *file_info,void *mode);
}plugin_info_t;

/*constant for media type*/
#define		IS_AUDIO				0x01			//contains only audio
#define		IS_VIDEO				0x02			//contains only video	
#define		IS_AV					0x03			//contains audio and video

/*demuxer inface*/
typedef struct{		
		int media_type;			/*01:audio;02:video;03:av*/
		int total_time;			/*total time it can play*/
		int bitrate;				
		int sample_rate;  		
		int channel;      
		int sample_bits;
		int audio_bitrate;				
		int width;
		int height;
		int frame_rate;
		int video_bitrate;
		unsigned int first_audio_time;		
		char audio[10];			
		char video[10];			
		int index_flag;		
		int ilength_audio;		
		int ilength_video;		
		int vc1_extradata_size;
		char vc1_extradata[16];
}movi_info_t;


#define		END_OF_AUDIO		-2
#define		END_OF_VIDEO		-3
typedef struct demux_plugin_s{
		int (*init)(struct demux_plugin_s *plugin,void *param);
		int (*parse_header)(struct demux_plugin_s *plugin);
		int (*parse_stream)(struct demux_plugin_s *plugin);
		int (*seek)(struct demux_plugin_s *plugin,int start_time);
		int (*skip)(struct demux_plugin_s *plugin,int direction);		
		int (*get_time)(struct demux_plugin_s *plugin);
		int (*get_err)(struct demux_plugin_s *plugin);		
		int (*AB_play)(struct demux_plugin_s *plugin,int AB_flag);		
		int (*dispose)(struct demux_plugin_s *plugin);						
}demux_plugin_t;

/*commands definition for audio_decoder_t function sync*/
#define AD_PLUGIN_PLAY     0
#define AD_PLUGIN_STOP     1
#define AD_PLUGIN_RESET    2
#define AD_PLUGIN_PAUSE    3


typedef struct audio_decoder_s{
		int (*init)(struct audio_decoder_s *plugin,void *param);
		int (*decode_data)(struct audio_decoder_s *plugin);
		int (*get_err)(struct audio_decoder_s *plugin);
		unsigned int (*get_time)(struct audio_decoder_s *plugin);
		int (*sync)(struct audio_decoder_s *plugin, unsigned int cmd);        
		int (*dispose)(struct audio_decoder_s *plugin);           
}audio_decoder_t;

typedef struct video_decoder_s{
		int (*init)(struct video_decoder_s *plugin,void *param);
		int (*decode_data)(struct video_decoder_s *plugin,int *changeflag,void *video_param,int FF_FB_flag);
		int (*preview_frame)(struct video_decoder_s *plugin,void *video_param,void *buf);
		int (*get_err)(struct video_decoder_s *plugin);
		int (*dispose)(struct video_decoder_s *plugin);
}video_decoder_t;

typedef struct{
		unsigned int time_stamp;	
		unsigned int width;
		unsigned int height;
		unsigned int formate;			
}vo_frame_t;							/*必须16字节对齐长度*/

typedef struct{
		unsigned int time_stamp;
		unsigned int sample_rate;
		unsigned int channels;
		unsigned int sample_bits;		
}ao_frame_t;							/*audio out frame header*/

/*interface related to img decoder*/
#define			FMT_RGB				0x00
#define			FMT_YUV				0x01

typedef struct{
		unsigned int year;
		unsigned int month;
		unsigned int day;
}image_date_t;

typedef struct{
		unsigned char bpp;
		unsigned char formate;
		unsigned int width;
		unsigned int height;
		image_date_t date;
		unsigned char exif[2*1024];
		unsigned int exif_pos;       //JPG缩略图在exif信息中的位置
}image_file_info_t;

typedef struct{		
		unsigned int formate;
		unsigned int bpp;		
		unsigned int width;
		unsigned int height;		
		unsigned char *buf;
		unsigned int len;
		unsigned int time_stamp;
		unsigned int exif_pos;       //JPG缩略图在exif信息中的位置
		int pic_dec_mode;
		// added by simon
		signed long brightness;
		signed long saturation;
		signed long contrast;
}image_info_t;

typedef struct{
		unsigned int photo_flag;	
		unsigned char *buf;
}camera_init_t;

typedef struct effect{
	signed long contrast;				//Adjust the contrast of the output picture
	signed long brightness;				//Adjust the brightness level of the output picture
	signed long saturation;				//Adjust the color saturation of the output picture
}effect;

typedef struct sf_effect{
	int contrast;				//Adjust the contrast of the output picture [-100 ~ 100]
	int brightness;				//Adjust the brightness level of the output picture [0 ~ 100]
	int saturation;				//Adjust the color saturation of the output picture [0 ~ 100]
	int hue;						//Adjust the color of the output [0 ~ 100]
	int videoinout;                         //0 for HDTV output, 1 for SDTV output     [0,1]
}sf_effect;

typedef struct{
	int *status;
	int fmt;
	int num;
	int den;
	int preview;

	int pip_enable;//换行输出
	int pos_x;
	int pos_y;

	int thumb_rot;    //缩略图旋转角度控制，赋值为以下的参数，定义与act_decoder.h
										//#define PP_ROT_NONE				0U
										//#define PP_ROT_RIGHT_90			1U
										//#define PP_ROT_LEFT_90			2U
										//#define PP_ROT_HOR_FLIP			3U
										//#define PP_ROT_VER_FLIP			4U
										//#define PP_ROT_180				5U
	int zoom_rate;	//除以100 是放大倍数
}decode_img_para;

typedef struct image_plugin_s{
		int (*init)(struct image_plugin_s *plugin);
		int (*decode_img)(struct image_plugin_s *plugin,image_info_t *img, unsigned long cmd,decode_img_para * para);
		//int (*decode_img)(struct image_plugin_s *plugin,image_info_t *img,unsigned long cmd);
		int (*dispose)(struct image_plugin_s *plugin);
}image_plugin_t;

typedef struct camera_plugin_s{
		int (*init)(struct camera_plugin_s *plugin,camera_init_t *param);
		int (*encode_img)(struct camera_plugin_s *plugin,image_info_t *img);
		int (*dispose)(struct camera_plugin_s *plugin);
}camera_plugin_t;

/*img post processing routine(ipp)*/
#define			MOVE_UP				0x01
#define			MOVE_DOWN			0x02
#define			MOVE_LEFT			0x03
#define			MOVE_RIGHT			0x04

#define			ROTATE_LEFT90		0x01
#define			ROTATE_RIGHT90		0x02
#define			ROTATE_180			0x03	

#define			ZOOM_IN				0x01
#define			ZOOM_OUT			0x02

#define			MIRROR_HORIZONTAL	0x01
#define			MIRROR_VERTICAL		0x02

typedef struct ipp_proc_s{
		int (*init)(struct ipp_proc_s *handle,image_info_t *img_in,image_info_t *img_out);
		int (*fmt_convert)(struct ipp_proc_s *handle);
		int (*move)(struct ipp_proc_s *handle,unsigned int dir);
		int (*rotate)(struct ipp_proc_s *handle,unsigned int dir);
		int (*zoom)(struct ipp_proc_s *handle,unsigned int size);
		int (*mirror)(struct ipp_proc_s *handle,unsigned int dir);
		int (*adjust_light)(struct ipp_proc_s *handle,unsigned int level);
		int (*resize)(struct ipp_proc_s *handle,image_info_t *img_out);
		int (*dispose)(struct ipp_proc_s *handle);
}ipp_proc_t;
ipp_proc_t *ipp_open(void);

/*与video encoder相关的数据结构与函数接口*/
typedef struct{		
		int (*read)(void *handle,char *buf,unsigned int offset,unsigned int len);
		int (*write)(void *handle,char *buf,unsigned int offset,unsigned int len);
}vmem_operations_t;

typedef struct{
		/*related to audio*/
		int sample_rate;
		int channel;
		int sample_bits;
		/*related to video*/
		int width;
		int height;
		int frame_rate;
		/*media type, used for av plugin loading*/		
		char *audio;		
		char *video;	
		/*虚拟缓存操作方法集*/
		vmem_operations_t *vmops;
}dv_param_t;

typedef struct mux_plugin_s{
		int (*init)(struct mux_plugin_s *plugin,dv_param_t *param);
		int (*create_header)(struct mux_plugin_s *plugin);
		int (*mux_stream)(struct mux_plugin_s *plugin);
		int (*get_error)(struct mux_plugin_s *plugin);
		int (*dispose)(struct mux_plugin_s *plugin);
}mux_plugin_t;

#define 	AE_PLUGIN_RECORD    0
#define 	AE_PLUGIN_STOP		1
#define 	AE_PLUGIN_PAUSE		2
#define 	AE_PLUGIN_RESUME	3

#define		STEREO				0
#define		JOINT_STEREO		1
#define		DUAL_CHANNEL		2
#define		MONO				3

typedef struct{
		char *audio;				//fmt of audio output
		unsigned int bpp;			//当前音频数据的分辨率
		unsigned int bitrate;		//编码后的比特率	
		unsigned int sample_rate;	//采样率	
		unsigned int channels;		//当前的通道数
		unsigned int encode_mode;	//编码模式立体声、联合立体声等等
}ae_init_t;

typedef struct audio_encoder_s{
		int (*init)(struct audio_encoder_s *plugin,ae_init_t *param);
		int (*encode_data)(struct audio_encoder_s *plugin);
		int (*syn)(struct audio_encoder_s *plugin,unsigned int cmd);
		unsigned int (*get_time)(struct audio_encoder_s *plugin);
		int (*get_err)(struct audio_encoder_s *plugin);
		int (*dispose)(struct audio_encoder_s *plugin);
}audio_encoder_t;

typedef struct video_encoder_s{
		int (*init)(struct video_encoder_s *plugin,void *param);
		int (*encode_data)(struct video_encoder_s *plugin);
		int (*get_err)(struct video_encoder_s *plugin);
		int (*dispose)(struct video_encoder_s *plugin);
}video_encoder_t;

/*packet formate*/
#define		INIT_PACKET 		0x01	
#define		DATA_PACKET 		0x02
#define		AUDIO_FMT_PACKET	0x03
#define		VIDEO_FMT_PACKET	0x04
#define		END_OF_FILE			0x05
#define 	RESET_PACKET 		0x06

typedef struct{
		unsigned int header_type;
		unsigned int block_len;		
		unsigned char buf[20];
}packet_header_t;

/************add for vc1 plungin***********/
typedef struct vc1_extradata_info_s
{
     int vc1_extradata_size;
     int width, height;
     char vc1_extradata[16];
}vc1_extradata_info_t;

typedef struct rgb_info_s
{
     int bit_cnt;
     int pic_width;
     int pic_height;
}rgb_info_t;
/************add for DE display***********/
typedef struct DE_DATA_S
{
	unsigned long change_flag;
	unsigned long input_width;				//input width //%16byte=0;
	unsigned long input_height; 			//input height
	unsigned long input_pix_fmt;			//input pixel format
	unsigned long display_mode; 		//pan_and_scan, letter_box, full_screen, actual_size
	unsigned long DisplaySource;			//DE_IMAGE,DE_DEFAULTCOLOR
	unsigned long DefaultColor; 			//RGB or YUV color mod
	
	unsigned char *input_luma;				//input luminance buffer pointer
	unsigned long input_luma_bus_addr;		//input luminance buffer bus address//%16byte=0;
	unsigned char *input_chroma;			//input chrominance buffer pointer
	unsigned long input_chroma_bus_addr;	//input chrominance buffer bus address//%16byte=0;				
	signed long contrast;					//Adjust the contrast of the output picture [-64,64]
	signed long brightness; 			//Adjust the brightness level of the output picture [-128,127]
	signed long saturation; 				//Adjust the color saturation of the output picture [-64,128]
	unsigned long dar_w;
	unsigned long dar_h;
	unsigned long par_w;
	unsigned long par_h;
	unsigned long s_dar_w;
	unsigned long s_dar_h;
	unsigned long s_par_w;
	unsigned long s_par_h;
	unsigned long reserve1;
	unsigned long reserve2;
}DE_DATA_T;
#ifdef __cplusplus
}
#endif

#endif

