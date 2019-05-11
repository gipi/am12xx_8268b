#ifndef _IMAGE_DECODE_H_
#define _IMAGE_DECODE_H_


#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "unistd.h"
#include <dlfcn.h>
#include <fcntl.h>
#include "sys_cfg.h"
#include "mmm_image.h"
#include "act_media_info.h"
#include <time.h>

typedef struct bg_img_dec_s{
	IMG_DECODE_PARAM_S  img_dec_para_info;	///< send the paras to the middleware
	IMG_DECODE_INFO_S img_dec_ret_info;		///< receive the decoded info from middleware
	unsigned long dec_cmd;						///< the dec command which is the same as the cmd in IMG_DECODE_PARAM_S
	IMG_DEC_MODE dec_mode;					///< the dec mode which is the same as the pic_dec_mode in IMG_DECODE_PARAM_S
	unsigned char active;							///< see the enum of req_state_e
	unsigned char photo_effect;					///< photo effect see photo_effect_e union
	unsigned char photo_rotation;				///< photo rotate by the user defined degree, adhere to the end of the file
	unsigned long pic_ID;							///< this is the pic id get from caller
	unsigned long timestamp_ID;					///< used for identifing the pic which has the same name and pic_ID
	char is_fhandle;								///< if it is a file handle, the user must close by himself, otherwise the decoder should close it.
	void *type;									///< used this value to identify which group fops should be used
	char file_name[768];							///< file name
}bg_img_dec_t;

typedef enum bg_img_dec_cmd{
	///0
	BG_IMG_ROT_NONE=0,					///< no rotate
	BG_IMG_ROT_RIGHT_90=1,				///< rotating 90 degree clockwise basic on the decoding operation ahead
	BG_IMG_ROT_LEFT_90=2,					///< rotating 90 degree anticlockwise basic on the decoding operation ahead
	BG_IMG_ROT_HOR_FLIP=3,				///< horizonal mirror  basic on the decoding operation ahead
	BG_IMG_ROT_VER_FLIP=4,				///< vertical mirror basic on the decoding operation ahead
	BG_IMG_ROT_180=5,						///< rotating 180 degree basic on the decoding operation ahead
	BG_IMG_ZOOM_IN=6,						///< zoom in basic on the decoding operation ahead
	BG_IMG_ZOOM_OUT=7,					///< zoom out basic on the decoding operation ahead
	BG_IMG_MOVE_UP=8,						///< move up basic on the decoding operation ahead
	BG_IMG_MOVE_DOWN=9,					///< move down basic on the decoding operation ahead

	//10
	BG_IMG_MOVE_LEFT=10,					///< move left  basic on the decoding operation ahead
	BG_IMG_MOVE_RIGHT=11,				///< move down  basic on the decoding operation ahead
	BG_IMG_ZOOM_RESET=12,				///< reset, decoding again
	BG_IMG_SET_SCALE_RATE=13,			///< scale the photo using the specified rate	
	BG_IMG_SET_ROTATION=14,				///< rotating the photo when decoding first time using specified degree		
	BG_IMG_GET_PHOTO_INFO=15,			///< getting information but not decoding	
	BG_IMG_DEC_STOP=16,					///< stop decode
	//
	BG_IMG_DECODE=17,						///< decoding photo
	////diff from middleware////
	BG_IMG_DEC_PREV=18,					///< decoding preview 
	BG_IMG_DEC_FULL2BUF=19,				///< decoding fullscreen
	BG_IMG_DEC_CLEAR_QUEUE=20,			///< clear the command queue
	BG_IMG_DEC_EXIT=21,					///< exit the decoding thread
	BG_IMG_DEC_HANDLE=22,				///< decoding the photo which is opened ready, the filehandle is needed
}bg_img_dec_cmd_e;

typedef enum{
	REQ_INVALID = 0,			///< the content of the idx in the queue is invaild
	REQ_ENQUEUE,				///< the status of content is enqueue
	REQ_DEQUEUE,				///< the status of content is sending to middle ware
	REQ_DECODED,				///< the status of content had been decoded yet		
}req_state_e;

typedef enum{
	SET_EXIF_ROTATION,		///< store the rotation to the file
	SET_EXIF_EFFECT,			///< store the effect to the file
}photo_exif_setcmd_e;

typedef enum{
	PHOTO_NOEFFCT=0,
	PHOTO_OLDPHOTO,
	PHOTO_NEGATIVE,
	PHOTO_EMBOSS,
	PHOTO_SHARPEN,
	PHOTO_MOSAIC,
	PHOTO_SOLARIZE,
	PHOTO_COLD,
	PHOTO_WARM,
	PHOTO_BW,
	PHOTO_BINARIZE,
	PHOTO_AUTOY,
	PHOTO_AUTOCONTRAST,
	PHOTO_AUTORGB,
	PHOTO_HISTEQU,
	PHOTO_BGENHANCE,
	PHOTO_AUTOFRESH,
}photo_effect_e;		///< the effect when the photo shown, this value will be stored in exif defined by actions-micro

typedef enum{
	PHOTO_BKG_BLACK,
	PHOTO_BKG_WHIET,
	PHOTO_BKG_SILVER,
	PHOTO_BKG_GREEN,
	PHOTO_BKG_RADIANT,
	PHOTO_BKG_RADIANTMIRROR,
}photo_background_effect_e;

typedef enum{
	PHOTO_ROT_NONE,
	PHOTO_ROT_90,
	PHOTO_ROT_180,
	PHOTO_ROT_270,
}photo_rotation_e;		///< the degree to be rotate when the photo be decoding, this value will be stored in exif defined by actions-micro

