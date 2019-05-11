
/* 图片中间件对外头文件 */

#ifndef __MMM_IMAGE_H__
#define __MMM_IMAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 解码命令集 */
#define IMG_ROT_NONE                0
#define IMG_ROT_RIGHT_90			1
#define IMG_ROT_LEFT_90			    2
#define IMG_ROT_HOR_FLIP			3
#define IMG_ROT_VER_FLIP			4
#define IMG_ROT_180					5
#define IMG_ZOOM_IN					6
#define IMG_ZOOM_OUT			    7
#define IMG_MOVE_UP					8
#define IMG_MOVE_DOWN				9
#define IMG_MOVE_LEFT				10
#define IMG_MOVE_RIGHT				11
#define IMG_ZOOM_RESET           	12
#define IMG_SET_SCALE_RATE          13
#define IMG_DECODE              	14
#define IMG_SET_ROTATION         	15 //直接指定这次相对原图的旋转方向, 而不是类似PP_ROT_LEFT_90等几个是在上次基础上...
#define IMG_GET_PHOTO_INFO          16
#define IMG_DEC_STOP                17

/* 图片格式 */
#define IMG_FMT_YCBCR_4_2_0_SEMIPLANAR              0		//IO
#define IMG_FMT_YCBCR_4_2_2_INTERLEAVED             1		//IO

/* 解码显示模式 */
typedef enum tagImgDecMode
{
	DEC_MODE_ORG          = 0,  	    //原始尺寸, 可能会在解码器内部根据原图情况发生转换
	DEC_MODE_LBOX		  = 1,   	    //letter_box, 可能会在解码器内部根据原图情况发生转换 
	DEC_MODE_EXP		  = 2,      	//pan_and_scan, 可能会在解码器内部根据原图情况发生转换  
	DEC_MODE_FULL		  = 3,      	//全屏模式, 可能会在解码器内部根据原图情况发生转换 
	DEC_MODE_ACTUAL_SIZE  = 4,          //原始尺寸,强制执行, 不会在解码器内部根据原图情况发生转换
	DEC_MODE_LETTER_BOX   = 5,    		//letter_box, 强制执行, 不会在解码器内部根据原图情况发生转换
	DEC_MODE_FULL_SCREEN  = 6,   		//全屏模式, 强制执行, 不会在解码器内部根据原图情况发生转换
	DEC_MODE_PAN_AND_SCAN = 7           //pan_and_scan, 强制执行, 不会在解码器内部根据原图情况发生转换
	
}IMG_DEC_MODE;

typedef struct tagLinearBuf
{
	unsigned char *buf;
	unsigned long bus_addr;
	unsigned long size;
	
}IMG_LINEAR_BUFFER;

/* 上层传给解码库的数据结构 */
typedef struct tagImgDecodeParam
{
    void *handle;
	char file_name[1024];
	int (*img_fread)(void*, void*, int);
	int (*img_fseek_set)(void*, int);
	int (*img_fseek_cur)(void*, int);
	int (*img_fseek_end)(void*, int);
	int (*img_ftell)(void*);
	unsigned long pic_ID;          //图像的ID号
	IMG_LINEAR_BUFFER out_buf_y;   //输出buffer: y分量
	IMG_LINEAR_BUFFER out_buf_uv;  //输出buffer: uv分量
	IMG_LINEAR_BUFFER tmp_buffer;  //解码库使用的中间临时buffer
	unsigned long cmd;             //解码命令
	int is_high_prio_cmd;          //是否是高优先级命令,1=yes,0=no
	unsigned int formate;          //解码图像格式
	unsigned long output_width;
	unsigned long output_height;   //解码输出宽高
	unsigned long lcd_width;       //屏幕宽高
	unsigned long lcd_height;
	IMG_DEC_MODE pic_dec_mode;     //显示模式
	signed long brightness;        //显示效果
	signed long saturation;
	signed long contrast;
	unsigned long num;             //像素宽高比计算使用
	unsigned long den;
	int pip_enable;                //是否开启"画中画"模式
	long pip_start_x;              //画中画起始x坐标(相对屏幕左上角)
	long pip_start_y;              //画中画起始y坐标(相对屏幕左上角)
	unsigned long pip_frame_width; //画中画frame buffer width
	unsigned long pip_frame_height;//画中画frame buffer height
	int usr_rotated;               //用户指定旋转角度,相对初始状态，而不是相对上一次
	int thumb_rot;                 //用户指定缩略图旋转方向(可以与大图不同)
	int auto_rotation_by_exif_en;  //是否开启根据exif信息自动旋转
	unsigned long usr_zoom_rate;   //用户指定放大倍数,相对于原图
	int thumb_browse_mode;         //是否是缩略图浏览模式
	unsigned long hw_rst_enable;   //whether to reset hantro
	unsigned long mem_use_trace;   //whether to trace mem use in photo midware
	unsigned long reg_trace;       //whether to trace hantro registers
	int can_be_stopped;            //下一个任务来时候，这个任务是否可以被打断
	
}IMG_DECODE_PARAM_S;

typedef struct tagImgDate
{
	unsigned int year;
	unsigned int month;
	unsigned int day;
	
}IMG_DATE;

/* 解码器反馈给上层的信息 */
typedef struct tagImgDecodeInfo
{
	int result;                    //发出当前命令后得到的返回值
	int img_ready;                 //解码成功完成
	unsigned long pic_ID;          //图像的ID号
	long img_start_x;              //解码输出的实际图像部分相对屏幕左上角的x坐标
    long img_start_y;              //解码输出的实际图像部分相对屏幕左上角的y坐标
    unsigned long img_actual_wid;  //解码输出的实际图像部分的宽度
    unsigned long img_actual_hei;  //解码输出的实际图像部分的高度
    unsigned long cur_scale_rate;  //整除100是当前缩放比率,注意是相对于原图的!
    IMG_DATE date;                 //日期时间信息
    unsigned int src_width;        //原始图片宽度
	unsigned int src_height;       //原始图片高度
	int ap_stop;
    
}IMG_DECODE_INFO_S;

/* 消息类型 */
#define DECODE_PARAM 0
#define DECODE_INFO  1

/* 关闭图片解码器API */
int image_dec_close(void *img_browser);

/* 打开图片解码器API */
void *image_dec_open(void *param);

/* 向消息队列发送消息API */
int image_dec_send_msg(void *img_browser, void *msg, int size, int type);

/* 从消息队列得到消息API,无pend */
int image_dec_get_msg(void *img_browser, void *msg, int size, int type);

/* stop */
int image_dec_stop(void *img_browser);

#ifdef __cplusplus
}
#endif

#endif //__MMM_IMAGE_H__