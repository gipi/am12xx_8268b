#ifndef __FILE_DEBUG_H__
#define __FILE_DEBUG_H__

#include "usb_debug.h"

/*for proc fs */
#define COFIG_PROC_FOR_USB_DEBUG

//#define TRANSFER_ONLY_IN_KERNEL
//#define DBG_USB_DEBUG

#ifdef  DBG_USB_DEBUG
#define DBG(fmt,stuff...)   			printk(KERN_INFO fmt,##stuff)
#else
#define DBG(fmt,stuff...)   			do {} while (0)
#endif

#define ERR(fmt,stuff...)			printk(KERN_NOTICE fmt,##stuff)
#define WARNNING(fmt,stuff...)	printk(KERN_WARNING fmt,##stuff)
#define INFO(fmt,stuff...)			printk(KERN_INFO fmt,##stuff)


#define USB_BULK_IN_FLAG			0x80
#define USB_DATA_TYPE_FLAG		0x70


#define NUM_DBG_BUFFERS			2

enum dbg_buffer_state {
	BUF_STATE_EMPTY = 0,
	BUF_STATE_FULL,
	BUF_STATE_BUSY
};

enum dbg_state {
	DBG_STATE_COMMAND_PHASE = -10,
	DBG_STATE_DATA_PHASE,
	DBG_STATE_STATUS_PHASE,
	DBG_STATE_IDLE = 0,
	DBG_STATE_ABORT_BULK_OUT,
	DBG_STATE_RESET,
	DBG_STATE_DISCONNECT,
	DBG_STATE_SOFTDISCON,
	DBG_STATE_EXIT,
	DBG_STATE_TERMINATED
};

struct dbg_buffhd {
	void						*buf;
	enum dbg_buffer_state		state;
	struct dbg_buffhd			*next;

	struct usb_request		*inreq;
	int	inreq_busy;
	struct usb_request		*outreq;
	int	outreq_busy;
};

struct _info_head{
	unsigned int cmd_type;		/*cmd_type indicate which cmd or status*/
	union{
		unsigned int	datalen;		//len for data to be transfer for cbw
		unsigned int	residue;		//Data residue for csw
	}dataprocess;
	
	unsigned int	tag;			//data transfer sync
	unsigned char	flag;		/*flag for cur trans bitmap:
								bit7: data transfer dir,	data out 1;data in 0;
								bit4~6: data transfer type,000:single transfer,001 multi bulk data 
								bit0~3:reserved
							*/
											
	unsigned char	cdblen;		//cdb len
	unsigned char	reserve1;
	unsigned char	reserve2;
	/*based on cmd ,cdb may be one of 
	dev_info,reg_data,etc*/
	unsigned char cdb[MAX_DEBUG_COMMAND];
}__attribute__ ((packed));

typedef struct _info_head csw_head;
typedef struct _info_head cbw_head;


enum vendor_cmd{
 	VND_CMD_SWITCH= 0x50,
	VND_CMD_QUIT	 = 0x51
};

struct usb_debug_dev{
	spinlock_t			lock;
	struct usb_gadget		*gadget;
#ifdef COFIG_PROC_FOR_USB_DEBUG
	struct proc_dir_entry 		*pde;	
#endif
	struct usb_ep			*bulk_in;
	struct usb_ep			*bulk_out;

	wait_queue_head_t		cmd_wait;
	wait_queue_head_t 	data_wait;
	wait_queue_head_t 	csw_wait;
	
	unsigned long			trans_bitflags;
#define DEBUG_CMD_READY	0
#define DEBUG_DATA_READY	1
#define DEBUG_CSW_READY	2
	
	/*copies from gadget ep0*/
	struct usb_ep				*ep0;	 
	struct usb_request		*ep0req;
	struct usb_request		*outreq;
	struct usb_request		*inreq;
	/*double buf to tx & rx data*/
	struct dbg_buffhd	*next_buffhd_to_fill;
	struct dbg_buffhd	*next_buffhd_to_drain;
	struct dbg_buffhd	 buffhds[NUM_DBG_BUFFERS];

	/*cmd related*/
	unsigned	int	cmd_type;
	u8			bulk_type;
	u8			cmnd_size;
	u8			cmnd[MAX_DEBUG_COMMAND];
	
	enum data_direction	data_dir;
	u32			data_size;
	u32			data_size_from_cmnd;

	u32			tag;
	u32			residue;
	u32			usb_amount_left;
	
	enum dbg_state		state;//indicate exceptions happens
	
	unsigned 				bulk_out_enabled:1,
						bulk_in_enabled:1,
						host_quit :1,
						disconnect:1,
						receive_switch:1,
						running : 1,
						need_exit : 1;
						
	int 					outbusy;
	int 					inbusy;
							
	int 					config;
	
	char 				cmd_buf[40];
	
	/*bit flags used to indicate dev state*/
	unsigned long			atomic_bitflags;
#define USB_DEBUG_DEV_NOTREADY		0
#define USB_DEBUG_DEV_OPEN			1
#define USB_DEBUG_DEV_BUSY			2
#define USB_DEBUG_DEV_ERR				3
#define USB_DEBUG_DEV_SUSPEND		4
#define USB_DEBUG_DEV_IDLE				5
	struct cdev			dev;

	/*define for receive thread*/
	int				thread_wakeup_needed;
	struct completion	thread_notifier;
	struct task_struct	*thread_task;
};


enum errtype{
	ERR_NONE = 0,
	ERR_QUIT ,
	ERR_DISCON,
	ERR_SWITCH 
};

#endif  //__SYS_RTC_H__
