#ifndef __FILE_STORAGE_H
#define __FILE_STORAGE_H

#include <linux/kernel.h>   
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/timer.h>
/********************************************************************/

#define DRIVER_VENDOR_ID	0x1DE1	// action usb mass-storage
#define DRIVER_PRODUCT_ID	0x1101	// product id for 

/* SCSI device types */
#define TYPE_DISK	0x00
#define TYPE_CDROM	0x05

/* USB protocol value = the transport method */
#define USB_PR_CBI			0x00		// Control/Bulk/Interrupt
#define USB_PR_CB			0x01		// Control/Bulk w/o interrupt
#define USB_PR_BULK			0x50		// Bulk-only

/* USB subclass value = the protocol encapsulation */
#define USB_SC_RBC			0x01		// Reduced Block Commands (flash)
#define USB_SC_8020			0x02		// SFF-8020i, MMC-2, ATAPI (CD-ROM)
#define USB_SC_QIC			0x03		// QIC-157 (tape)
#define USB_SC_UFI			0x04		// UFI (floppy)
#define USB_SC_8070			0x05		// SFF-8070i (removable)
#define USB_SC_SCSI			0x06		// Transparent SCSI


/* Bulk-only data structures */
/* Command Block Wrapper */
struct bulk_cb_wrap {
	__le32	Signature;		// Contains 'USBC'
	u32	Tag;			// Unique per command id
	__le32	DataTransferLength;	// Size of the data
	u8	Flags;			// Direction in bit 7
	u8	Lun;			// LUN (normally 0)
	u8	Length;			// Of the CDB, <= MAX_COMMAND_SIZE
	u8	CDB[16];		// Command Data Block
};

#define USB_BULK_CB_WRAP_LEN		31
#define USB_BULK_CB_SIG				0x43425355	// Spells out USBC
#define USB_BULK_IN_FLAG			0x80

/* Command Status Wrapper */
struct bulk_cs_wrap {
	__le32	Signature;		// Should = 'USBS'
	u32	Tag;			// Same as original command
	__le32	Residue;		// Amount not transferred
	u8	Status;			// See below
};

#define USB_BULK_CS_WRAP_LEN		13
#define USB_BULK_CS_SIG				0x53425355	// Spells out 'USBS'
#define USB_STATUS_PASS				0
#define USB_STATUS_FAIL				1
#define USB_STATUS_PHASE_ERROR	2

/* Bulk-only class specific requests */
#define USB_BULK_RESET_REQUEST			0xff
#define USB_BULK_GET_MAX_LUN_REQUEST	0xfe

/* Length of a SCSI Command Data Block*/
#define MAX_COMMAND_SIZE	16

/* SCSI commands that we recognize */
#define SC_FORMAT_UNIT			0x04
#define SC_INQUIRY			0x12
#define SC_MODE_SELECT_6		0x15
#define SC_MODE_SELECT_10		0x55
#define SC_MODE_SENSE_6			0x1a
#define SC_MODE_SENSE_10		0x5a
#define SC_PREVENT_ALLOW_MEDIUM_REMOVAL	0x1e
#define SC_READ_6			0x08
#define SC_READ_10			0x28
#define SC_READ_12			0xa8
#define SC_READ_CAPACITY		0x25
#define SC_READ_FORMAT_CAPACITIES	0x23
#define SC_RELEASE			0x17
#define SC_REQUEST_SENSE		0x03
#define SC_RESERVE			0x16
#define SC_SEND_DIAGNOSTIC		0x1d
#define SC_START_STOP_UNIT		0x1b
#define SC_SYNCHRONIZE_CACHE		0x35
#define SC_TEST_UNIT_READY		0x00
#define SC_VERIFY			0x2f
#define SC_WRITE_6			0x0a
#define SC_WRITE_10			0x2a
#define SC_WRITE_12			0xaa

#define SC_READ_HEADER		0x44
#define SC_READ_TOC			0x43

//bq:add the command for mac autorun 
#define SC_GET_CONFIGURATION  0x46
#define SC_SET_CDROM_SPEED      0xbb
#define SC_READ_CD      0xbe

/*scsi cmd:private switch cmd for switch to subdisp*/
#define SC_USB_SWITCH_2_SUBDISP 0xff

/*scsi cmd:private switch cmd for switch to usb debug*/
#define SC_USB_SWITCH_2_DEBUG 0xfe

/*scsi cmd :for adfu update*/
#define SC_USB_ADFU_GETDISK  		0xcc
#define SC_USB_ADFU_TESTREADY		0xcb
#define SC_USB_ADFU_GETSYSINFO	0xaf
#define SC_USB_ADFU_SENDATA		0Xb0

