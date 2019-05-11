#ifndef 	__LINUX_USB_MEDIAOPS
#define	 __LINUX_USB_MEDIAOPS
#include <linux/ioctl.h>

/*cmd for block dev used */
#define IOCTL_GET_MEDIAOPS  _IOR(0xF5,0,long)	

struct media_ops{
	/*media read*/
	int 	(*media_read)(u32 offset,u8 *buffer,u32 sectors, u8*disk_name);
	/*media write*/
	int		(*media_write)(u32 offset,u8 *buffer,u32 sectors,u8*disk_name);
	/*get media total sector size ,for nand partitions ,
		use partitons name "udisk","vram",for card or others ,not used*/
	int		(*get_media_cap)(u8 *disk_name);
	int 		(*media_update)(void);
	int 		(*media_wpdetect)(void);
	void	*priv;
};
#endif
