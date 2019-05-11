#ifndef __USB_DEBUG_H__
#define __USB_DEBUG_H__

#define IOCTL_USB_DEBUG_GET_CMD	_IOR(0xF6, 0x41, unsigned char)
#define IOCTL_USB_DEBUG_SET_CSW	_IOR(0xF6, 0x42, unsigned char)

#define		DEBUG_CMD_GAMMA			0x1200
#define		DEBUG_CMD_CBSH			0x1201
#define		DEBUG_CMD_SHARPNESS		0x1202

#define		DEBUG_CMD_WRITEREG		0x1300
#define		DEBUG_CMD_READREG		0x1301
#define		DEBUG_CMD_PATTERN		0x1302
#define		DEBUG_CMD_DFCOLOR		0x1303

#define		DEBUG_CMD_WRITEFILE		0x04
#define		DEBUG_CMD_READFILE		0x05


#define MAX_DEBUG_COMMAND 		16



enum data_direction {
	DATA_DIR_UNKNOWN = 0,
	DATA_DIR_FROM_HOST,
	DATA_DIR_TO_HOST,
	DATA_DIR_NONE
};
enum trans_type{
	BULK_UNKNOW_DATA = -1,
	BULK_SINGLE_DATA,
	BULK_MULTI_DATA
};

enum csw_status{
	CMD_PHASE_GOOD,
	CMD_PHASE_FAIL,
	CMD_PHASE_ERROR
};

struct bulk_data_info{
	unsigned int totallen;
	unsigned int remainlen;
};

struct _bulk_head{
	unsigned int cmd_type;
	unsigned int infolen;
	unsigned char info[MAX_DEBUG_COMMAND];
};

struct _cmd_info{
	unsigned char	bulk_type;
	unsigned char	datadir;
	unsigned int	datalen;
	struct _bulk_head head;
};

#endif  //__SYS_RTC_H__