/* SCSI Sense Key/Additional Sense Code/ASC Qualifier values */
#define SS_NO_SENSE				0
#define SS_COMMUNICATION_FAILURE		0x040800
#define SS_INVALID_COMMAND			0x052000
#define SS_INVALID_FIELD_IN_CDB			0x052400
#define SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE	0x052100
#define SS_LOGICAL_UNIT_NOT_SUPPORTED		0x052500
#define SS_MEDIUM_NOT_PRESENT			0x023a00
#define SS_MEDIUM_REMOVAL_PREVENTED		0x055302
#define SS_NOT_READY_TO_READY_TRANSITION	0x062800
#define SS_RESET_OCCURRED			0x062900
#define SS_SAVING_PARAMETERS_NOT_SUPPORTED	0x053900
#define SS_UNRECOVERED_READ_ERROR		0x031100
#define SS_WRITE_ERROR				0x030c02
#define SS_WRITE_PROTECTED			0x072700

#define SK(x)		((u8) ((x) >> 16))	// Sense Key byte, etc.
#define ASC(x)		((u8) ((x) >> 8))
#define ASCQ(x)		((u8) (x))

/*-------------------------------------------------------------------------*/

/*
 * These definitions will permit the compiler to avoid generating code for
 * parts of the driver that aren't used in the non-TEST version.  Even gcc
 * can recognize when a test of a constant expression yields a dead code
 * path.
 */
struct lun {
	struct file		*filp;
	/*for private media interface*/
	char 			lun_name[20] ;
	char 			*inquiry_name;
	struct media_ops	 *media_ops;
	
	loff_t			file_length;
	loff_t			num_sectors;
	
	unsigned int		ro : 1;
	unsigned int		cdrom : 1;
	unsigned int 		autorun:1;
	unsigned int		prevent_medium_removal : 1;
	unsigned int		registered : 1;
	unsigned int		info_valid : 1;
	unsigned int 		media_present:1;
	unsigned int		use_media_ops:1;
	unsigned int 		is_written:1;
	
	u32				sense_data;
	u32				sense_data_info;
	u32				unit_attention_data;
	struct device		dev;
};
#define backing_file_is_open(curlun)	((curlun)->filp != NULL)


static struct lun *dev_to_lun(struct device *dev)
{
	return container_of(dev, struct lun, dev);
}

/* Big enough to hold our biggest descriptor */
#define EP0_BUFSIZE		256
#define DELAYED_STATUS	(EP0_BUFSIZE + 999)	// An impossibly large value

/* Number of buffers we will use.  2 is enough for double-buffering */
#define NUM_BUFFERS			2
enum fsg_buffer_state {
	BUF_STATE_EMPTY = 0,
	BUF_STATE_FULL,
	BUF_STATE_BUSY
};

struct fsg_buffhd {
	void				*buf;
	enum fsg_buffer_state		state;
	struct fsg_buffhd		*next;
	/* The NetChip 2280 is faster, and handles some protocol faults
	 * better, if we don't submit any short bulk-out read requests.
	 * So we will record the intended request length here. */
	u32			bulk_out_intended_length;

	struct usb_request		*inreq;
	int				inreq_busy;
	struct usb_request		*outreq;
	int				outreq_busy;
};

enum fsg_state {
	FSG_STATE_COMMAND_PHASE = -10,		// This one isn't used anywhere
	FSG_STATE_DATA_PHASE,
	FSG_STATE_STATUS_PHASE,
	FSG_STATE_IDLE = 0,
	FSG_STATE_ABORT_BULK_OUT,
	FSG_STATE_RESET,
	FSG_STATE_INTERFACE_CHANGE,
	FSG_STATE_CONFIG_CHANGE,
	FSG_STATE_DISCONNECT,
	FSG_STATE_SOFTDISCON,
	FSG_STATE_TODEBUG,
	FSG_STATE_EXIT,
	FSG_STATE_TERMINATED
};

enum data_direction {
	DATA_DIR_UNKNOWN = 0,
	DATA_DIR_FROM_HOST,
	DATA_DIR_TO_HOST,
	DATA_DIR_NONE
};


struct fsg_dev {
	/* lock protects: state, all the req_busy's, and cbbuf_cmnd */
	spinlock_t		lock;
	struct usb_gadget	*gadget;
	
	/* filesem protects: backing files in use */
	struct rw_semaphore	filesem;
	
	/* reference counting: wait until all LUNs are released */
	struct kref		ref;
	
	struct usb_ep		*ep0;		// Handy copy of gadget->ep0
	struct usb_request	*ep0req;	// For control responses
	unsigned int		ep0_req_tag;
	const char		*ep0req_name;

	struct usb_request	*intreq;	// For interrupt responses
	int				intreq_busy;
	struct fsg_buffhd	*intr_buffhd;

 	unsigned int			bulk_out_maxpacket;
	enum fsg_state		state;		// For exception handling

	unsigned int		exception_req_tag;
	u8				config, new_config;

	unsigned int		running : 1;
	unsigned int		bulk_in_enabled : 1;
	unsigned int		bulk_out_enabled : 1;
	unsigned int		intr_in_enabled : 1;
	unsigned int		phase_error : 1;
	unsigned int		short_packet_received : 1;
	unsigned int		bad_lun_okay : 1;
	unsigned long		atomic_bitflags;
#define REGISTERED		0
#define IGNORE_BULK_OUT		1
#define SUSPENDED		2

