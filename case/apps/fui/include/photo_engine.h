#ifndef SWF_EXT_PHOTO_ENGINE_H
#define SWF_EXT_PHOTO_ENGINE_H


//#define PHOTO_ENGINE_DEBUG
#ifdef PHOTO_ENGINE_DEBUG
#define pe_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define pe_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#else

#define pe_info(fmt,arg...) do{}while(0);
#define pe_err(fmt,arg...) do{}while(0);
#endif

typedef enum{
	/**显示模式:维持宽高比*/
	PE_RATIO_LETTER_BOX = 0,
	/**显示模式:宽高同屏幕尺寸*/
	PE_RATIO_FULL_SCREEN = 1,
	/**显示模式:原始大小*/
	PE_RATIO_ACTUAL_SIZE = 2,
	/**显示模式:放大*/
	PE_RATIO_ZOOMOUT=3,
	/**显示模式:缩小*/
	PE_RATIO_ZOOMIN=4,	 
	/**显示模式:左旋*/
	PE_RATIO_ROTATE_LEFT=5,	
	/**显示模式:右旋*/
	PE_RATIO_ROTATE_RIGHT=6,		
	/**显示模式:左移*/
	PE_RATIO_MOVE_LEFT=7,			
	/**显示模式:右移*/
	PE_RATIO_MOVE_RIGHT=8,			
	/**显示模式:上移*/
	PE_RATIO_MOVE_UP=9,			
	/**显示模式:下移*/
	PE_RATIO_MOVE_DOWN=10,
	/** resize photo from zoom status */
	PE_RATIO_RESIZE=11
}SWF_PHOTO_DECODE_MODE;

typedef enum {
	PROTATE_NONE=0,
	PROTATE_90=1,
	PROTATE_180=2,
	PROTATE_270=3
}pe_rotate_e;

typedef enum{
	PE_ROTATE_LEFT_90 = 0,
	PE_ROTATE_LEFT_180,
	PE_ROTATE_LEFT_270,
	PE_ROTATE_LEFT_360
}PE_ROTATE_STATE;


enum {
	NO_DECODED = -2,                 //并未解码完成
	DECODED_ERROR = -1,              //解码失败
	DECODED_OK = 0,                  //解码成功
	DECODED_NOT_SUPPORT = 1,         // can not zoom anymore
	DECODED_STOP = 2	             //解码被停止
}DecodeErr_Type;  /*该枚举类型针对解码完成之后的返回类型用的*/

enum{
	PE_GET_SRC_WIDTH,
	PE_GET_SRC_HEIGHT,
	PE_GET_SCALE_RATE,
	PE_GET_ACTUAL_WIDTH,
	PE_GET_ACTUAL_HEIGHT,
}DecodedResult_e;

typedef enum{
	EXIF_TIME=1,
	EXIF_RESOLUTION=2,
	EXIF_USERTAG=4,
}PE_EXIF_INFO_E;


typedef enum{
	GET_FRAME_DELAYTIME=0,  ///< get the delay time of current frame, 1/100s. if the value is 100, it means 1s
	GET_FRAME_DEC_RET=1,	///< get the decoded result of current frame
}PE_GIF_INFO_E;

int get_bitmap_status(int id);

#endif