typedef enum req_query_cmd{
	QUE_CMD_FILENAME,
}req_query_cmd_e;

typedef struct img_io_layer_s{
	void *handle;										///<handle to the file
	long (*img_fread)(void *, unsigned char *, unsigned long);	///<read operation
	long (*img_fseek_set)(void *, long);						///<seek set operation
	long (*img_fseek_cur)(void *, long);					///<seek cur operation
	long (*img_fseek_end)(void *, long);					///<seek end operation
	long (*img_ftell)(void *);								///<ftell operation
}img_io_layer_t;


/**
*@brief set default file operations
*
*@param[out] io_layer:	the pointer to the structure of img_io_layer_t
*@return always return 1
*/
int img_dec_set_ops_default(img_io_layer_t *io_layer);

/**
*@brief reset the command which is in the queue
*
*@param[in] newcmd		:the cmd to be executed,see the enum of req_query_cmd_e
*@param[in] index			:the index of the command which will be replaced in the queue	
*@param[in] param		: if the newcmd == BG_IMG_SET_ROTATION it is the angle to be rotated, see the enum of pe_rotate_e	
*@return NULL
*/
void img_dec_req_reset_deccmd(int newcmd,int index,void *param);


/**
* @brief search the queue to find the idx which is matched the condition
*
*param[in] que_cmd: the cmd to be executed,see the enum of req_query_cmd_e
*param[in] para: if the que_cmd==QUE_CMD_FILENAME , it is the pointer to file name
*return  -1:can find, others the index which is matched the condition
*/
void * img_dec_req_query(int que_cmd,void * para);

/**
* @brief check the decoding result of the specified file
*
* @param[in] userid	: the userid which is received from fui
* @param[out] info		: store the result if the returned value is 1;
* @retval -1 			:failed,can't find the specified file
* @retval  0			: the file is decoding or just in the  queue
* @retval  1			: the file is decoded and the result will be stored in the para of info
*/
int img_get_dec_result_by_userid(int userid,IMG_DECODE_INFO_S *info);


/**
* @brief check the decoding result of the specified file
*
* @param[in] file		: the filename which will be queried
* @param[out] info		: store the result if the returned value is 1;
* @retval -1 			:failed,can't find the specified file
* @retval  0			: the file is decoding or just in the  queue
* @retval  1			: the file is decoded and the result will be stored in the para of info
*/
int  img_get_dec_result(char * file,IMG_DECODE_INFO_S *info);


/**
*@brief  send the command and the parameter to the image thread
*
*@param[in] cmd:	see the enum of bg_img_dec_cmd_e
*@param[in] param: if the cmd is one of the following command,it is a pointer to the structure of bg_img_dec_t,otherwise it is a NULL
*		BG_IMG_DECODE,
*		BG_IMG_ZOOM_IN,
*		BG_IMG_ZOOM_OUT,
*		BG_IMG_ZOOM_RESET,		
*		BG_IMG_MOVE_LEFT,
*		BG_IMG_MOVE_RIGHT,
*		BG_IMG_MOVE_UP,
*		BG_IMG_MOVE_DOWN,
*		BG_IMG_SET_ROTATION,
*		BG_IMG_ROT_NONE,
*		BG_IMG_ROT_LEFT_90,
*		BG_IMG_ROT_180,
*		BG_IMG_ROT_RIGHT_90
*		
*@retval 0: success
*@retval -1: error
*/
int img_dec_io_ctrl(unsigned int cmd, void *param);


/**
* @brief used this function to send a decoded command to the request queue, it will fill the necessary parameters before sending  command
*
* @param[in] dec_cmd		: the decoded command, see the bg_img_dec_cmd_e;
* @param[in] dec_mode[in]	: the decoded mode,see the IMG_DEC_MODE
* @param[in] para			: if the dec_cmd==BG_IMG_DEC_HANDLE, it is a pointer to the structure of img_io_layer_t,
*							else it is pointer to a file name
* @param[in] buffer		: the output buffer where the image will be decoded
* @param[in] w			: the width of image in output buffer 
* @param[in] h			: the height of image in output buffer
* @param[in] id			: the unique id which will be identify by middleware
* @return: -1 failed, 0 success
*/
int img_dec_send_cmd(bg_img_dec_cmd_e dec_cmd,IMG_DEC_MODE dec_mode,void * para,unsigned char *buffer,int w,int h,INT32S id);



/**
*@brief 	create a thread used for image decoding 
*
*@param[in] NULL
*@retval 0	:success
*@retval -1	:failed
*/
int image_thread_create();


/**
*@brief  delete the image decoding thread 
*
*@param[in] NULL
*@return always return 0
*/
int image_thread_exit();


/**
@brief read the exif infomation which is defined by actions-micro
@param[in] filename	: the file to be opened
@param[out] db		: where to store the information
@return
	- 0	: succeed
	- -1	: failed
**/
int img_read_photo_exif(char *filename,PHOTO_DB_T *db);


/**
@brief store the exif infomation which is defined by actions-micro to the file
@param[in] filename	: the file to be opened
@param[in] cmd		: see photo_exif_setcmd_e
@param[in] param		: the value to be stored
	- if cmd==SET_EXIF_ROTATION, the param is the value of photo_rotation_e
	- if cmd==SET_EXIF_EFFECT, the param is the value of photo_effect_e
@return
	- 0	: succeed
	- -1	: failed
**/
int img_store_photo_exif(char *filename,int cmd,char param);

#endif