	struct usb_ep		*bulk_in;
	struct usb_ep		*bulk_out;
	struct usb_ep		*intr_in;

	struct fsg_buffhd	*next_buffhd_to_fill;
	struct fsg_buffhd	*next_buffhd_to_drain;
	struct fsg_buffhd	 buffhds[NUM_BUFFERS];

	int				thread_wakeup_needed;
	struct completion	thread_notifier;
	struct task_struct	*thread_task;

	int			cmnd_size;
	u8			cmnd[MAX_COMMAND_SIZE];
	enum data_direction	data_dir;
	u32			data_size;
	u32			data_size_from_cmnd;
	u32			tag;
	unsigned int		lun;
	u32			residue;
	u32			usb_amount_left;

	/* The CB protocol offers no way for a host to know when a command
	 * has completed.  As a result the next command may arrive early,
	 * and we will still have to handle it.  For that reason we need
	 * a buffer to store new commands when using CB (or CBI, which
	 * does not oblige a host to wait for command completion either). */
	int			cbbuf_cmnd_size;
	u8			cbbuf_cmnd[MAX_COMMAND_SIZE];
	unsigned int		nluns;
	struct lun		*luns;
	struct lun		*curlun;	

	/*info for adfu update*/
	unsigned int 		adfu_update:1;
	u8 			*adfu_buf;
	u32			adfu_sense;	
	int			brec_block_size;
	int			lfi_sector_num;
	int			lfi_read_sector_num;

	struct timer_list	status_t;
	unsigned char		status;//for usr space to check udisk state
};

typedef void (*fsg_routine_t)(struct fsg_dev *);


/* Routines for unaligned data access */
static inline  u16 get_be16(u8 *buf)
{
	return ((u16) buf[0] << 8) | ((u16) buf[1]);
}


static inline  u32 get_be32(u8 *buf)
{
	return ((u32) buf[0] << 24) | ((u32) buf[1] << 16) |
			((u32) buf[2] << 8) | ((u32) buf[3]);
}


static inline void put_be16(u8 *buf, u16 val)
{
	buf[0] = val >> 8;
	buf[1] = val;
}

static inline void put_be32(u8 *buf, u32 val)
{
	buf[0] = val >> 24;
	buf[1] = val >> 16;
	buf[2] = val >> 8;
	buf[3] = val & 0xff;
}

/*add by liucan*/
static inline u16 get_le16(u8 *buf) 
{
	return ((u16) buf[1] << 8) | ((u16) buf[0]);
}


 static inline u16 get_le32(u8 *buf) 
 {
	 return ((u32) buf[3]<<24) |((u32) buf[2]<<16)
	 		| ((u32) buf[1] << 8) | ((u32) buf[0]);
 }

#define	FSG_MAIN_STATUS_REMOVABLE  (1<<3)

enum fsg_main_status {
	FSG_MAIN_STATUS_UNKNOWN = 0,
	FSG_MAIN_STATUS_ALLOW_ACCESS,
	FSG_MAIN_STATUS_PREVENT_ACCESS
};
enum fsg_sub_status {
	FSG_SUB_STATUS_IDLE = 1,
	FSG_SUB_STATUS_READ,
	FSG_SUB_STATUS_WRITE,
	FSG_SUB_STATUS_UPDATE,
	FSG_SUB_STATUS_DISCON,
	FSG_SUB_STATUS_SOFTDISCON,
	FSG_SUB_STATUS_SUSPEND
};

#define	get_main_status(status) 			(status & 0x07)
#define	set_main_status(status,value)		((status & 0xf8) |(value & 0x07))
#define	get_sub_status(status)  				((status & 0xf0) >> 4)
#define	set_sub_status(status,value)			((status & 0x0f) |((value & 0x0f) << 4))
#define	set_allow_remove_status(status)		(status & ~FSG_MAIN_STATUS_REMOVABLE)
#define	set_prevent_remove_status(status)	(status | FSG_MAIN_STATUS_REMOVABLE)

static inline void  update_ustatus(struct fsg_dev *fsg,unsigned char curstatus)
{
	fsg->status = set_sub_status(fsg->status,curstatus);
	mod_timer(&fsg->status_t,jiffies + 250);
}

 void start_transfer(struct fsg_dev *fsg, struct usb_ep *ep,struct usb_request *req, int *pbusy,enum fsg_buffer_state *state);
 int do_adfu_get_devid(struct fsg_dev *fsg, struct fsg_buffhd *bh);
 int do_adfu_test_dev_ready(struct fsg_dev *fsg, struct fsg_buffhd *bh) ;
 int do_adfu_get_sysinfo(struct fsg_dev *fsg, struct fsg_buffhd *bh) ;
 int do_adfu_send_data(struct fsg_dev *fsg, struct fsg_buffhd *bh);
 int check_adfu_command(struct fsg_dev *fsg, enum data_direction data_dir, const char *name);

/****************************************************************************/
#endif
