#ifndef __USB_SUBDISP_H__
#define __USB_SUBDISP_H__

#define IOCTL_SUBDISP_GET_IMAGE		_IOR(0xF6, 0x21, unsigned char)
#define IOCTL_SUBDISP_SET_DEVINFO	_IOW(0xF6, 0x22, unsigned char)
#define IOCTL_SUBDISP_GET_LASTERR	_IOR(0xf6, 0x23, unsigned char)

#define 	TAG_DEVINFO_CMD			1
#define 	TAG_PICFMT_CMD			2 
#define 	TAG_HBSC_CMD			3

#define		FLAG_TO_HOST			1
#define 	FLAG_FROM_HOST			0

struct cmd_head{
	/*tag indicate which cmd or status */
	unsigned long		tag;
	unsigned char  	flag;
	unsigned char  	len;
	unsigned char  	reserve0;
	unsigned char  	reserve1;
	/*based on cmd ,cdb may be one of 
	dev_info,pic_fmt,hbsc_info,etc*/
	unsigned char 	cdb[16];
}__attribute__ ((packed));


#define 	 OUT_PIX_FMT_JPEG		1
#define		 OUT_PIX_FMT_YUV420		2
#define 	 OUT_PIX_FMT_YUV422		3
#define      OUT_PIX_FMT_MPEG2      4

struct pic_fmt{
	unsigned long format;
	unsigned long width;
	unsigned long height;
	unsigned long isize;
};

struct  hbsc_info{
	unsigned char h_info;
	unsigned char b_info;
	unsigned char s_info;
	unsigned char c_info;
};

struct dev_info{
	unsigned long width;
	unsigned long height;
	unsigned char h_info;
	unsigned char b_info;
	unsigned char s_info;
	unsigned char c_info;
	unsigned long chip_id;
};

struct buffer_head {
	unsigned long buffer;
	unsigned long size;
};

enum errtype{
	ERR_NONE = 0,
	ERR_QUIT ,
	ERR_DISCON,
	ERR_SWITCH 
};

extern int subdisp_set_devinfo(int fd,struct dev_info *devinfo);
extern int subdisp_get_image(int fd,struct buffer_head *head);
extern int subdisp_get_lasterr(int fd,enum errtype * type);
#endif  //__SYS_RTC_H__
