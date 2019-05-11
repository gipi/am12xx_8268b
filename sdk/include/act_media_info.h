
/* 媒体库公用头文件 */

#ifndef _ACT_MEDIA_INFO_H_
#define _ACT_MEDIA_INFO_H_

/* 图像信息提取API返回值类型 */
typedef enum
{
	M_INFO_OK                 = 0,  //OK
	M_INFO_PRAM_ERR           = -1, //调用API接口函数参数错误
	M_INFO_CALL_INTERNAL_ERR  = -2, //调用内部函数失败
	M_INFO_NOT_SUPPORT        = -3, //不支持的媒体类型
	M_NO_USR_TAG              = 1   //没有用户tag
	
}MEDIA_INFO_RET;

/* CMD 类型 */
typedef enum
{
	M_PHOTO                   = 0,   //photo info
	M_MUSIC                   = 1,   //music info
	M_VIDEO                   = 2,   //video info
	M_PHOTO_USR_TAG           = 3,   //read photo info and usr tag(s)
	M_USR_TAG_R               = 4,   //read usr tag(s) only
	M_USR_TAG_W               = 5    //write usr tag(s)
	
}MEDIA_INFO_CMD;

/* 调用media info API的输入结构 */
typedef struct tagMInfoInput
{
	void *file_handle;
	long (*p_read)(void *, unsigned char *, unsigned long);
	long (*p_write)(void *, unsigned char *, unsigned long);
    long (*p_tell)(void *);
    long (*p_seek_set)(void *, unsigned long);
    long (*p_seek_cur)(void *, long);
	long (*p_seek_end)(void *, long);           
	void *(*p_malloc)(int);        
	void (*p_free)(void *);
	
}M_INFO_INPUT_S;

/* 原始时间(拍摄时间) */
typedef struct tagDateTimeOriginal
{
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
}DATE_TIME_ORIGINAL_S;

/* 图像信息结构 */
typedef struct tagImgInfo
{
	DATE_TIME_ORIGINAL_S iie_date_time_original;      //拍摄时间
	unsigned short main_img_width;                    //主图宽度(后面可能会扩充缩略图宽度)
	unsigned short main_img_height;                   //主图高度(后面可能会扩充缩略图高度)
	/* 希望提取图像的哪些信息，把哪项enable注册为1，待扩充 */
	int date_time_org_enable;
	int img_wid_enable;
	int img_hei_enable;

}IMG_INFO_S;

/* tags added by usrs */
typedef struct{
	char magic[9];         // always "ACT-MICRO"
	char version;
	char rotation;
	char effect;
	unsigned int tag;
	char reserved[2];      // for 16bytes allignment
}PHOTO_DB_T;

typedef struct tagImgAndTagInfo
{
	IMG_INFO_S img_info;
	PHOTO_DB_T usr_tag;
	
}IMG_INFO_USR_TAG_S;

/* 图像信息提取API 每一个文件调用一次 */
MEDIA_INFO_RET GetMediaInfo(M_INFO_INPUT_S *media_input, MEDIA_INFO_CMD cmd, void *info);

#endif

