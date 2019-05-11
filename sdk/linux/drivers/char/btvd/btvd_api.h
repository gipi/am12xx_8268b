#include <linux/ioctl.h>    /* needed for the _IOW etc stuff used later */

/* Use 'g' as magic number */
#define BTVD_IOC_MAGIC  'b'
/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */
#define BTVD_IOCSOPEN       	_IOW(BTVD_IOC_MAGIC,  1, void *)
#define BTVD_IOCTCLOSE       	_IO(BTVD_IOC_MAGIC,  2)
#define BTVD_GET_IMG      	_IO(BTVD_IOC_MAGIC,  3)
#define BTVD_IO_W_REG      	_IO(BTVD_IOC_MAGIC,  4)
#define BTVD_IO_R_REG      	_IO(BTVD_IOC_MAGIC,  5)
//video_format
#define BT656_480I 0
#define BT656_576I 1

enum buffer_state {
	BUF_EMPTY = 0,
	BUF_FULL,
	BUF_BUSY,
};

struct btvd_data{
	unsigned char reg;
	unsigned char val;
};

typedef struct _buf_btvd{
	unsigned long logic_buf;
	unsigned long phy_buf;
	char state;
}buf_btvd, *pbuf_btvd;

typedef struct _btvd_info
{
	int video_format;
	buf_btvd buf1;
	buf_btvd buf2;
	unsigned long buf_size;
	buf_btvd out_buf;
}BTVD_INFO;
