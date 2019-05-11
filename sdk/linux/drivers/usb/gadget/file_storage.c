/*
 * file_storage.c -- File-backed USB Storage Gadget, for USB development
 *
 * Copyright (C) 2003-2007 Alan Stern
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the above-listed copyright holders may not be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/*
 * The File-backed Storage Gadget acts as a USB Mass Storage device,
 * appearing to the host as a disk drive.  In addition to providing an
 * example of a genuinely useful gadget driver for a USB device, it also
 * illustrates a technique of double-buffering for increased throughput.
 * Last but not least, it gives an easy way to probe the behavior of the
 * Mass Storage drivers in a USB host.
 *
 * Backing storage is provided by a regular file or a block device, specified
 * by the "file" module parameter.  Access can be limited to read-only by
 * setting the optional "ro" module parameter.  The gadget will indicate that
 * it has removable media if the optional "removable" module parameter is set.
 *
 * The gadget supports the Control-Bulk (CB), Control-Bulk-Interrupt (CBI),
 * and Bulk-Only (also known as Bulk-Bulk-Bulk or BBB) transports, selected
 * by the optional "transport" module parameter.  It also supports the
 * following protocols: RBC (0x01), ATAPI or SFF-8020i (0x02), QIC-157 (0c03),
 * UFI (0x04), SFF-8070i (0x05), and transparent SCSI (0x06), selected by
 * the optional "protocol" module parameter.  In addition, the default
 * Vendor ID, Product ID, and release number can be overridden.
 *
 * There is support for multiple logical units (LUNs), each of which has
 * its own backing file.  The number of LUNs can be set using the optional
 * "luns" module parameter (anywhere from 1 to 8), and the corresponding
 * files are specified using comma-separated lists for "file" and "ro".
 * The default number of LUNs is taken from the number of "file" elements;
 * it is 1 if "file" is not given.  If "removable" is not set then a backing
 * file must be specified for each LUN.  If it is set, then an unspecified
 * or empty backing filename means the LUN's medium is not loaded.
 *
 * Requirements are modest; only a bulk-in and a bulk-out endpoint are
 * needed (an interrupt-out endpoint is also needed for CBI).  The memory
 * requirement amounts to two 16K buffers, size configurable by a parameter.
 * Support is included for both full-speed and high-speed operation.
 *
 * Note that the driver is slightly non-portable in that it assumes a
 * single memory/DMA buffer will be useable for bulk-in, bulk-out, and
 * interrupt-in endpoints.  With most device controllers this isn't an
 * issue, but there may be some with hardware restrictions that prevent
 * a buffer from being used by more than one endpoint.
 *
 * Module options:
 *
 *	file=filename[,filename...]
 *				Required if "removable" is not set, names of
 *					the files or block devices used for
 *					backing storage
 *	ro=b[,b...]		Default false, booleans for read-only access
 *	removable		Default false, boolean for removable media
 *	luns=N			Default N = number of filenames, number of
 *					LUNs to support
 *	stall			Default determined according to the type of
 *					USB device controller (usually true),
 *					boolean to permit the driver to halt
 *					bulk endpoints
 *	transport=XXX		Default BBB, transport name (CB, CBI, or BBB)
 *	protocol=YYY		Default SCSI, protocol name (RBC, 8020 or
 *					ATAPI, QIC, UFI, 8070, or SCSI;
 *					also 1 - 6)
 *	vendor=0xVVVV		Default 0x0525 (NetChip), USB Vendor ID
 *	product=0xPPPP		Default 0xa4a5 (FSG), USB Product ID
 *	release=0xRRRR		Override the USB release number (bcdDevice)
 *	buflen=N		Default N=16384, buffer size used (will be
 *					rounded down to a multiple of
 *					PAGE_CACHE_SIZE)
 *
 * If CONFIG_USB_FILE_STORAGE_TEST is not set, only the "file", "ro",
 * "removable", "luns", and "stall" options are available; default values
 * are used for everything else.
 *
 * The pathnames of the backing files and the ro settings are available in
 * the attribute files "file" and "ro" in the lun<n> subdirectory of the
 * gadget's sysfs directory.  If the "removable" option is set, writing to
 * these files will simulate ejecting/loading the medium (writing an empty
 * line means eject) and adjusting a write-enable tab.  Changes to the ro
 * setting are not allowed when the medium is loaded.
 *
 * This gadget driver is heavily based on "Gadget Zero" by David Brownell.
 * The driver's SCSI command interface was based on the "Information
 * technology - Small Computer System Interface - 2" document from
 * X3T9.2 Project 375D, Revision 10L, 7-SEP-93, available at
 * <http://www.t10.org/ftp/t10/drafts/s2/s2-r10l.pdf>.  The single exception
 * is opcode 0x23 (READ FORMAT CAPACITIES), which was based on the
 * "Universal Serial Bus Mass Storage Class UFI Command Specification"
 * document, Revision 1.0, December 14, 1998, available at
 * <http://www.usb.org/developers/devclass_docs/usbmass-ufi10.pdf>.
 */


/*
 *				Driver Design
 *
 * The FSG driver is fairly straightforward.  There is a main kernel
 * thread that handles most of the work.  Interrupt routines field
 * callbacks from the controller driver: bulk- and interrupt-request
 * completion notifications, endpoint-0 events, and disconnect events.
 * Completion events are passed to the main thread by wakeup calls.  Many
 * ep0 requests are handled at interrupt time, but SetInterface,
 * SetConfiguration, and device reset requests are forwarded to the
 * thread in the form of "exceptions" using SIGUSR1 signals (since they
 * should interrupt any ongoing file I/O operations).
 *
 * The thread's main routine implements the standard command/data/status
 * parts of a SCSI interaction.  It and its subroutines are full of tests
 * for pending signals/exceptions -- all this polling is necessary since
 * the kernel has no setjmp/longjmp equivalents.  (Maybe this is an
 * indication that the driver really wants to be running in userspace.)
 * An important point is that so long as the thread is alive it keeps an
 * open reference to the backing file.  This will prevent unmounting
 * the backing file's underlying filesystem and could cause problems
 * during system shutdown, for example.  To prevent such problems, the
 * thread catches INT, TERM, and KILL signals and converts them into
 * an EXIT exception.
 *
 * In normal operation the main thread is started during the gadget's
 * fsg_bind() callback and stopped during fsg_unbind().  But it can also
 * exit when it receives a signal, and there's no point leaving the
 * gadget running when the thread is dead.  So just before the thread
 * exits, it deregisters the gadget driver.  This makes things a little
 * tricky: The driver is deregistered at two places, and the exiting
 * thread can indirectly call fsg_unbind() which in turn can tell the
 * thread to exit.  The first problem is resolved through the use of the
 * REGISTERED atomic bitflag; the driver will only be deregistered once.
 * The second problem is resolved by having fsg_unbind() check
 * fsg->state; it won't try to stop the thread if the state is already
 * FSG_STATE_TERMINATED.
 *
 * To provide maximum throughput, the driver uses a circular pipeline of
 * buffer heads (struct fsg_buffhd).  In principle the pipeline can be
 * arbitrarily long; in practice the benefits don't justify having more
 * than 2 stages (i.e., double buffering).  But it helps to think of the
 * pipeline as being a long one.  Each buffer head contains a bulk-in and
 * a bulk-out request pointer (since the buffer can be used for both
 * output and input -- directions always are given from the host's
 * point of view) as well as a pointer to the buffer and various state
 * variables.
 *
 * Use of the pipeline follows a simple protocol.  There is a variable
 * (fsg->next_buffhd_to_fill) that points to the next buffer head to use.
 * At any time that buffer head may still be in use from an earlier
 * request, so each buffer head has a state variable indicating whether
 * it is EMPTY, FULL, or BUSY.  Typical use involves waiting for the
 * buffer head to be EMPTY, filling the buffer either by file I/O or by
 * USB I/O (during which the buffer head is BUSY), and marking the buffer
 * head FULL when the I/O is complete.  Then the buffer will be emptied
 * (again possibly by USB I/O, during which it is marked BUSY) and
 * finally marked EMPTY again (possibly by a completion routine).
 *
 * A module parameter tells the driver to avoid stalling the bulk
 * endpoints wherever the transport specification allows.  This is
 * necessary for some UDCs like the SuperH, which cannot reliably clear a
 * halt on a bulk endpoint.  However, under certain circumstances the
 * Bulk-only specification requires a stall.  In such cases the driver
 * will halt the endpoint and set a flag indicating that it should clear
 * the halt in software during the next device reset.  Hopefully this
 * will permit everything to work correctly.  Furthermore, although the
 * specification allows the bulk-out endpoint to halt when the host sends
 * too much data, implementing this would cause an unavoidable race.
 * The driver will always use the "no-stall" approach for OUT transfers.
 *
 * One subtle point concerns sending status-stage responses for ep0
 * requests.  Some of these requests, such as device reset, can involve
 * interrupting an ongoing file I/O operation, which might take an
 * arbitrarily long time.  During that delay the host might give up on
 * the original ep0 request and issue a new one.  When that happens the
 * driver should not notify the host about completion of the original
 * request, as the host will no longer be waiting for it.  So the driver
 * assigns to each ep0 request a unique tag, and it keeps track of the
 * tag value of the request associated with a long-running exception
 * (device-reset, interface-change, or configuration-change).  When the
 * exception handler is finished, the status-stage response is submitted
 * only if the current ep0 request tag is equal to the exception request
 * tag.  Thus only the most recently received ep0 request will get a
 * status-stage response.
 *
 * Warning: This driver source file is too long.  It ought to be split up
 * into a header file plus about 3 separate .c files, to handle the details
 * of the Gadget, USB Mass Storage, and SCSI protocols.
 */


/* #define VERBOSE_DEBUG */
/* #define DUMP_MSGS */


#include <linux/blkdev.h>
#include <linux/completion.h>
#include <linux/dcache.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/kref.h>
#include <linux/kthread.h>
#include <linux/limits.h>
#include <linux/rwsem.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/freezer.h>
#include <linux/utsname.h>
#include <linux/usb_mediaops.h> /*for struct media_ops*/
#include <linux/vmalloc.h>
#include "file_storage.h"
#include <sys_msg.h>
#include <asm/unaligned.h>

#include <am7x_dev.h>
#include "gadget_chips.h"
#include "am7x_flash_api.h"
#include "am7x_board.h"

#define ADFU_UPDATE

/*-------------------------------------------------------------------------*/
#define DRIVER_DESC			"File-backed Storage Gadget"
#define DRIVER_NAME			"actions mass storage"
#define DRIVER_VERSION		"20/3/2010"

static const char longname[]  = DRIVER_DESC;
static const char shortname[] = DRIVER_NAME;


static char usb_configname[] = "/am7x/case/data/usb_mass.bin";

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("Alan Stern");
MODULE_LICENSE("GPL");

/*
 * This driver assumes self-powered hardware and has no way for users to
 * trigger remote wakeup.  It uses autoconfiguration to select endpoints
 * and endpoint addresses.
 */

/*-------------------------------------------------------------------------*/
#ifdef DEBUG
#define fsg_dbg(format, arg...)		\
	printk(KERN_INFO  format , ## arg)	
#else
#define fsg_dbg( format, arg...) 		\
		do{}while(0)
#endif

#define fsg_err( format, arg...)		\
	printk(KERN_ERR  format , ## arg)	
#define fsg_warn( format, arg...)		\
	printk(KERN_WARNING format , ## arg)	
#define fsg_notice( format, arg...)		\
	printk(KERN_NOTICE  format , ## arg)	
#define fsg_info( format, arg...)		\
	printk(KERN_INFO  format , ## arg)	
	
/*-------------------------------------------------------------------------*/
/* Encapsulate the module parameter settings */
#define MAX_LUNS	4

#if 0
static char				manufacturer[64];
static char				serial[13];
#endif

struct {
	char				*file[MAX_LUNS];
	int				ro[MAX_LUNS];
	int 				autorun[MAX_LUNS];
	
	unsigned int		num_filenames;
	unsigned int		num_ros;
	unsigned int		num_autoruns;
	unsigned int		nluns;

	int				removable;
	int				can_stall;
	int 				support_switch;
	int				cdrom;
	int                            msg_identify;//bq
	
	unsigned short	vendor;
	unsigned short	product;

	char				inquiry_vendor[9];
	char 			inquiry_products[MAX_LUNS][17];	
	unsigned short  	release;
	
	/* The CBI specification limits the serial string to 12 uppercase hexadecimal
	 * characters. */
	char 			manufacturer[64];
	char				productname[20];
	char				serials[13];
	
	unsigned int		buflen;
	char				*transport_parm;
	char				*protocol_parm;
	char				*transport_name;	
	char				*protocol_name;

	int				transport_type;
	int				protocol_type;
} mod_data = {					// Default values
	.removable			= 1,	
	.can_stall				= 0,	
	.support_switch		= 1,
	.cdrom				= 0,
	.vendor				= DRIVER_VENDOR_ID,
	.product				= DRIVER_PRODUCT_ID,
	.manufacturer			= "artek",
	.productname			= "am7x",
	.serials				= "0123456789.",

	.inquiry_vendor		= "actions",				
	.release				= 0xffff,	// Use controller chip type
	.buflen				= 3, /*PAGE ORDERS*/

	.transport_parm		= "BBB",
	.protocol_parm		= "SCSI",	
	.transport_name 		= "Bulk-only",
	.protocol_name 		= "8070i",
	.transport_type 		= USB_PR_BULK,
	.protocol_type 		= USB_SC_8070,
	
};


module_param_array_named(file, mod_data.file, charp, &mod_data.num_filenames,S_IRUGO);
MODULE_PARM_DESC(file, "names of backing files or devices");

module_param_array_named(ro, mod_data.ro, bool, &mod_data.num_ros, S_IRUGO);
MODULE_PARM_DESC(ro, "true to force read-only");

module_param_array_named(autorun, mod_data.autorun, bool, &mod_data.num_autoruns, S_IRUGO);
MODULE_PARM_DESC(autorun, "true to force autorun");

module_param_named(luns, mod_data.nluns, uint, S_IRUGO);
MODULE_PARM_DESC(luns, "number of LUNs");

module_param_named(buflen,mod_data.buflen,uint,S_IRUGO);
MODULE_PARM_DESC(buflen, "buflen size");

module_param_named(support_switch, mod_data.support_switch, bool, S_IRUGO);
MODULE_PARM_DESC(support_switch, "number of LUNs");

module_param_named(cdrom, mod_data.cdrom, bool, S_IRUGO);
MODULE_PARM_DESC(cdrom, "true to emulate cdrom instead of disk");

module_param_named(msg_identify, mod_data.msg_identify, bool, S_IRUGO);//bq
MODULE_PARM_DESC(msg_identify, "identify usb message type");
/*************************************************************************/
 u32 get_buffer_len(void)
{
	return (u32)PAGE_SIZE*(1<<mod_data.buflen);
}

static int exception_in_progress(struct fsg_dev *fsg)
{
	return (fsg->state > FSG_STATE_IDLE);
}

/* Make bulk-out requests be divisible by the maxpacket size */
static void set_bulk_out_req_length(struct fsg_dev *fsg,
		struct fsg_buffhd *bh, unsigned int length)
{
	unsigned int	rem;

	bh->bulk_out_intended_length = length;
	rem = length % fsg->bulk_out_maxpacket;
	if (rem > 0)
		length += fsg->bulk_out_maxpacket - rem;
	bh->outreq->length = length;
}

static struct fsg_dev			*the_fsg;
static struct usb_gadget_driver		fsg_driver;

static int 		open_backing_file(struct lun *curlun, const char *filename);
static void	close_backing_file(struct lun *curlun);
static void	close_all_backing_files(struct fsg_dev *fsg);

/*-------------------------------------------------------------------------*/

#ifdef DUMP_MSGS
static void dump_msg(struct fsg_dev *fsg, const char *label,
		const u8 *buf, unsigned int length)
{
	if (length < 512) {
		fsg_dbg( "%s, length %u:\n", label, length);
		print_hex_dump(KERN_DEBUG, "", DUMP_PREFIX_OFFSET,
				16, 1, buf, length, 0);
	}
}

static void dump_cdb(struct fsg_dev *fsg)
{}

#else

static void dump_msg(struct fsg_dev *fsg, const char *label,
		const u8 *buf, unsigned int length)
{
	if (length >4096) {
		fsg_dbg( "%s, length %u:\n", label, length);
		print_hex_dump(KERN_DEBUG, "", DUMP_PREFIX_OFFSET,
				16, 1, buf, 16, 0);
	}
}

#ifdef VERBOSE_DEBUG
static void dump_cdb(struct fsg_dev *fsg)
{
	print_hex_dump(KERN_ALERT, "SCSI CDB: ", DUMP_PREFIX_NONE,
			16, 1, fsg->cmnd, fsg->cmnd_size, 0);
}

#else
static void dump_cdb(struct fsg_dev *fsg)
{}

#endif /* VERBOSE_DEBUG */
#endif /* DUMP_MSGS */


static int fsg_set_halt(struct fsg_dev *fsg, struct usb_ep *ep)
{
	const char	*name;

	if (ep == fsg->bulk_in)
		name = "bulk-in";
	else if (ep == fsg->bulk_out)
		name = "bulk-out";
	else
		name = ep->name;
	fsg_dbg( "%s set halt\n", name);
	return usb_ep_set_halt(ep);
}

/*-------------------------------------------------------------------------*/

/*
 * DESCRIPTORS ... most are static, but strings and (full) configuration
 * descriptors are built on demand.  Also the (static) config and interface
 * descriptors are adjusted during fsg_bind().
 */
#define STRING_MANUFACTURER		1
#define STRING_PRODUCT				2
#define STRING_SERIAL					3
#define STRING_CONFIG				4
#define STRING_INTERFACE			5

/* There is only one configuration. */
#define	CONFIG_VALUE				1

static struct usb_device_descriptor
device_desc = {
	.bLength =		sizeof device_desc,
	.bDescriptorType =	USB_DT_DEVICE,

	.bcdUSB =		__constant_cpu_to_le16(0x0200),
	.bDeviceClass =		USB_CLASS_PER_INTERFACE,

	/* The next three values can be overridden by module parameters */
	.idVendor =		__constant_cpu_to_le16(DRIVER_VENDOR_ID),
	.idProduct =		__constant_cpu_to_le16(DRIVER_PRODUCT_ID),
	.bcdDevice =		__constant_cpu_to_le16(0xffff),

	.iManufacturer =	STRING_MANUFACTURER,
	.iProduct =		STRING_PRODUCT,
	.iSerialNumber =	STRING_SERIAL,
	.bNumConfigurations =	1,
};

static struct usb_config_descriptor
config_desc = {
	.bLength =		sizeof config_desc,
	.bDescriptorType =	USB_DT_CONFIG,

	/* wTotalLength computed by usb_gadget_config_buf() */
	.bNumInterfaces =	1,
	.bConfigurationValue =	CONFIG_VALUE,
	.iConfiguration =	STRING_CONFIG,
	.bmAttributes =		USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower =		1,	// self-powered
};

static struct usb_otg_descriptor
otg_desc = {
	.bLength =		sizeof(otg_desc),
	.bDescriptorType =	USB_DT_OTG,
	.bmAttributes =		USB_OTG_SRP,
};

/* There is only one interface. */
static struct usb_interface_descriptor
intf_desc = {
	.bLength =		sizeof intf_desc,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bNumEndpoints =	2,	
	.bInterfaceClass =	USB_CLASS_MASS_STORAGE,
	.bInterfaceSubClass =	USB_SC_8070,	// Adjusted during fsg_bind()
	.bInterfaceProtocol =	USB_PR_BULK,	// Adjusted during fsg_bind()
	.iInterface =		STRING_INTERFACE,
};


/* Three full-speed endpoint descriptors: bulk-in, bulk-out,
 * and interrupt-in. */
static struct usb_endpoint_descriptor
fs_bulk_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	/* wMaxPacketSize set by autoconfiguration */
};


static struct usb_endpoint_descriptor
fs_bulk_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	/* wMaxPacketSize set by autoconfiguration */
};


static const struct usb_descriptor_header *fs_function[] = {
	(struct usb_descriptor_header *) &otg_desc,
	(struct usb_descriptor_header *) &intf_desc,
	(struct usb_descriptor_header *) &fs_bulk_in_desc,
	(struct usb_descriptor_header *) &fs_bulk_out_desc,
	NULL,
};

#define FS_FUNCTION_PRE_EP_ENTRIES	2


/*
 * USB 2.0 devices need to expose both high speed and full speed
 * descriptors, unless they only run at full speed.
 *
 * That means alternate endpoint descriptors (bigger packets)
 * and a "device qualifier" ... plus more construction options
 * for the config descriptor.
 */

#ifdef	CONFIG_USB_GADGET_DUALSPEED
/*
 * USB 2.0 devices need to expose both high speed and full speed
 * descriptors, unless they only run at full speed.
 *
 * That means alternate endpoint descriptors (bigger packets)
 * and a "device qualifier" ... plus more construction options
 * for the config descriptor.
 */
static struct usb_qualifier_descriptor
dev_qualifier = {
	.bLength =		sizeof dev_qualifier,
	.bDescriptorType =	USB_DT_DEVICE_QUALIFIER,

	.bcdUSB =		__constant_cpu_to_le16(0x0200),
	.bDeviceClass =		USB_CLASS_PER_INTERFACE,

	.bNumConfigurations =	1,
};

static struct usb_endpoint_descriptor
hs_bulk_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	/* bEndpointAddress copied from fs_bulk_in_desc during fsg_bind() */
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};


static struct usb_endpoint_descriptor
hs_bulk_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	/* bEndpointAddress copied from fs_bulk_out_desc during fsg_bind() */
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
	.bInterval =		1,	// NAK every 1 uframe
};



static const struct usb_descriptor_header *hs_function[] = {
	(struct usb_descriptor_header *) &otg_desc,
	(struct usb_descriptor_header *) &intf_desc,
	(struct usb_descriptor_header *) &hs_bulk_in_desc,
	(struct usb_descriptor_header *) &hs_bulk_out_desc,
	NULL,
};

#define HS_FUNCTION_PRE_EP_ENTRIES	2
/* Maxpacket and other transfer characteristics vary by speed. */
#define ep_desc(g,fs,hs)	(((g)->speed==USB_SPEED_HIGH) ? (hs) : (fs))
#else
/* If there's no high speed support, always use the full-speed descriptor. */
#define ep_desc(g,fs,hs)	fs
#endif	/* !CONFIG_USB_GADGET_DUALSPEED */

/* Static strings, in UTF-8 (for simplicity we use only ASCII characters) */
static struct usb_string		strings[] = {
	{STRING_MANUFACTURER,	mod_data.manufacturer},
	{STRING_PRODUCT,	mod_data.productname},
	{STRING_SERIAL,		mod_data.serials},
	{STRING_CONFIG,		"Self-powered"},
	{STRING_INTERFACE,	"Mass Storage"},
	//{STRING_INTERFACE,	"ben10"},
	{}
};

static struct usb_gadget_strings	stringtab = {
	.language	= 0x0409,		// en-us
	.strings	= strings,
};


/*
 * Config descriptors must agree with the code that sets configurations
 * and with code managing interfaces and their altsettings.  They must
 * also handle different speeds and other-speed requests.
 */
static int populate_config_buf(struct usb_gadget *gadget,
		u8 *buf, u8 type, unsigned index)
{
	enum usb_device_speed			speed = gadget->speed;
	int					len;
	const struct usb_descriptor_header	**function;

	if (index > 0)
		return -EINVAL;

	if (/*gadget_is_dualspeed(gadget) &&*/ type == USB_DT_OTHER_SPEED_CONFIG)
		speed = (USB_SPEED_FULL + USB_SPEED_HIGH) - speed;
	if (/*gadget_is_dualspeed(gadget) && */speed == USB_SPEED_HIGH)
		function = hs_function;
	else
		function = fs_function;

	/* for now, don't advertise srp-only devices */
	if (!gadget_is_otg(gadget))
		function++;

	len = usb_gadget_config_buf(&config_desc, buf, EP0_BUFSIZE, function);
	((struct usb_config_descriptor *) buf)->bDescriptorType = type;
	return len;
}


/*-------------------------------------------------------------------------*/

/* These routines may be called in process context or in_irq */

/* Caller must hold fsg->lock */
static void wakeup_thread(struct fsg_dev *fsg)
{
	/* Tell the main thread that something has happened */
	fsg->thread_wakeup_needed = 1;
	if (fsg->thread_task)
		wake_up_process(fsg->thread_task);
}


static void raise_exception(struct fsg_dev *fsg, enum fsg_state new_state)
{
	unsigned long		flags;

	/* Do nothing if a higher-priority exception is already in progress.
	 * If a lower-or-equal priority exception is in progress, preempt it
	 * and notify the main thread by sending it a signal. */
	spin_lock_irqsave(&fsg->lock, flags);
	if (fsg->state <= new_state) {
		fsg->exception_req_tag = fsg->ep0_req_tag;
		fsg->state = new_state;
		if (fsg->thread_task)
			send_sig_info(SIGUSR1, SEND_SIG_FORCED,fsg->thread_task);
	}
	spin_unlock_irqrestore(&fsg->lock, flags);
}


/*-------------------------------------------------------------------------*/

/* The disconnect callback and ep0 routines.  These always run in_irq,
 * except that ep0_queue() is called in the main thread to acknowledge
 * completion of various requests: set config, set interface, and
 * Bulk-only device reset. */

static void fsg_disconnect(struct usb_gadget *gadget)
{
	struct fsg_dev		*fsg = get_gadget_data(gadget);
	fsg_dbg( "disconnect or port reset\n");
	raise_exception(fsg, FSG_STATE_DISCONNECT);
}


static int ep0_queue(struct fsg_dev *fsg)
{
	int	rc;

	rc = usb_ep_queue(fsg->ep0, fsg->ep0req, GFP_ATOMIC);
	if (rc != 0 && rc != -ESHUTDOWN) {
		/* We can't do much more than wait for a reset */
		fsg_warn("error in submission: %s --> %d\n",fsg->ep0->name, rc);
	}
	return rc;
}

static void ep0_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct fsg_dev		*fsg = ep->driver_data;

	if (req->actual > 0)
		dump_msg(fsg, fsg->ep0req_name, req->buf, req->actual);
	if (req->status || req->actual != req->length)
		fsg_dbg( "%s --> %d, %u/%u\n", __func__,
				req->status, req->actual, req->length);
	if (req->status == -ECONNRESET)		// Request was cancelled
		usb_ep_fifo_flush(ep);

	if (req->status == 0 && req->context)
		((fsg_routine_t) (req->context))(fsg);
}


/*-------------------------------------------------------------------------*/

/* Bulk and interrupt endpoint completion handlers.
 * These always run in_irq. */

static void bulk_in_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct fsg_dev		*fsg = ep->driver_data;
	struct fsg_buffhd	*bh = req->context;
	
	if (req->status || req->actual != req->length){
		fsg_dbg("%s --> %d, %u/%u\n", __func__,
				req->status, req->actual, req->length);
	}
	
	if (req->status == -ECONNRESET)		// Request was cancelled
		usb_ep_fifo_flush(ep);
	
	/* Hold the lock while we update the request and buffer states */
	smp_wmb();
	spin_lock(&fsg->lock);
	bh->inreq_busy = 0;
	bh->state = BUF_STATE_EMPTY;
	wakeup_thread(fsg);
	spin_unlock(&fsg->lock);	
}

static void bulk_out_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct fsg_dev		*fsg = ep->driver_data;
	struct fsg_buffhd	*bh = req->context;
	
	dump_msg(fsg, "bulk-out", req->buf, req->actual);
	
	if (req->status || req->actual != bh->bulk_out_intended_length)
		fsg_dbg( "%s --> %d, %u/%u\n", __func__,
				req->status, req->actual,bh->bulk_out_intended_length);
	#if 0
	if((req->actual > 1024*8) && (get_be32(req->buf) == 0x55534243)){
		 print_hex_dump(KERN_ALERT, "bulk out: ", DUMP_PREFIX_ADDRESS,
 			16, 1, req->buf, 31, 1);
	}
	#endif
	
	if (req->status == -ECONNRESET)		// Request was cancelled
		usb_ep_fifo_flush(ep);
	
	/* Hold the lock while we update the request and buffer states */
	smp_wmb();
	
	spin_lock(&fsg->lock);
	bh->outreq_busy = 0;
	bh->state = BUF_STATE_FULL;
	wakeup_thread(fsg);
	spin_unlock(&fsg->lock);

}
#if 1
static int _remove_udisk_symbol()
{
	struct am_sys_msg  msg;
	msg.type = SYSMSG_USB;
	msg.subtype = DEVICE_USB_OUT;
	msg.dataload = DEVICE_R_UDISK;//HOST_RAW;
	//msg.reserved[0] =USB_GROUP0;
	printk("remove udisk symbol\n");
	am_put_sysmsg(msg);
	return 0;
}
#endif
static int class_setup_req(struct fsg_dev *fsg,
		const struct usb_ctrlrequest *ctrl)
{
	struct usb_request	*req = fsg->ep0req;
	int			value = -EOPNOTSUPP;
	u16			w_index; 
	u16                w_value;
	u16			w_length; 

	w_index = le16_to_cpu(ctrl->wIndex);
	w_value = le16_to_cpu(ctrl->wValue);
	w_length = le16_to_cpu(ctrl->wLength);

	fsg_info("mass-storage class-specific control req "
			"%02x.%02x v%04x i%04x l%u\n",
			ctrl->bRequestType, ctrl->bRequest,
			le16_to_cpu(ctrl->wValue), w_index, w_length);
	
	if (!fsg->config)
		return value;
	/* Handle Bulk-only class-specific requests */
	switch (ctrl->bRequest) {
		case USB_BULK_RESET_REQUEST:
			if (ctrl->bRequestType != (USB_DIR_OUT |
					USB_TYPE_CLASS | USB_RECIP_INTERFACE))
				break;
			if (w_index != 0 || w_value != 0) {
				value = -EDOM;
				break;
			}

			/* Raise an exception to stop the current operation
			 * and reinitialize our state. */
			fsg_dbg( "bulk reset request\n");
			raise_exception(fsg, FSG_STATE_RESET);
			value = DELAYED_STATUS;
			break;

		case USB_BULK_GET_MAX_LUN_REQUEST:
			if (ctrl->bRequestType != (USB_DIR_IN |
					USB_TYPE_CLASS | USB_RECIP_INTERFACE))
				break;
			if (w_index != 0 || w_value != 0) {
				value = -EDOM;
				break;
			}
			
			fsg_dbg("get max LUN\n");
			*(u8 *) req->buf = fsg->nluns - 1;
			value = 1;
			break;
	}

	if (value == -EOPNOTSUPP)
		fsg_dbg("unknown class-specific control req "
			"%02x.%02x v%04x i%04x l%u\n",
			ctrl->bRequestType, ctrl->bRequest,
			le16_to_cpu(ctrl->wValue), w_index, w_length);
	return value;
}


/*-------------------------------------------------------------------------*/

/* Ep0 standard request handlers.  These always run in_irq. */

static int standard_setup_req(struct fsg_dev *fsg,
		const struct usb_ctrlrequest *ctrl)
{
	struct usb_request	*req = fsg->ep0req;
	int			value = -EOPNOTSUPP;
	u16			w_index = le16_to_cpu(ctrl->wIndex);
	u16			w_value = le16_to_cpu(ctrl->wValue);
	
	/* Usually this just stores reply data in the pre-allocated ep0 buffer,
	 * but config change events will also reconfigure hardware. */
	switch (ctrl->bRequest) {

	case USB_REQ_GET_DESCRIPTOR:
		if (ctrl->bRequestType != (USB_DIR_IN | USB_TYPE_STANDARD |
				USB_RECIP_DEVICE))
			break;
		switch (w_value >> 8) {

		case USB_DT_DEVICE:
			fsg_dbg( "get device descriptor\n");
			value = sizeof device_desc;
			memcpy(req->buf, &device_desc, value);
			break;
		case USB_DT_DEVICE_QUALIFIER:
			fsg_dbg( "get device qualifier\n");			
#if 0
			if (!gadget_is_dualspeed(fsg->gadget))
				break;
#endif			
			value = sizeof dev_qualifier;
			memcpy(req->buf, &dev_qualifier, value);
			break;

		case USB_DT_OTHER_SPEED_CONFIG:
			fsg_dbg( "get other-speed config descriptor\n");
#if 0			
			if (!gadget_is_dualspeed(fsg->gadget))
				break;
#endif			
			goto get_config;
		case USB_DT_CONFIG:
			fsg_dbg( "get configuration descriptor\n");
get_config:
			value = populate_config_buf(fsg->gadget,
					req->buf,
					w_value >> 8,
					w_value & 0xff);
			break;

		case USB_DT_STRING:
			fsg_dbg("get string descriptor\n");

			/* wIndex == language code */
			value = usb_gadget_get_string(&stringtab,
					w_value & 0xff, req->buf);
			break;
		}
		break;
	/* One config, two speeds */
	case USB_REQ_SET_CONFIGURATION:
		if (ctrl->bRequestType != (USB_DIR_OUT | USB_TYPE_STANDARD |
				USB_RECIP_DEVICE))
			break;
		fsg_dbg("set configuration\n");
		if (w_value == CONFIG_VALUE || w_value == 0) {
			fsg->new_config = w_value;

			/* Raise an exception to wipe out previous transaction
			 * state (queued bufs, etc) and set the new config. */
			raise_exception(fsg, FSG_STATE_CONFIG_CHANGE);
			value = DELAYED_STATUS;
		}
		break;
	case USB_REQ_GET_CONFIGURATION:
		if (ctrl->bRequestType != (USB_DIR_IN | USB_TYPE_STANDARD |
				USB_RECIP_DEVICE))
			break;
		fsg_dbg( "get configuration\n");
		*(u8 *) req->buf = fsg->config;
		value = 1;
		break;

	case USB_REQ_SET_INTERFACE:
		if (ctrl->bRequestType != (USB_DIR_OUT| USB_TYPE_STANDARD |
				USB_RECIP_INTERFACE))
			break;
		if (fsg->config && w_index == 0) {

			/* Raise an exception to wipe out previous transaction
			 * state (queued bufs, etc) and install the new
			 * interface altsetting. */
			raise_exception(fsg, FSG_STATE_INTERFACE_CHANGE);
			value = DELAYED_STATUS;
		}
		break;
	case USB_REQ_GET_INTERFACE:
		if (ctrl->bRequestType != (USB_DIR_IN | USB_TYPE_STANDARD |
				USB_RECIP_INTERFACE))
			break;
		if (!fsg->config)
			break;
		if (w_index != 0) {
			value = -EDOM;
			break;
		}
		fsg_dbg( "get interface\n");
		*(u8 *) req->buf = 0;
		value = 1;
		break;

	default:
		fsg_dbg("unknown control req %02x.%02x v%04x i%04x l%u\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, le16_to_cpu(ctrl->wLength));
	}

	return value;
}



static int fsg_setup(struct usb_gadget *gadget,
		const struct usb_ctrlrequest *ctrl)
{
	struct fsg_dev		*fsg = get_gadget_data(gadget);
	int			rc;
	int			w_length = le16_to_cpu(ctrl->wLength);

	++fsg->ep0_req_tag;		// Record arrival of a new request
	fsg->ep0req->context = NULL;
	fsg->ep0req->length = 0;
	dump_msg(fsg, "ep0-setup", (u8 *) ctrl, sizeof(*ctrl));

	if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS)
		rc = class_setup_req(fsg, ctrl);
	else
		rc = standard_setup_req(fsg, ctrl);

	/* Respond with data/status or defer until later? */
	if (rc >= 0 && rc != DELAYED_STATUS) {
		rc = min(rc, w_length);
		fsg->ep0req->length = rc;
		fsg->ep0req->zero = rc < w_length;
		fsg->ep0req_name = (ctrl->bRequestType & USB_DIR_IN ?
				"ep0-in" : "ep0-out");
		rc = ep0_queue(fsg);
	}

	/* Device either stalls (rc < 0) or reports success */
	return rc;
}

/*-------------------------------------------------------------------------*/

/* All the following routines run in process context */


/* Use this for bulk or interrupt transfers, not ep0 */
 void start_transfer(struct fsg_dev *fsg, struct usb_ep *ep,
		struct usb_request *req, int *pbusy,
		enum fsg_buffer_state *state)
{
	int	rc;
	if (ep == fsg->bulk_in)
		dump_msg(fsg, "bulk-in", req->buf, req->length);
	
	spin_lock_irq(&fsg->lock);
	*pbusy = 1;
	*state = BUF_STATE_BUSY;
	spin_unlock_irq(&fsg->lock);
	
	rc = usb_ep_queue(ep, req, GFP_KERNEL);
	if (rc != 0) {
		*pbusy = 0;
		*state = BUF_STATE_EMPTY;

		/* We can't do much more than wait for a reset */

		/* Note: currently the net2280 driver fails zero-length
		 * submissions if DMA is enabled. */
		if (	rc != -ESHUTDOWN 
			&& !(rc == -EOPNOTSUPP && req->length == 0))
			printk( "error in submission: %s --> %d\n",ep->name, rc);
	}
	
}


int sleep_thread(struct fsg_dev *fsg)
{
	int	rc = 0;

	/* Wait until a signal arrives or we are woken up */
	for (;;) {
		try_to_freeze();
		set_current_state(TASK_INTERRUPTIBLE);
		if (signal_pending(current)) {
			rc = -EINTR;
			break;
		}
		if (fsg->thread_wakeup_needed)
			break;
		schedule();
	}
	__set_current_state(TASK_RUNNING);
	fsg->thread_wakeup_needed = 0;
	return rc;
}

/*-------------------------------------------------------------------------*/
static int do_read(struct fsg_dev *fsg)
{
	struct lun		*curlun = fsg->curlun;
	u32			lba;
	struct fsg_buffhd	*bh;
	int			rc;
	u32			amount_left;
	loff_t			file_offset, file_offset_tmp;
	unsigned int		amount;
	ssize_t			nread;
	
	//unsigned int		partial_page;
	
	/* Get the starting Logical Block Address and check that it's
	 * not too big */
	// printk("%s:%d\n",__FUNCTION__,__LINE__);
	if (fsg->cmnd[0] == SC_READ_6)
		lba = (fsg->cmnd[1] << 16) | get_be16(&fsg->cmnd[2]);
	else {
		lba = get_be32(&fsg->cmnd[2]);

		/* We allow DPO (Disable Page Out = don't save data in the
		 * cache) and FUA (Force Unit Access = don't read from the
		 * cache), but we don't implement them. */
		if ((fsg->cmnd[1] & ~0x18) != 0) {
			curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
			return -EINVAL;
		}
	}
	
	if (lba >= curlun->num_sectors) {
		curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		return -EINVAL;
	}

	file_offset = ((loff_t) lba) << 9;
	/* Carry out the file reads */
	amount_left = fsg->data_size_from_cmnd;
	if (unlikely(amount_left == 0))
		return -EIO;		// No default reply
	for (;;) {
		/* Figure out how much we need to read:
		 * Try to read the remaining amount.
		 * But don't read more than the buffer size.
		 * And don't try to read past the end of the file.
		 * Finally, if we're not at a page boundary, don't read past
		 *	the next page.
		 * If this means reading 0 then we were asked to read past
		 *	the end of file. */
		amount = min((unsigned int) amount_left, (u32)(PAGE_SIZE<<mod_data.buflen));
		amount = min((loff_t) amount,curlun->file_length - file_offset);
	
		//page aligned
		/*
		partial_page = file_offset & (PAGE_CACHE_SIZE - 1);
		if(partial_page > 0)
			amount = min(amount, (unsigned int) PAGE_CACHE_SIZE - partial_page);
		*/
		
		/* Wait for the next buffer to become available */
		bh = fsg->next_buffhd_to_fill;
		while (bh->state != BUF_STATE_EMPTY) {
			rc = sleep_thread(fsg);
			if (rc)
				return rc;
		}
		
		/* If we were asked to read past the end of file,
		 * end with an empty buffer. */
		if (amount == 0){
			curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
			curlun->sense_data_info = file_offset >> 9;
			curlun->info_valid = 1;
			bh->inreq->length = 0;
			bh->state = BUF_STATE_FULL;
			break;
		}
		
		/* Perform the read */
		file_offset_tmp = file_offset;	
		if( curlun->use_media_ops){
			/*media read private intf*/
			rc = curlun->media_ops->media_read(file_offset >> 9,
										     bh->buf,
										     amount>>9,
										     curlun->lun_name);
			if(!rc){
				nread = amount;
				fsg_dbg( "%s: read %u @ %llu -> %d\n", 
					curlun->lun_name,
					amount,
					(unsigned long long) file_offset,
					(int) nread);
			}else{
				fsg_err("read error:%d\n",rc);
				return rc;
			}			
		}else{	
			nread = vfs_read(curlun->filp,
					/*(char __user *)*/ bh->buf,
					amount, &file_offset_tmp);

			fsg_dbg( "file read %u @ %llu -> %d\n", amount,
					(unsigned long long) file_offset,
					(int) nread);
		}
		
		if (signal_pending(current))
			return -EINTR;
		
		if (nread < 0){
			fsg_err("error in file read: %d\n",
					(int) nread);
			nread = 0;
		} else if (nread < amount){
			fsg_dbg( "partial file read: %d/%u\n",
					(int) nread, amount);
			nread -= (nread & 511);	// Round down to a block
		}
		
		file_offset  += nread;
		amount_left  -= nread;
		fsg->residue -= nread;
		bh->inreq->length = nread;
		bh->state = BUF_STATE_FULL;

		/* If an error occurred, report it and its position */
		if (nread < amount) {
			curlun->sense_data = SS_UNRECOVERED_READ_ERROR;
			curlun->sense_data_info = file_offset >> 9;
			curlun->info_valid = 1;
			break;
		}

		if (amount_left == 0)
			break;		// No more left to read

		/* Send this buffer and go read some more */
		bh->inreq->zero = 0;
		start_transfer(fsg, fsg->bulk_in, bh->inreq,
				&bh->inreq_busy, &bh->state);
		fsg->next_buffhd_to_fill = bh->next;
	}
	// No default reply
	return -EIO;		
}


/*-------------------------------------------------------------------------*/
static int do_write(struct fsg_dev *fsg)
{
	struct lun		*curlun = fsg->curlun;
	u32			lba;
	struct fsg_buffhd	*bh;
	int			get_some_more;
	u32			amount_left_to_req, amount_left_to_write;
	loff_t			usb_offset, file_offset, file_offset_tmp;
	unsigned int		amount;
	ssize_t			nwritten;
	int			rc;
	//unsigned int		partial_page;

	if (curlun->ro) {
		curlun->sense_data = SS_WRITE_PROTECTED;
		return -EINVAL;
	}
	
	curlun->filp->f_flags &= ~O_SYNC;	// Default is not to wait

	/* Get the starting Logical Block Address and check that it's
	 * not too big */
	if (fsg->cmnd[0] == SC_WRITE_6)
		lba = (fsg->cmnd[1] << 16) | get_be16(&fsg->cmnd[2]);
	else {
		lba = get_be32(&fsg->cmnd[2]);

		/* We allow DPO (Disable Page Out = don't save data in the
		 * cache) and FUA (Force Unit Access = write directly to the
		 * medium).  We don't implement DPO; we implement FUA by
		 * performing synchronous output. */
		if ((fsg->cmnd[1] & ~0x18) != 0) {
			curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
			return -EINVAL;
		}
		if (fsg->cmnd[1] & 0x08)	// FUA
			curlun->filp->f_flags |= O_SYNC;
	}
	
	if (lba >= curlun->num_sectors) {
		curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		return -EINVAL;
	}

	/*flag for is written*/
	if(!curlun->is_written)
		curlun->is_written = 1;

	/* Carry out the file writes */
	get_some_more = 1;
	file_offset = usb_offset = ((loff_t) lba) << 9;
	amount_left_to_req = amount_left_to_write = fsg->data_size_from_cmnd;
	
	while (amount_left_to_write > 0){
		/* Queue a request for more data from the host */
		bh = fsg->next_buffhd_to_fill;
		if (bh->state == BUF_STATE_EMPTY && get_some_more) {
			/* Figure out how much we want to get:
			 * Try to get the remaining amount.
			 * But don't get more than the buffer size.
			 * And don't try to go past the end of the file.
			 * If we're not at a page boundary,
			 *	don't go past the next page.
			 * If this means getting 0, then we were asked
			 *	to write past the end of file.
			 * Finally, round down to a block boundary. */
			 
			amount = min(amount_left_to_req,(u32)(PAGE_SIZE<<mod_data.buflen));
			amount = min((loff_t) amount, curlun->file_length - usb_offset);
			/*
			partial_page = usb_offset & (PAGE_CACHE_SIZE - 1);
			if (partial_page > 0)
				amount = min(amount,
						(unsigned int) PAGE_CACHE_SIZE - partial_page);
			*/
			if (amount == 0) {
				get_some_more = 0;
				curlun->sense_data =
					SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
				curlun->sense_data_info = usb_offset >> 9;
				curlun->info_valid = 1;
				continue;
			}
			
			amount -= (amount & 511);
			if (amount == 0) {
				/* Why were we were asked to transfer a
				 * partial block? */
				get_some_more = 0;
				continue;
			}

			/* Get the next buffer */
			usb_offset += amount;
			fsg->usb_amount_left -= amount;
			amount_left_to_req -= amount;
			if (amount_left_to_req == 0)
				get_some_more = 0;

			/* amount is always divisible by 512, hence by
			 * the bulk-out maxpacket size */
			bh->outreq->length = bh->bulk_out_intended_length =
					amount;
			bh->outreq->short_not_ok = 1;
			start_transfer(fsg, fsg->bulk_out, bh->outreq,
					&bh->outreq_busy, &bh->state);
			fsg->next_buffhd_to_fill = bh->next;
			continue;
		}

		/* Write the received data to the backing file */
		bh = fsg->next_buffhd_to_drain;
		if (bh->state == BUF_STATE_EMPTY && !get_some_more)
			break;			// We stopped early
			
		if (bh->state == BUF_STATE_FULL) {
			smp_rmb();
			fsg->next_buffhd_to_drain = bh->next;
			bh->state = BUF_STATE_EMPTY;
			
			/* Did something go wrong with the transfer? */
			if (bh->outreq->status != 0) {
				curlun->sense_data = SS_COMMUNICATION_FAILURE;
				curlun->sense_data_info = file_offset >> 9;
				curlun->info_valid = 1;
				break;
			}

			amount = bh->outreq->actual;
			if (curlun->file_length - file_offset < amount) 
				amount = curlun->file_length - file_offset;
		
			/* Perform the write */
			file_offset_tmp = file_offset;
			if(curlun->use_media_ops) {
				rc = curlun->media_ops->media_write(file_offset_tmp>>9,
								bh->buf,
								amount>>9,
								curlun->lun_name);
				if(!rc){	
					nwritten = amount;
					fsg_dbg( "%s: write %u @ %llu -> %d\n", 
						curlun->lun_name,
						amount,
						(unsigned long long) file_offset,
						(int) nwritten);
				}
				else{
					fsg_err("write failed,rc:%d\n",rc);
					return rc;
				}		
			}else{
				nwritten = vfs_write(curlun->filp,
						/*(char __user *)*/ bh->buf,
						amount, &file_offset_tmp);
				
				fsg_dbg("file write %u @ %llu -> %d\n", amount,
						(unsigned long long) file_offset,
						(int) nwritten);
			}

			if (signal_pending(current))
				return -EINTR;		// Interrupted!

			if (nwritten < 0) {
				fsg_dbg( "error in file write: %d\n",
						(int) nwritten);
				nwritten = 0;
			} else if (nwritten < amount) {
				fsg_dbg( "partial file write: %d/%u\n",
						(int) nwritten, amount);
				nwritten -= (nwritten & 511);
						// Round down to a block
			}
			
			file_offset += nwritten;
			amount_left_to_write -= nwritten;
			fsg->residue -= nwritten;

			/* If an error occurred, report it and its position */
			if (nwritten < amount) {
				curlun->sense_data = SS_WRITE_ERROR;
				curlun->sense_data_info = file_offset >> 9;
				curlun->info_valid = 1;
				break;
			}

			/* Did the host decide to stop early? */
			if (bh->outreq->actual != bh->outreq->length) {
				fsg->short_packet_received = 1;
				break;
			}
			continue;
		}

		/* Wait for something to happen */
		rc = sleep_thread(fsg);
		if (rc)
			return rc;
	}

	return -EIO;		// No default reply
}


/*-------------------------------------------------------------------------*/

/* Sync the file data, don't bother with the metadata.
 * This code was copied from fs/buffer.c:sys_fdatasync(). */
static int fsync_sub(struct lun *curlun)
{
	struct file	*filp = curlun->filp;
	struct inode	*inode;
	int		rc, err;

	if (curlun->ro || !filp)
		return 0;
	
	if (!filp->f_op->fsync)
		return -EINVAL;

	inode = filp->f_path.dentry->d_inode;
	mutex_lock(&inode->i_mutex);
	rc = filemap_fdatawrite(inode->i_mapping);
	err = filp->f_op->fsync(filp, filp->f_path.dentry, 1);
	if (!rc)
		rc = err;
	err = filemap_fdatawait(inode->i_mapping);
	if (!rc)
		rc = err;
	mutex_unlock(&inode->i_mutex);
	fsg_dbg("fdatasync -> %d\n", rc);
	return rc;
}

static void fsync_all(struct fsg_dev *fsg)
{
	int	i;

	for (i = 0; i < fsg->nluns; ++i)
		fsync_sub(&fsg->luns[i]);
}



static int do_synchronize_cache(struct fsg_dev *fsg)
{
	struct lun	*curlun = fsg->curlun;
	int		rc;

	/* We ignore the requested LBA and write out all file's
	 * dirty data buffers. */
	rc = fsync_sub(curlun);
	if (rc)
		curlun->sense_data = SS_WRITE_ERROR;
	return 0;
}


/*-------------------------------------------------------------------------*/

static void invalidate_sub(struct lun *curlun)
{
	struct file	*filp = curlun->filp;
	struct inode	*inode = filp->f_path.dentry->d_inode;
	unsigned long	rc;

	rc = invalidate_mapping_pages(inode->i_mapping, 0, -1);
	fsg_dbg("invalidate_inode_pages -> %ld\n", rc);
}


static int do_verify(struct fsg_dev *fsg)
{
	struct lun		*curlun = fsg->curlun;
	u32			lba;
	u32			verification_length;
	struct fsg_buffhd	*bh = fsg->next_buffhd_to_fill;
	loff_t			file_offset, file_offset_tmp;
	u32			amount_left;
	unsigned int		amount;
	ssize_t			nread;

	/* Get the starting Logical Block Address and check that it's
	 * not too big */
	lba = get_be32(&fsg->cmnd[2]);
	if (lba >= curlun->num_sectors) {
		curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		return -EINVAL;
	}

	/* We allow DPO (Disable Page Out = don't save data in the
	 * cache) but we don't implement it. */
	if ((fsg->cmnd[1] & ~0x10) != 0) {
		curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
		return -EINVAL;
	}

	verification_length = get_be16(&fsg->cmnd[7]);
	if (unlikely(verification_length == 0))
		return -EIO;		// No default reply

	/* Prepare to carry out the file verify */
	amount_left = verification_length << 9;
	file_offset = ((loff_t) lba) << 9;

	/* Write out all the dirty buffers before invalidating them */
	fsync_sub(curlun);
	if (signal_pending(current))
		return -EINTR;

	invalidate_sub(curlun);
	if (signal_pending(current))
		return -EINTR;

	/* Just try to read the requested blocks */
	while (amount_left > 0) {

		/* Figure out how much we need to read:
		 * Try to read the remaining amount, but not more than
		 * the buffer size.
		 * And don't try to read past the end of the file.
		 * If this means reading 0 then we were asked to read
		 * past the end of file. */
		amount = min((unsigned int) amount_left, (u32)(PAGE_SIZE<<mod_data.buflen));
		amount = min((loff_t) amount,
				curlun->file_length - file_offset);
		if (amount == 0) {
			curlun->sense_data =
					SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
			curlun->sense_data_info = file_offset >> 9;
			curlun->info_valid = 1;
			break;
		}

		/* Perform the read */
		file_offset_tmp = file_offset;
		nread = vfs_read(curlun->filp,
				(char __user *) bh->buf,
				amount, &file_offset_tmp);
		fsg_dbg( "file read %u @ %llu -> %d\n", amount,
				(unsigned long long) file_offset,
				(int) nread);
		if (signal_pending(current))
			return -EINTR;

		if (nread < 0) {
			fsg_dbg( "error in file verify: %d\n",
					(int) nread);
			nread = 0;
		} else if (nread < amount) {
			fsg_dbg( "partial file verify: %d/%u\n",
					(int) nread, amount);
			nread -= (nread & 511);	// Round down to a sector
		}
		if (nread == 0) {
			curlun->sense_data = SS_UNRECOVERED_READ_ERROR;
			curlun->sense_data_info = file_offset >> 9;
			curlun->info_valid = 1;
			break;
		}
		file_offset += nread;
		amount_left -= nread;
	}
	return 0;
}


/*-------------------------------------------------------------------------*/
/*
  *the function :process inquiry scsi command
  */
static int do_inquiry(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	u8	*buf = (u8 *) bh->buf;
	struct lun * curlun = fsg->curlun;

	
	if (!curlun) {		// Unsupported LUNs are okay
		fsg->bad_lun_okay = 1;
		memset(buf, 0, 36);
		buf[0] = 0x7f;		// Unsupported, no device-type
		return 36;
	}

	memset(buf, 0, 8);	// Non-removable, direct-access device

	//buf[0] = (mod_data.cdrom ? TYPE_CDROM : TYPE_DISK);//inform host as disk or cdrom
	if(mod_data.cdrom==0)
		buf[0]=TYPE_DISK;
	else if(mod_data.cdrom==1){
		if(curlun->cdrom==1){
			buf[0]=TYPE_CDROM;   //bq:response data to host 
		}else {
			buf[0]=TYPE_DISK;
		}
	}

	if (mod_data.removable)
		buf[1] = 0x80;
	
	buf[2] = 1;		// ANSI SCSI level 2
	buf[3] = 2;		// SCSI-2 INQUIRY data format
	buf[4] = 31;		// Additional length
	// No special options
	sprintf(buf + 8, "%-8s%-16s%04x", mod_data.inquiry_vendor ,curlun->inquiry_name, mod_data.release);
	return 36;
}


static int do_request_sense(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	struct lun	*curlun = fsg->curlun;
	u8		*buf = (u8 *) bh->buf;
	u32		sd, sdinfo;
	int		valid;

	/*
	 * From the SCSI-2 spec., section 7.9 (Unit attention condition):
	 *
	 * If a REQUEST SENSE command is received from an initiator
	 * with a pending unit attention condition (before the target
	 * generates the contingent allegiance condition), then the
	 * target shall either:
	 *   a) report any pending sense data and preserve the unit
	 *	attention condition on the logical unit, or,
	 *   b) report the unit attention condition, may discard any
	 *	pending sense data, and clear the unit attention
	 *	condition on the logical unit for that initiator.
	 *
	 * FSG normally uses option a); enable this code to use option b).
	 */
#if 0
	if (curlun && curlun->unit_attention_data != SS_NO_SENSE) {
		curlun->sense_data = curlun->unit_attention_data;
		curlun->unit_attention_data = SS_NO_SENSE;
	}
#endif

	if (!curlun) {		// Unsupported LUNs are okay
		fsg->bad_lun_okay = 1;
		sd = SS_LOGICAL_UNIT_NOT_SUPPORTED;
		sdinfo = 0;
		valid = 0;
	} else {
		sd = curlun->sense_data;
		sdinfo = curlun->sense_data_info;
		valid = curlun->info_valid << 7;
		curlun->sense_data = SS_NO_SENSE;
		curlun->sense_data_info = 0;
		curlun->info_valid = 0;
	}

	memset(buf, 0, 18);
	buf[0] = valid | 0x70;			// Valid, current error
	buf[2] = SK(sd);
	put_be32(&buf[3], sdinfo);		// Sense information
	buf[7] = 18 - 8;			// Additional sense length
	buf[12] = ASC(sd);
	buf[13] = ASCQ(sd);
	return 18;
}


static int do_read_capacity(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	struct lun	*curlun = fsg->curlun;
	u32		lba = get_be32(&fsg->cmnd[2]);
	int		pmi = fsg->cmnd[8];
	u8		*buf = (u8 *) bh->buf;

	/* Check the PMI and LBA fields */
	if (pmi > 1 || (pmi == 0 && lba != 0)) {
		curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
		return -EINVAL;
	}
	
	put_be32(&buf[0], curlun->num_sectors - 1);	// Max logical block
	put_be32(&buf[4], 512);				// Block length
	return 8;
}
static void store_cdrom_address(u8 *dest, int msf, u32 addr)
{
	if (msf) {
		/* Convert to Minutes-Seconds-Frames */
		addr >>= 2;		/* Convert to 2048-byte frames */
		addr += 2*75;		/* Lead-in occupies 2 seconds */
		dest[3] = addr % 75;	/* Frames */
		addr /= 75;
		dest[2] = addr % 60;	/* Seconds */
		addr /= 60;
		dest[1] = addr;		/* Minutes */
		dest[0] = 0;		/* Reserved */
	} else {
		/* Absolute sector */
		put_unaligned_be32(addr, dest);
	}
}

static int do_read_header(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	struct lun	*curlun = fsg->curlun;
	int		msf = fsg->cmnd[1] & 0x02;
	u32		lba = get_unaligned_be32(&fsg->cmnd[2]);
	u8		*buf = (u8 *) bh->buf;

	if ((fsg->cmnd[1] & ~0x02) != 0) {		/* Mask away MSF */
		curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
		return -EINVAL;
	}
	if (lba >= curlun->num_sectors) {
		curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		return -EINVAL;
	}

	memset(buf, 0, 8);
	buf[0] = 0x01;		/* 2048 bytes of user data, rest is EC */
	store_cdrom_address(&buf[4], msf, lba);
	return 8;
}

/*
  *funcation:identify windows with mac process read toc command
  *1,windows process it with formatted toc
  *2,mac os process it with raw toc
  */
static int fsg_get_toc(struct lun *curlun, int msf, int format, u8*buf)
{
	int len;
	
	switch(format){
	case 0:
		/*formatted toc*/
		len=4+2*8;
		memset(buf,0,len);
		buf[1] =len -2;  /*toc length excludes length field*/
		buf[2] = 1;         
		buf[3] = 1;
		buf[5] = 0x16;
		buf[6] =0x01;

		store_cdrom_address(&buf[8],msf,0);
		#if 0
		for(j=0;j<4;j++)
			printk("debug---buf[8+%d]=%d\n",j,buf[8+j]);
		#endif
		buf[13] =0x16;
		buf[14] = 0xaa; //track number

		store_cdrom_address(&buf[16], msf, curlun->num_sectors);
		#if 0
		for(j=0;j<4;j++)
			printk("debug---buf[16+%d]=%d\n",j,buf[8+j]);
		#endif
		break;
	
	case 2:
		/* Raw TOC */
		len = 37; /* 4 byte header + 3 descriptors */
		memset(buf, 0, len); /* Header + A0, A1 & A2 descriptors */
		buf[1] = len; /* TOC Length excludes length field */
		buf[2] = 1; /* First complete session */
		buf[3] = 2; /* Last complete session */

		buf += 4;
		/* A0 points */
		buf[0] = 1; /* Session number */
		buf[1] = 0x16; /* Data track, copying allowed */
		/* 2 - Track number 0 -> TOC */
		buf[3] = 0xA0; /* A0point */
		/* 4, 5, 6 - Min, sec, frame is zero */
		/*7 -zero*/
		buf[8] = 1; /* Pmin: last track number */
		buf[9] = 0;
		/* 9 - disc type 0: CD-ROM/DA with 1st track in mode 1 */
		 /* 10 - pframe 0 */
		buf += 11; /* go to next track descriptor */

		/*A1 point*/
		buf[0] = 1;
		buf[1] = 0x16;
		buf[3] = 0xa1;
		buf[8] = 1;
		
		buf += 11; /* go back to A2 descriptor */
	#if 1
		/*A2 point*/
		buf[0] = 1;
		buf[1] = 0x16;
		buf[3] = 0xa2;

	#else
		/*A2 point */
		buf[0] = 1;
		buf[1] = 0x16;
		buf[3] = 0xa2;
		buf[8] = 0x03;
		buf[9] = 0x14;
		buf[10] = 0x34;

		buf += 11;
		/*A3 point*/
		buf[0] = 1;
		buf[1] = 0x14;
		buf[3] = 1;
	#endif
		/* For A2, 7, 8, 9, 10 - zero, Pmin, Psec, Pframe of Lead out */
		store_cdrom_address(&buf[7], msf, curlun->num_sectors);
	#if 0
		for(j=0;j<4;j++)
			printk("debug---buf[7+%d]=%d\n",j,buf[7+j]);
	#endif
		//return len;
		break;
	default:
		/* Multi-session, PMA, ATIP, CD-TEXT not supported/required */
		return -EINVAL;
		break;

	}
	return len;
}

/***/

#if 0
static int do_read_toc(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	struct lun	*curlun = fsg->curlun;
	int		msf = fsg->cmnd[1] & 0x02;
	int		start_track = fsg->cmnd[6];
	u8		*buf = (u8 *) bh->buf;

	if ((fsg->cmnd[1] & ~0x02) != 0 ||		/* Mask away MSF */
			start_track > 1) {
		curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
		return -EINVAL;
	}

	memset(buf, 0, 20);
	buf[1] = (20-2);		/* TOC data length */
	buf[2] = 1;			/* First track number */
	buf[3] = 1;			/* Last track number */
	buf[5] = 0x16;			/* Data track, copying allowed */
	buf[6] = 0x01;			/* Only track is number 1 */
	store_cdrom_address(&buf[8], msf, 0);

	buf[13] = 0x16;			/* Lead-out track is data */
	buf[14] = 0xAA;			/* Lead-out track number */
	store_cdrom_address(&buf[16], msf, curlun->num_sectors);
	return 20;
}

#else
static int do_read_toc(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	struct lun	*curlun = fsg->curlun;
	int		msf = fsg->cmnd[1] & 0x02;
	int		start_track = fsg->cmnd[6];
	u8		*buf = (u8 *) bh->buf;
	u8 format;
	int ret;

	if ((fsg->cmnd[1] & ~0x02) != 0 ||		/* Mask away MSF */
			start_track > 1) {
		curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
		return -EINVAL;
	}

	format =fsg->cmnd[2]&0xf;
	/*
	  *check if cdb is old style sff-8020i 
	  *i.e.format is in 2 msbs of byte 9 .mac os-x host sends us this.
	  */
	//printk("debug---%s %d format:%d\n",__FILE__,__LINE__,format);
	if(format == 0)
		format = (fsg->cmnd[9]>>6)&0x3;

	//printk("debug---%s %d format:%d\n",__FILE__,__LINE__,format);
	ret = fsg_get_toc(curlun,msf,format,buf);
	if (ret < 0) {
		curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
		 return -EINVAL;
	 } else if (ret > fsg->data_size_from_cmnd) {
		ret = fsg->data_size_from_cmnd;
	} else {
		fsg->residue = ret;
	}
	return ret;	
}
#endif


static int do_mode_sense(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	struct lun	*curlun = fsg->curlun;
	int 	mscmnd = fsg->cmnd[0];
	u8		*buf = (u8 *) bh->buf;
	u8		*buf0 = buf;
	int 	pc, page_code;
	int 	changeable_values, all_pages;
	int 	valid_page = 0;
	int 	len, limit;

	if ((fsg->cmnd[1] & ~0x08) != 0) {		// Mask away DBD
		curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
		return -EINVAL;
	}
	
	pc = fsg->cmnd[2] >> 6;
	page_code = fsg->cmnd[2] & 0x3f;
	if (pc == 3) {
		curlun->sense_data = SS_SAVING_PARAMETERS_NOT_SUPPORTED;
		return -EINVAL;
	}
	
	changeable_values = (pc == 1);
	all_pages = (page_code == 0x3f);

	/* Write the mode parameter header.  Fixed values are: default
	* medium type, no cache control (DPOFUA), and no block descriptors.
	* The only variable value is the WriteProtect bit.  We will fill in
	* the mode data length later. */
	memset(buf, 0, 8);
	if (mscmnd == SC_MODE_SENSE_6) {
		buf[2] = (curlun->ro ? 0x80 : 0x00);		// WP, DPOFUA
		buf += 4;
		limit = 255;
	} else {			// SC_MODE_SENSE_10
		buf[3] = (curlun->ro ? 0x80 : 0x00);		// WP, DPOFUA
		buf += 8;
		limit = 65535;		// Should really be mod_data.buflen
	}

	/* No block descriptors */

	/* The mode pages, in numerical order.	The only page we support
	* is the Caching page. */
	if (page_code == 0x08 || all_pages) {
		valid_page = 1;
		buf[0] = 0x08;		// Page code
		buf[1] = 10;		// Page length
		memset(buf+2, 0, 10);	// None of the fields are changeable

		if (!changeable_values) {
			buf[2] = 0x04;	// Write cache enable,
					// Read cache not disabled
					// No cache retention priorities
			put_be16(&buf[4], 0xffff);	// Don't disable prefetch
					// Minimum prefetch = 0
			put_be16(&buf[8], 0xffff);	// Maximum prefetch
			put_be16(&buf[10], 0xffff); // Maximum prefetch ceiling
		}
		buf += 12;
	}

	/* Check that a valid page was requested and the mode data length
	* isn't too long. */
	len = buf - buf0;
	if (!valid_page || len > limit) {
		curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
		return -EINVAL;
	}

	/*	Store the mode data length */
	if (mscmnd == SC_MODE_SENSE_6)
		buf0[0] = len - 1;
	else
		put_be16(buf0, len - 2);
	return len;	
}


static int do_start_stop(struct fsg_dev *fsg)
{
	struct lun	*curlun = fsg->curlun;
	int		loej, start;

	if (!mod_data.removable) {
		curlun->sense_data = SS_INVALID_COMMAND;
		return -EINVAL;
	}

	// int immed = fsg->cmnd[1] & 0x01;
	loej = fsg->cmnd[4] & 0x02;
	start = fsg->cmnd[4] & 0x01;

#ifdef CONFIG_USB_FILE_STORAGE_TEST
	if ((fsg->cmnd[1] & ~0x01) != 0 ||		// Mask away Immed
			(fsg->cmnd[4] & ~0x03) != 0) {	// Mask LoEj, Start
		curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
		return -EINVAL;
	}

	if (!start) {
		/* Are we allowed to unload the media? */
		if (curlun->prevent_medium_removal) {
			fsg_dbg( "unload attempt prevented\n");
			curlun->sense_data = SS_MEDIUM_REMOVAL_PREVENTED;
			return -EINVAL;
		}
		if (loej) {		// Simulate an unload/eject
			up_read(&fsg->filesem);
			down_write(&fsg->filesem);
			close_backing_file(curlun);
			up_write(&fsg->filesem);
			down_read(&fsg->filesem);
		}
	} else {

		/* Our emulation doesn't support mounting; the medium is
		 * available for use as soon as it is loaded. */
		if (!backing_file_is_open(curlun)) {
			curlun->sense_data = SS_MEDIUM_NOT_PRESENT;
			return -EINVAL;
		}
	}
#endif
	_remove_udisk_symbol();
	return 0;
}


static int do_prevent_allow(struct fsg_dev *fsg)
{
	struct lun	*curlun = fsg->curlun;
	int		prevent;

	if (!mod_data.removable) {
		curlun->sense_data = SS_INVALID_COMMAND;
		return -EINVAL;
	}

	/*
	  *the prevent bit ,Page137 in scsi spec
	  */
	prevent = fsg->cmnd[4] & 0x01;
	if ((fsg->cmnd[4] & ~0x01) != 0) {		// Mask away Prevent
		curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
		return -EINVAL;
	}

	if (curlun->prevent_medium_removal && !prevent)
		fsync_sub(curlun);
	curlun->prevent_medium_removal = prevent;
	//_remove_udisk_symbol();
	return 0;
}


static int do_read_format_capacities(struct fsg_dev *fsg,
			struct fsg_buffhd *bh)
{
	struct lun	*curlun = fsg->curlun;
	u8		*buf = (u8 *) bh->buf;

	buf[0] = buf[1] = buf[2] = 0;
	buf[3] = 8;		// Only the Current/Maximum Capacity Descriptor
	buf += 4;

	put_be32(&buf[0], curlun->num_sectors);		// Number of blocks
	put_be32(&buf[4], 512);						// Block length
	buf[4] = 0x02;								// Current capacity
	return 12;
}


static int do_mode_select(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	struct lun	*curlun = fsg->curlun;

	/* We don't support MODE SELECT */
	curlun->sense_data = SS_INVALID_COMMAND;
	return -EINVAL;
}
#if  1
/*
  *bq:add three scsi command for mac cd-rom 
  */
static int do_get_config(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	int rt,sfn,rp_data_len;
	u8		*buf = (u8 *) bh->buf;

	/*rt:request type*/
	rt = fsg->cmnd[1] &0x03;     
	/**sfn(startint feature number)**/
	sfn = get_be16(&fsg->cmnd[2] ); 
	/*rq_data_len:the get configuration response data*/
	rp_data_len = get_be16(&fsg->cmnd[7]);
	#if 0
	printk("debug---%s %d cmnd:%02x\n",__FUNCTION__,__LINE__,fsg->cmnd[0]);
	printk("debug---%s %d rt:%02x\n",__FILE__,__LINE__,rt);
	printk("debug---%s %d sfn:%02x\n",__FILE__,__LINE__,sfn);
	#endif
	/*the feature refer to MMC-2 scsi command special*/
	#if 1
	if(rt==0x00){
		u16 current_profile = 0x0008;
		u16 feature_code = 0x001e;
		put_be32(&buf[0], rp_data_len);		
		put_be16(&buf[6], current_profile);
		/*feature code is define to 0x001e,the meaning is can read 
		    cd specific informtion.
		*/
		put_be16(&buf[8], feature_code);
		buf[10]=0x03;
		buf[11] = 0x04;
		buf[12] = 0x03;
		
	}else if(rt==0x01){
		u16 current_profile = 0x0008;
		u16 feature_code = 0x001e;
		put_be32(&buf[0], rp_data_len);		
		put_be16(&buf[6], current_profile);
		/*feature code is define to 0x001e,the meaning is can read 
		    cd specific informtion.
		*/
		put_be16(&buf[8], feature_code);
		buf[10]=0x03;  //bq:beacause the current bit is must be set to 1;
		buf[11] = 0x04;
		buf[12] = 0x03;
		
	}else if(rt==0x02){
		u16 current_profile = 0x0008;
		u16 feature_code = 0x001e;
		put_be32(&buf[0], rp_data_len);		
		put_be16(&buf[6], current_profile);
		/*feature code is define to 0x001e,the meaning is can read 
		    cd specific informtion.
		*/
		put_be16(&buf[8], feature_code);
		buf[10]=0x05;  //bq:beacause the current bit is must be set to 1;
		buf[11] = 0x04;
		buf[12] = 0x03;
		;/*indicates that the feature header and only those feature descriptors,this may be used
		to request feature 0;*/

	}else	
	     ;//the is do nothing
	#endif	

	
	return 0;
}

static int do_set_speed(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	u16 lu_reads,lu_writes;
	struct lun	*curlun = fsg->curlun;
	u8		*buf = (u8 *) bh->buf;
	lu_reads =get_be16(&fsg->cmnd[2]);
	lu_writes =get_be16(&fsg->cmnd[4]);
#if 1
	printk("debug--- lu_reads=%02x\n",lu_reads);
	printk("debug--- lu_writes=%02x\n",lu_writes);
#endif

	
	if(!mod_data.cdrom){
		curlun->sense_data = SS_LOGICAL_UNIT_NOT_SUPPORTED;
		return -EINVAL;
	}
	
	if(lu_reads == 0xffff){  // the speed of read
		u16 nvls,cmrs,bss; //bss:buffer size supported
		buf[0] = 0xaa;
		buf[1] = 0x18;
		buf[2] = 0x3f;
		buf[4] = 0x7d;
		buf[5] = 0xfd;
		buf[6] = 0x88;
		buf[7] = 0x3f;
		nvls = 0x02;
		put_be16(&buf[10], nvls);
		bss= 0x00;
		put_be16(&buf[12], bss);
		buf[17] = 0x3f;
	        cmrs = 0x00;
		put_be16(&buf[22], cmrs);
	}
				
	return 0;
}

static int do_read_cd(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{

	int sector_type;
	int sub_channel;
	int header_code,user_data,edc,error_field;
	int transfer_len;
	int			rc;
	struct lun		*curlun = fsg->curlun;
	u32			lba;   //the start logic block address
	//u8		*buf = (u8 *) bh->buf;

	u32			amount_left;
	loff_t			file_offset, file_offset_tmp;
	unsigned int		amount;
	ssize_t			nread;
	
	sector_type = (fsg->cmnd[1]>>2)&0x07;  
	lba = get_be32(&fsg->cmnd[2]);
	transfer_len = (get_be16(&fsg->cmnd[6])<<8) | (fsg->cmnd[8]);
	sub_channel = fsg->cmnd[10]&0x07;
	header_code = (fsg->cmnd[9]>>5)&0x03;
	user_data = (fsg->cmnd[9]>>4) & 0x01;
	edc = (fsg->cmnd[9]>>3) & 0x01;
	error_field = (fsg->cmnd[9]>>1) & 0x03;
#if 0
	printk("debug---%s %d cmnd:%02x\n",__FUNCTION__,__LINE__,fsg->cmnd[0]);
	printk("debug--- sector_type=%02x\n",sector_type);
	printk("debug--- sub_channel=%02x\n",sub_channel);
	printk("debug--- header_code=%02x\n",header_code);
	printk("debug--- transfer_len=%d\n",transfer_len);
	printk("debug--- user_data=%02x\n",user_data);
	printk("debug--- edc=%02x\n",edc);
	printk("debug--- error_field=%02x\n",error_field);
#endif
	switch(sector_type){

		case 0x000:  //all type

			break;
		case 0x001: //cd-da type
		case 0x010:  //mode1
		case 0x011:  //mode2
		case 0x100: //mode2 from 1
		case 0x101: //mode2 from 2
		default:      //the unkown type
			;//the is do nothing 
			break;
	}

	if (lba >= curlun->num_sectors) {
		curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
		return -EINVAL;
	}

	file_offset = ((loff_t) lba) << 9;
	/* Carry out the file reads */
	amount_left = fsg->data_size_from_cmnd;
	
	if (amount_left == 0)
		return -EIO;		
		
	for (;;) {
		amount = min((unsigned int) amount_left, (u32)(PAGE_SIZE<<mod_data.buflen));
		amount = min((loff_t) amount,curlun->file_length - file_offset);
	
		
		/* Wait for the next buffer to become available */
		bh = fsg->next_buffhd_to_fill;
		while (bh->state != BUF_STATE_EMPTY) {
			rc = sleep_thread(fsg);
			if (rc)
				return rc;
		}
		
		/* If we were asked to read past the end of file,
		 * end with an empty buffer. */
		if (amount == 0){
			curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
			curlun->sense_data_info = file_offset >> 9;
			curlun->info_valid = 1;
			bh->inreq->length = 0;
			bh->state = BUF_STATE_FULL;
			break;
		}
		
		/* Perform the read */
		file_offset_tmp = file_offset;	
		if( curlun->use_media_ops){
			/*media read private intf*/
			rc = curlun->media_ops->media_read(file_offset >> 9,
										     bh->buf,
										     amount>>9,
										     curlun->lun_name);
			if(!rc){
				nread = amount;
				fsg_dbg( "%s: read %u @ %llu -> %d\n", 
					curlun->lun_name,
					amount,
					(unsigned long long) file_offset,
					(int) nread);
			}else{
				fsg_err("read error:%d\n",rc);
				return rc;
			}			
		}else{	
			nread = vfs_read(curlun->filp,
					bh->buf,
					amount, &file_offset_tmp);

			fsg_dbg( "file read %u @ %llu -> %d\n", amount,
					(unsigned long long) file_offset,
					(int) nread);
		}
		
		if (signal_pending(current))
			return -EINTR;
		
		if (nread < 0){
			fsg_err("error in file read: %d\n",
					(int) nread);
			nread = 0;
		} else if (nread < amount){
			fsg_dbg( "partial file read: %d/%u\n",
					(int) nread, amount);
			nread -= (nread & 511);	// Round down to a block
		}
		
		file_offset  += nread;
		amount_left  -= nread;
		fsg->residue -= nread;
		bh->inreq->length = nread;
		bh->state = BUF_STATE_FULL;

		/* If an error occurred, report it and its position */
		if (nread < amount) {
			curlun->sense_data = SS_UNRECOVERED_READ_ERROR;
			curlun->sense_data_info = file_offset >> 9;
			curlun->info_valid = 1;
			break;
		}

		if (amount_left == 0)
			break;		// No more left to read

		/* Send this buffer and go read some more */
		bh->inreq->zero = 0;
		start_transfer(fsg, fsg->bulk_in, bh->inreq,
				&bh->inreq_busy, &bh->state);
		fsg->next_buffhd_to_fill = bh->next;
	}
	// No default reply
/*	return -EIO;		
	buf[0] = 0x41;
	buf[1] = 0x00;
	buf[2] = 0xa0;
	//buf[3] = 0x00;
	buf[6] = 0x00;
	buf[7] = 0xa0;
	buf[8] = 0x00;
	buf[9] = 0x00;
	*/
	return 0;
}
#endif
/*-------------------------------------------------------------------------*/

static int halt_bulk_in_endpoint(struct fsg_dev *fsg)
{
	int	rc;

	rc = fsg_set_halt(fsg, fsg->bulk_in);
	if (rc == -EAGAIN)
		fsg_dbg( "delayed bulk-in endpoint halt\n");
	while (rc != 0) {
		if (rc != -EAGAIN) {
			fsg_dbg( "usb_ep_set_halt -> %d\n", rc);
			rc = 0;
			break;
		}

		/* Wait for a short time and then try again */
		if (msleep_interruptible(100) != 0)
			return -EINTR;
		rc = usb_ep_set_halt(fsg->bulk_in);
	}
	return rc;
}



static int wedge_bulk_in_endpoint(struct fsg_dev *fsg)
{
	int	rc;

	fsg_dbg( "bulk-in set wedge\n");
	rc = usb_ep_set_wedge(fsg->bulk_in);
	if (rc == -EAGAIN)
		fsg_dbg( "delayed bulk-in endpoint wedge\n");
	while (rc != 0) {
		if (rc != -EAGAIN) {
			fsg_warn( "usb_ep_set_wedge -> %d\n", rc);
			rc = 0;
			break;
		}

		/* Wait for a short time and then try again */
		if (msleep_interruptible(100) != 0)
			return -EINTR;
		rc = usb_ep_set_wedge(fsg->bulk_in);
	}
	return rc;
}


static int pad_with_zeros(struct fsg_dev *fsg)
{
	struct fsg_buffhd	*bh = fsg->next_buffhd_to_fill;
	u32			nkeep = bh->inreq->length;
	u32			nsend;
	int			rc;
	
	bh->state = BUF_STATE_EMPTY;		// For the first iteration
	fsg->usb_amount_left = nkeep + fsg->residue;
	while (fsg->usb_amount_left > 0) {
		/* Wait for the next buffer to be free */
		while (bh->state != BUF_STATE_EMPTY) {
			rc = sleep_thread(fsg);
			if (rc){
				return rc;
			}	
		}
		
		nsend = min(fsg->usb_amount_left, (u32) (PAGE_SIZE<<mod_data.buflen));
		memset(bh->buf + nkeep, 0, nsend - nkeep);
		bh->inreq->length = nsend;
		bh->inreq->zero = 0;
		
		start_transfer(fsg, fsg->bulk_in, bh->inreq,
				&bh->inreq_busy, &bh->state);
		bh = fsg->next_buffhd_to_fill = bh->next;
		fsg->usb_amount_left -= nsend;
		nkeep = 0;
	}
	return 0;	
}


static int throw_away_data(struct fsg_dev *fsg)
{
	struct fsg_buffhd	*bh;
	u32			amount;
	int			rc;

	while ((bh = fsg->next_buffhd_to_drain)->state != BUF_STATE_EMPTY ||
			fsg->usb_amount_left > 0) {

		/* Throw away the data in a filled buffer */
		if (bh->state == BUF_STATE_FULL) {
			smp_rmb();
			bh->state = BUF_STATE_EMPTY;
			fsg->next_buffhd_to_drain = bh->next;

			/* A short packet or an error ends everything */
			if (bh->outreq->actual != bh->outreq->length ||
					bh->outreq->status != 0) {
				raise_exception(fsg, FSG_STATE_ABORT_BULK_OUT);
				return -EINTR;
			}
			continue;
		}

		/* Try to submit another request if we need one */
		bh = fsg->next_buffhd_to_fill;
		if (bh->state == BUF_STATE_EMPTY && fsg->usb_amount_left > 0) {
			amount = min(fsg->usb_amount_left, (u32) (PAGE_SIZE<<mod_data.buflen));

			/* amount is always divisible by 512, hence by
			 * the bulk-out maxpacket size */
			bh->outreq->length = bh->bulk_out_intended_length =
					amount;
			bh->outreq->short_not_ok = 1;
			start_transfer(fsg, fsg->bulk_out, bh->outreq,
					&bh->outreq_busy, &bh->state);	
			fsg->next_buffhd_to_fill = bh->next;
			fsg->usb_amount_left -= amount;
			continue;
		}

		/* Otherwise wait for something to happen */
		rc = sleep_thread(fsg);
		if (rc)
			return rc;
	}
	return 0;
}


static int finish_reply(struct fsg_dev *fsg)
{
		struct fsg_buffhd	*bh = fsg->next_buffhd_to_fill;
		int 		rc = 0;
	
		switch (fsg->data_dir) {
		case DATA_DIR_NONE:
			break;			// Nothing to send
	
		/* If we don't know whether the host wants to read or write,
		 * this must be CB or CBI with an unknown command.	We mustn't
		 * try to send or receive any data.  So stall both bulk pipes
		 * if we can and wait for a reset.But We only support Bulk Only, so
		 this branch will never be executed */
		case DATA_DIR_UNKNOWN:
			if (mod_data.can_stall) {
				fsg_set_halt(fsg, fsg->bulk_out);
				rc = halt_bulk_in_endpoint(fsg);
			}
			break;
	
		/* All but the last buffer of data must have already been sent */
		case DATA_DIR_TO_HOST:
			if (fsg->data_size == 0)
				;		// Nothing to send
	
			/* If there's no residue, simply send the last buffer */
			else if (fsg->residue == 0) {
				bh->inreq->zero = 0;
				start_transfer(fsg, fsg->bulk_in, bh->inreq,
						&bh->inreq_busy, &bh->state);
				fsg->next_buffhd_to_fill = bh->next;
			}
	
			/* For Bulk-only, if we're allowed to stall then send the
			 * short packet and halt the bulk-in endpoint.	If we can't
			 * stall, pad out the remaining data with 0's. */
			else {
				if (mod_data.can_stall) {
					bh->inreq->zero = 1;
					start_transfer(fsg, fsg->bulk_in, bh->inreq,
							&bh->inreq_busy, &bh->state);
					fsg->next_buffhd_to_fill = bh->next;
					rc = halt_bulk_in_endpoint(fsg);
				} else{					
					rc = pad_with_zeros(fsg);
				}
			}
			break;
	
		/* We have processed all we want from the data the host has sent.
		 * There may still be outstanding bulk-out requests. */
		case DATA_DIR_FROM_HOST:
			if (fsg->residue == 0)
				;		// Nothing to receive
	
			/* Did the host stop sending unexpectedly early? */
			else if (fsg->short_packet_received) {
				raise_exception(fsg, FSG_STATE_ABORT_BULK_OUT);
				rc = -EINTR;
			}
				  
			/* We haven't processed all the incoming data.	Even though
			 * we may be allowed to stall, doing so would cause a race.
			 * The controller may already have ACK'ed all the remaining
			 * bulk-out packets, in which case the host wouldn't see a
			 * STALL.  Not realizing the endpoint was halted, it wouldn't
			 * clear the halt -- leading to problems later on. */
#if 0
			else if (mod_data.can_stall) {
				fsg_set_halt(fsg, fsg->bulk_out);
				raise_exception(fsg, FSG_STATE_ABORT_BULK_OUT);
				rc = -EINTR;
			}
#endif
	
			/* We can't stall.	Read in the excess data and throw it
			 * all away. */
			else
				rc = throw_away_data(fsg);
			break;
		}
		return rc;

}





static int send_status(struct fsg_dev *fsg)
{
	struct lun		*curlun = fsg->curlun;
	struct fsg_buffhd	*bh;
	struct bulk_cs_wrap	*csw;
	int			rc;
	u8			status = USB_STATUS_PASS;
	u32			sd, sdinfo = 0;
	
	/* Wait for the next buffer to become available */
	bh = fsg->next_buffhd_to_fill;
	while (bh->state != BUF_STATE_EMPTY){
		rc = sleep_thread(fsg);
		if (rc)
			return rc;
	}
	
       if (curlun)  {
	   	if(fsg->adfu_update && (fsg->lun == 0)){
			sd = fsg->adfu_sense;
		 	fsg->adfu_sense = SS_NO_SENSE;
	   	}else{
			sd = curlun->sense_data;
		 	sdinfo = curlun->sense_data_info;
		}
       }else if (fsg->bad_lun_okay)
            sd = SS_NO_SENSE;
      else
            sd = SS_LOGICAL_UNIT_NOT_SUPPORTED;

       if (fsg->phase_error) {
		fsg_dbg( "sending phase-error status\n");
		status = USB_STATUS_PHASE_ERROR;
		sd = SS_INVALID_COMMAND;
       } else if (sd != SS_NO_SENSE) {
		fsg_dbg( "sending command-failure status\n");
		status = USB_STATUS_FAIL;
		fsg_dbg("sense data: SK x%02x, ASC x%02x, ASCQ x%02x;"
				"  info x%x\n",SK(sd), ASC(sd), ASCQ(sd), sdinfo);
       }

	/* Store and send the Bulk-only CSW */
	csw = (struct bulk_cs_wrap *) bh->buf;
		csw->Signature = __constant_cpu_to_le32(USB_BULK_CS_SIG);
	csw->Tag = fsg->tag;
		csw->Residue = cpu_to_le32(fsg->residue);
	csw->Status = status;

	bh->inreq->length = USB_BULK_CS_WRAP_LEN;
	bh->inreq->zero = 0;
	
	start_transfer(fsg, fsg->bulk_in, bh->inreq,
			&bh->inreq_busy, &bh->state);
	
	fsg->next_buffhd_to_fill = bh->next;
	return 0;	
}

//check cmd valid
int check_adfu_command(struct fsg_dev *fsg,
    enum data_direction data_dir,
	const char *name)
{
	static const char dirletter[4] = {'u', 'o', 'i', 'n'};
	char      hdlen[20];																								

	hdlen[0] = 0;
	if (fsg->data_dir != DATA_DIR_UNKNOWN)
		sprintf(hdlen, ", H%c=%u", dirletter[(int) fsg->data_dir],fsg->data_size);

	//printk(KERN_ALERT "ADFU command: %s; LUN=%d, Dc=%d, D%c=%u;  Hc=%d%s\n",
	//	name, fsg->lun,fsg->cmnd_size, dirletter[(int) data_dir],
	//		fsg->data_size_from_cmnd, fsg->cmnd_size, hdlen);

	/* We can't reply at all until we know the correct data direction
	* and size. */
	
	if (fsg->data_size_from_cmnd == 0)
		data_dir = DATA_DIR_NONE;

	if (fsg->data_size < fsg->data_size_from_cmnd)
		printk(KERN_ALERT "Host Data < Device Data\n");
	
	fsg->residue = fsg->usb_amount_left = fsg->data_size_from_cmnd;
	/* Conflicting data directions is a phase error */
	if (fsg->data_dir != data_dir && fsg->data_size_from_cmnd > 0) {
		fsg->phase_error = 1;
		return -EINVAL;
	}
	return 0;
}


/*-------------------------------------------------------------------------*/

/* Check whether the command is properly formed and whether its data size
 * and direction agree with the values we already have. */
static int check_command(struct fsg_dev *fsg, int cmnd_size,
		enum data_direction data_dir, unsigned int mask,
		int needs_medium, const char *name)
{
	int			i;
	int			lun = fsg->cmnd[1] >> 5;
	static const char	dirletter[4] = {'u', 'o', 'i', 'n'};
	char			hdlen[20];
	struct lun		*curlun;

	/* Adjust the expected cmnd_size for protocol encapsulation padding.
	 * Transparent SCSI doesn't pad. */
	//cmnd_size =12;
	hdlen[0] = 0;
	if (fsg->data_dir != DATA_DIR_UNKNOWN)
		sprintf(hdlen, ", H%c=%u", dirletter[(int) fsg->data_dir],
				fsg->data_size);
	
	fsg_dbg( "SCSI: %s;  Dc:%d, D%c:%u;  Hc:%d%s\n",
			name, cmnd_size, dirletter[(int) data_dir],
			fsg->data_size_from_cmnd, fsg->cmnd_size, hdlen);

	/* We can't reply at all until we know the correct data direction
	 * and size. */
	if (fsg->data_size_from_cmnd == 0)
		data_dir = DATA_DIR_NONE;
	
	if (fsg->data_dir == DATA_DIR_UNKNOWN) {	// CB or CBI
		fsg->data_dir = data_dir;
		fsg->data_size = fsg->data_size_from_cmnd;
	} else {					// Bulk-only
		if (fsg->data_size < fsg->data_size_from_cmnd) {

			/* Host data size < Device data size is a phase error.
			 * Carry out the command, but only transfer as much
			 * as we are allowed. */
			fsg->data_size_from_cmnd = fsg->data_size;
			fsg->phase_error = 1;
		}
	}
	
	fsg->residue = fsg->usb_amount_left = fsg->data_size;

	/* Conflicting data directions is a phase error */
	if (fsg->data_dir != data_dir && fsg->data_size_from_cmnd > 0) {
		fsg->phase_error = 1;
		return -EINVAL;
	}
	
	/* Verify the length of the command itself */
	/*no need to do this,else condition will cause mac os not nitify the disk*/
	/*
	if (cmnd_size != fsg->cmnd_size) {

		// Special case workaround: MS-Windows issues REQUEST SENSE
		 // with cbw->Length == 12 (it should be 6). 
		if (fsg->cmnd[0] == SC_REQUEST_SENSE && fsg->cmnd_size == 12)
			cmnd_size = fsg->cmnd_size;
		else {
			fsg->phase_error = 1;
			return -EINVAL;
		}
	}
	*/
	/* Check that the LUN values are consistent */
	if (fsg->lun != lun)
		fsg_dbg( "using LUN %d from CBW, "
					"not LUN %d from CDB\n",
					fsg->lun, lun);

	/* Check the LUN */
	if (fsg->lun >= 0 && fsg->lun < fsg->nluns) {
		fsg->curlun = curlun = &fsg->luns[fsg->lun];
		if (fsg->cmnd[0] != SC_REQUEST_SENSE) {
			curlun->sense_data = SS_NO_SENSE;
			curlun->sense_data_info = 0;
			curlun->info_valid = 0;
		}
	} else {
		fsg->curlun = curlun = NULL;
		fsg->bad_lun_okay = 0;

		/* INQUIRY and REQUEST SENSE commands are explicitly allowed
		 * to use unsupported LUNs; all others may not. */
		if (fsg->cmnd[0] != SC_INQUIRY &&
				fsg->cmnd[0] != SC_REQUEST_SENSE) {
			fsg_dbg( "unsupported LUN %d\n", fsg->lun);
			return -EINVAL;
		}
	}

	/* If a unit attention condition exists, only INQUIRY and
	 * REQUEST SENSE commands are allowed; anything else must fail. */
	if (curlun && curlun->unit_attention_data != SS_NO_SENSE &&
			fsg->cmnd[0] != SC_INQUIRY &&
			fsg->cmnd[0] != SC_REQUEST_SENSE) {
		curlun->sense_data = curlun->unit_attention_data;
		curlun->unit_attention_data = SS_NO_SENSE;
		return -EINVAL;
	}
	
	/* Check that only command bytes listed in the mask are non-zero */
	fsg->cmnd[1] &= 0x1f;			// Mask away the LUN
	for (i = 1; i < cmnd_size; ++i) {
		if (fsg->cmnd[i] && !(mask & (1 << i))) {
			if (curlun)
				curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
			return -EINVAL;
		}
	}

	/* If the medium isn't mounted and the command needs to access
	 * it, return an error. */
	if (curlun &&!backing_file_is_open(curlun) && needs_medium) {
		fsg_dbg("media not present \n");
		curlun->sense_data = SS_MEDIUM_NOT_PRESENT;
		return -EINVAL;
	}
	
	fsg_dbg("check %s param sucessfully\n",name);
	return 0;
}


static int do_scsi_command(struct fsg_dev *fsg)
{
	struct fsg_buffhd	*bh;
	int			rc;
	int			reply = -EINVAL;
	int			i;
	static char		unknown[16];

	dump_cdb(fsg);

	/* Wait for the next buffer to become available for data or status */
	bh = fsg->next_buffhd_to_drain = fsg->next_buffhd_to_fill;
	while (bh->state != BUF_STATE_EMPTY) {
		rc = sleep_thread(fsg);
		if (rc)
			return rc;
	}
	
	fsg->phase_error = 0;
	fsg->short_packet_received = 0;

	down_read(&fsg->filesem);	// We're using the backing file
	//printk("-----scsi:%02x\n",fsg->cmnd[0]);
	switch (fsg->cmnd[0]) {

	case SC_INQUIRY:
		fsg->data_size_from_cmnd = fsg->cmnd[4];
		if ((reply = check_command(fsg, 6, DATA_DIR_TO_HOST,
				(1<<4), 0,
				"INQUIRY")) == 0)
			reply = do_inquiry(fsg, bh);
		break;
#if 1	
 	case SC_GET_CONFIGURATION:
		fsg->data_size_from_cmnd = fsg->cmnd[8];
		if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
				(1<<1) | (3<<7), 0, 
				"GET CONFIGURATION")) == 0)
			reply = do_get_config(fsg, bh);
		break;

	case SC_SET_CDROM_SPEED:
		fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
		
		if ((reply = check_command(fsg, 12, DATA_DIR_FROM_HOST,
				(1<<1) | (0xf<<2) | (0xf<<6), 0,
				"SC_SET_CDROM_SPEED")) == 0)
			reply = do_set_speed(fsg, bh);		
		break;

	case SC_READ_CD:
		fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
		

		if ((reply = check_command(fsg, 12, DATA_DIR_TO_HOST,
				(1<<1) | (0xf<<2) | (0xf<<6), 0,
				"SC_READ_CD")) == 0)
			reply = do_read_cd(fsg, bh);		
		break;
#endif	
	case SC_MODE_SELECT_6:
		fsg->data_size_from_cmnd = fsg->cmnd[4];
		if ((reply = check_command(fsg, 6, DATA_DIR_FROM_HOST,
				(1<<1) | (1<<4), 0,
				"MODE SELECT(6)")) == 0)
			reply = do_mode_select(fsg, bh);
		break;

	case SC_MODE_SELECT_10:
		fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
		if ((reply = check_command(fsg, 10, DATA_DIR_FROM_HOST,
				(1<<1) | (3<<7), 0,
				"MODE SELECT(10)")) == 0)
			reply = do_mode_select(fsg, bh);
		break;

	case SC_MODE_SENSE_6:
		fsg->data_size_from_cmnd = fsg->cmnd[4];
		if ((reply = check_command(fsg, 6, DATA_DIR_TO_HOST,
				(1<<1) | (1<<2) | (1<<4), 0,
				"MODE SENSE(6)")) == 0)
			reply = do_mode_sense(fsg, bh);
		break;

	case SC_MODE_SENSE_10:
		fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
		if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
				(1<<1) | (1<<2) | (3<<7), 0,
				"MODE SENSE(10)")) == 0)
			reply = do_mode_sense(fsg, bh);
		break;

	case SC_PREVENT_ALLOW_MEDIUM_REMOVAL:
		fsg->data_size_from_cmnd = 0;
		if ((reply = check_command(fsg, 6, DATA_DIR_NONE,
				(1<<4), 0,
				"PREVENT-ALLOW MEDIUM REMOVAL")) == 0)
			reply = do_prevent_allow(fsg);
		break;

	case SC_READ_6:
		update_ustatus(fsg,FSG_SUB_STATUS_READ);
		i = fsg->cmnd[4];
		fsg->data_size_from_cmnd = (i == 0 ? 256 : i) << 9;
		if ((reply = check_command(fsg, 6, DATA_DIR_TO_HOST,
				(7<<1) | (1<<4), 1,
				"READ(6)")) == 0)
			reply = do_read(fsg);
	//	update_main_status(fsg,FSG_STATUS_MAIN_IDLE);
		break;

	case SC_READ_10:
		update_ustatus(fsg,FSG_SUB_STATUS_READ);
		fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]) << 9;
		if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
				(1<<1) | (0xf<<2) | (3<<7), 1,
				"READ(10)")) == 0)
			reply = do_read(fsg);
		//update_main_status(fsg,FSG_STATUS_MAIN_IDLE);
		break;

	case SC_READ_12:
		update_ustatus(fsg,FSG_SUB_STATUS_READ);
		fsg->data_size_from_cmnd = get_be32(&fsg->cmnd[6]) << 9;
		if ((reply = check_command(fsg, 12, DATA_DIR_TO_HOST,
				(1<<1) | (0xf<<2) | (0xf<<6), 1,
				"READ(12)")) == 0)
			reply = do_read(fsg);
		//update_main_status(fsg,FSG_STATUS_MAIN_IDLE);
		break;
		
	case SC_READ_CAPACITY:
		fsg->data_size_from_cmnd = 8;
		if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
				(0xf<<2) | (1<<8), 1,
				"READ CAPACITY")) == 0)
			reply = do_read_capacity(fsg, bh);
		break;

	case SC_READ_HEADER:
		if (!mod_data.cdrom)
			goto unsupport_cmd;
		fsg->data_size_from_cmnd = get_unaligned_be16(&fsg->cmnd[7]);
		if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
				(3<<7) | (0x1f<<1), 1,
				"READ HEADER")) == 0)
			reply = do_read_header(fsg, bh);
		break;
#if 0
	case SC_READ_TOC:
		if (!mod_data.cdrom)
			goto unsupport_cmd;
		#if 1  //bq :add to change
		printk("debug---%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
		for(i=0;i<fsg->cmnd_size;i++)
			printk("debug %d ---%02x\n",__LINE__,fsg->cmnd[i]);
		#endif
		fsg->data_size_from_cmnd = get_unaligned_be16(&fsg->cmnd[7]);
		if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
				(7<<6) | (1<<1), 1,
				"READ TOC")) == 0)
				printk("debug---%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
			reply = do_read_toc(fsg, bh);
		break;
#else
	case SC_READ_TOC:
		if (!mod_data.cdrom)
			goto unsupport_cmd;
		#if 0  //bq :add to change
		printk("debug---%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
		for(i=0;i<fsg->cmnd_size;i++)
			printk("debug %d ---%02x\n",__LINE__,fsg->cmnd[i]);
		#endif
		fsg->data_size_from_cmnd = get_unaligned_be16(&fsg->cmnd[7]);
		if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
				(0xf<<6) | (1<<1), 1,
				"READ TOC")) == 0)
			reply = do_read_toc(fsg, bh);

		break;
#endif
	case SC_READ_FORMAT_CAPACITIES:
		fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
		if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
				(3<<7), 1,
				"READ FORMAT CAPACITIES")) == 0)
			reply = do_read_format_capacities(fsg, bh);
		break;

	case SC_REQUEST_SENSE:
		fsg->data_size_from_cmnd = fsg->cmnd[4];
		if ((reply = check_command(fsg, 6, DATA_DIR_TO_HOST,
				(1<<4), 0,
				"REQUEST SENSE")) == 0)
			reply = do_request_sense(fsg, bh);
		break;

	case SC_START_STOP_UNIT:
		fsg->data_size_from_cmnd = 0;
		if ((reply = check_command(fsg, 6, DATA_DIR_NONE,
				(1<<1) | (1<<4), 0,
				"START-STOP UNIT")) == 0)
			reply = do_start_stop(fsg);
		break;

	case SC_SYNCHRONIZE_CACHE:
		fsg->data_size_from_cmnd = 0;
		if ((reply = check_command(fsg, 10, DATA_DIR_NONE,
				(0xf<<2) | (3<<7), 1,
				"SYNCHRONIZE CACHE")) == 0)
			reply = do_synchronize_cache(fsg);
		break;

	case SC_TEST_UNIT_READY:
		fsg->data_size_from_cmnd = 0;
		reply = check_command(fsg, 6, DATA_DIR_NONE,
				0, 1,
				"TEST UNIT READY");
		break;

	/* Although optional, this command is used by MS-Windows.  We
	 * support a minimal version: BytChk must be 0. */
	case SC_VERIFY:
		fsg->data_size_from_cmnd = 0;
		if ((reply = check_command(fsg, 10, DATA_DIR_NONE,
				(1<<1) | (0xf<<2) | (3<<7), 1,
				"VERIFY")) == 0)
			reply = do_verify(fsg);
		break;

	case SC_WRITE_6:
		update_ustatus(fsg,FSG_SUB_STATUS_WRITE);
		i = fsg->cmnd[4];
		fsg->data_size_from_cmnd = (i == 0 ? 256 : i) << 9;
		if ((reply = check_command(fsg, 6, DATA_DIR_FROM_HOST,
				(7<<1) | (1<<4), 1,
				"WRITE(6)")) == 0)
			reply = do_write(fsg);
	//	update_main_status(fsg,FSG_STATUS_MAIN_IDLE);
		break;

	case SC_WRITE_10:
		update_ustatus(fsg,FSG_SUB_STATUS_WRITE);
		fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]) << 9;
		if ((reply = check_command(fsg, 10, DATA_DIR_FROM_HOST,
				(1<<1) | (0xf<<2) | (3<<7), 1,
				"WRITE(10)")) == 0)
			reply = do_write(fsg);
	//	update_main_status(fsg,FSG_STATUS_MAIN_IDLE);
		break;

	case SC_WRITE_12:
		update_ustatus(fsg,FSG_SUB_STATUS_WRITE);
		fsg->data_size_from_cmnd = get_be32(&fsg->cmnd[6]) << 9;
		if ((reply = check_command(fsg, 12, DATA_DIR_FROM_HOST,
				(1<<1) | (0xf<<2) | (0xf<<6), 1,
				"WRITE(12)")) == 0)
			reply = do_write(fsg);
		break;
		
	/* Comand belows used for us to update our  firmware*/	
#ifdef ADFU_UPDATE
	case SC_USB_ADFU_GETDISK:
		fsg->data_size_from_cmnd = 0x09; 
		if((reply = check_adfu_command(fsg,DATA_DIR_TO_HOST,"ADFU_GETDISK")) == 0)
			reply = do_adfu_get_devid(fsg,bh);
		break;
	case SC_USB_ADFU_TESTREADY:
		  fsg->data_size_from_cmnd = 2;
		if((reply = check_adfu_command(fsg,DATA_DIR_TO_HOST,"ADFU_TEST_READY")) == 0)
			reply = do_adfu_test_dev_ready(fsg,bh);
		break;
		
	case SC_USB_ADFU_GETSYSINFO:	
		fsg->data_size_from_cmnd = get_le16(&fsg->cmnd[7]);
		if((reply = check_adfu_command(fsg,DATA_DIR_TO_HOST,"ADFU_GETSYSINFO")) == 0)
			reply = do_adfu_get_sysinfo(fsg,bh);
		break;
		
	case SC_USB_ADFU_SENDATA:
		update_ustatus(fsg,FSG_SUB_STATUS_UPDATE);
		reply = do_adfu_send_data(fsg,bh);
		break;
#endif

	case SC_USB_SWITCH_2_SUBDISP:
		if(mod_data.support_switch){
			fsg->data_size_from_cmnd = 0; 
			fsg_info("get config change cmd \n");
			raise_exception(fsg, FSG_STATE_SOFTDISCON);
			reply = 0;
		}else{
			fsg_info("unsupport scsi command\n");
			goto unsupport_cmd;
		}
		break;
	case SC_USB_SWITCH_2_DEBUG:
		if(mod_data.support_switch){
			fsg->data_size_from_cmnd = 0; 
			fsg_info("get config change cmd \n");
			raise_exception(fsg, FSG_STATE_TODEBUG);
			reply = 0;
		}else{
			fsg_info("unsupport scsi command\n");
			goto unsupport_cmd;
		}
		break;
	/* Some mandatory commands that we recognize but don't implement.
	 * They don't mean much in this setting.  It's left as an exercise
	 * for anyone interested to implement RESERVE and RELEASE in terms
	 * of Posix locks. 
	 */	
	case SC_FORMAT_UNIT:
	case SC_RELEASE:
	case SC_RESERVE:
	case SC_SEND_DIAGNOSTIC:
	default:
unsupport_cmd:		
		fsg->data_size_from_cmnd = 0;
		sprintf(unknown, "Unknown x%02x", fsg->cmnd[0]);
		if ((reply = check_command(fsg, fsg->cmnd_size,
				DATA_DIR_UNKNOWN, 0xff, 0, unknown)) == 0) {
			fsg->curlun->sense_data = SS_INVALID_COMMAND;
			reply = -EINVAL;
		}
		break;
	}
	up_read(&fsg->filesem);

	if (reply == -EINTR || signal_pending(current))
		return -EINTR;

	/* Set up the single reply buffer for finish_reply() */
	if (reply == -EINVAL)
		reply = 0;		// Error reply length
		
	if (reply >= 0 && fsg->data_dir == DATA_DIR_TO_HOST) {
		reply = min((u32) reply, fsg->data_size_from_cmnd);
		bh->inreq->length = reply;
		bh->state = BUF_STATE_FULL;
		fsg->residue -= reply;
	}				// Otherwise it's already set
	return 0;
}


/*-------------------------------------------------------------------------*/

static int received_cbw(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
	struct usb_request	*req = bh->outreq;
	struct bulk_cb_wrap	*cbw = req->buf;

	/* Was this a real packet?  Should it be ignored? */
	if (req->status || test_bit(IGNORE_BULK_OUT, &fsg->atomic_bitflags))
		return -EINVAL;

	/* Is the CBW valid? */
	if (req->actual != USB_BULK_CB_WRAP_LEN ||
			cbw->Signature != __constant_cpu_to_le32(USB_BULK_CB_SIG)) {
		fsg_dbg( "invalid CBW: len %u sig 0x%x\n",
				req->actual,
				le32_to_cpu(cbw->Signature));

		/* The Bulk-only spec says we MUST stall the IN endpoint
		 * (6.6.1), so it's unavoidable.  It also says we must
		 * retain this state until the next reset, but there's
		 * no way to tell the controller driver it should ignore
		 * Clear-Feature(HALT) requests.
		 *
		 * We aren't required to halt the OUT endpoint; instead
		 * we can simply accept and discard any data received
		 * until the next reset. */
		wedge_bulk_in_endpoint(fsg);
		set_bit(IGNORE_BULK_OUT, &fsg->atomic_bitflags);
		return -EINVAL;
	}

	/* Is the CBW meaningful? */
//	if (cbw->Lun >= MAX_LUNS || cbw->Flags & ~USB_BULK_IN_FLAG ||
	if (cbw->Lun >= mod_data.num_filenames|| cbw->Flags & ~USB_BULK_IN_FLAG ||
			cbw->Length <= 0 || cbw->Length > MAX_COMMAND_SIZE) {
		fsg_dbg( "non-meaningful CBW: lun = %u, flags = 0x%x, "
				"cmdlen %u\n",
				cbw->Lun, cbw->Flags, cbw->Length);

		/* We can do anything we want here, so let's stall the
		 * bulk pipes if we are allowed to. */
		if (mod_data.can_stall) {
			fsg_set_halt(fsg, fsg->bulk_out);
			halt_bulk_in_endpoint(fsg);
		}
		return -EINVAL;
	}

	/* Save the command for later */
	fsg->cmnd_size = cbw->Length;
	memcpy(fsg->cmnd, cbw->CDB, fsg->cmnd_size);

	if (cbw->Flags & USB_BULK_IN_FLAG)
		fsg->data_dir = DATA_DIR_TO_HOST;
	else
		fsg->data_dir = DATA_DIR_FROM_HOST;
	
	fsg->data_size = le32_to_cpu(cbw->DataTransferLength);
	if (fsg->data_size == 0)
		fsg->data_dir = DATA_DIR_NONE;
	fsg->lun = cbw->Lun;
	fsg->tag = cbw->Tag;
	return 0;
}


static int get_next_command(struct fsg_dev *fsg)
{
	struct fsg_buffhd	*bh;
	int			rc = 0;
	
	/* Wait for the next buffer to become available */
	bh = fsg->next_buffhd_to_fill;
	while (bh->state != BUF_STATE_EMPTY) {
		rc = sleep_thread(fsg);
		if (rc)
			return rc;
	}
	
	/* Queue a request to read a Bulk-only CBW */
	set_bulk_out_req_length(fsg, bh, USB_BULK_CB_WRAP_LEN);
	bh->outreq->short_not_ok = 1;
	start_transfer(fsg, fsg->bulk_out, bh->outreq,&bh->outreq_busy, &bh->state);

	/* We will drain the buffer in software, which means we
	 * can reuse it for the next filling.  No need to advance
	 * next_buffhd_to_fill. */

	/* Wait for the CBW to arrive */
	while (bh->state != BUF_STATE_FULL) {
		rc = sleep_thread(fsg);
		if (rc)
			return rc;
	}
	
	smp_rmb();
	rc = received_cbw(fsg, bh);
	bh->state = BUF_STATE_EMPTY;

	return rc;
}


/*-------------------------------------------------------------------------*/

static int enable_endpoint(struct fsg_dev *fsg, struct usb_ep *ep,
		const struct usb_endpoint_descriptor *d)
{
	int	rc;
	ep->driver_data = fsg;
	rc = usb_ep_enable(ep, d);
	if (rc)
		fsg_err("can't enable %s, result %d\n", ep->name, rc);
	return rc;
}

static int alloc_request(struct fsg_dev *fsg, struct usb_ep *ep,
		struct usb_request **preq)
{
	*preq = usb_ep_alloc_request(ep, GFP_ATOMIC);
	if (*preq)
		return 0;
	fsg_err("can't allocate request for %s\n", ep->name);
	return -ENOMEM;
}

/*
 * Reset interface setting and re-init endpoint state (toggle etc).
 * Call with altsetting < 0 to disable the interface.  The only other
 * available altsetting is 0, which enables the interface.
 */
static int do_set_interface(struct fsg_dev *fsg, int altsetting)
{
	int	rc = 0;
	int	i;
	const struct usb_endpoint_descriptor	*d;
	
	if (fsg->running)
		fsg_dbg( "reset interface\n");
reset:
	/* Deallocate the requests */
	for (i = 0; i < NUM_BUFFERS; ++i) {
		struct fsg_buffhd *bh = &fsg->buffhds[i];

		if (bh->inreq) {
			usb_ep_free_request(fsg->bulk_in, bh->inreq);
			bh->inreq = NULL;
		}
		if (bh->outreq) {
			usb_ep_free_request(fsg->bulk_out, bh->outreq);
			bh->outreq = NULL;
		}
	}
	
	if (fsg->intreq) {
		usb_ep_free_request(fsg->intr_in, fsg->intreq);
		fsg->intreq = NULL;
	}

	/* Disable the endpoints */
	if (fsg->bulk_in_enabled) {
		usb_ep_disable(fsg->bulk_in);
		fsg->bulk_in_enabled = 0;
	}
	if (fsg->bulk_out_enabled) {
		usb_ep_disable(fsg->bulk_out);
		fsg->bulk_out_enabled = 0;
	}
	if (fsg->intr_in_enabled) {
		usb_ep_disable(fsg->intr_in);
		fsg->intr_in_enabled = 0;
	}

	fsg->running = 0;
	if (altsetting < 0 || rc != 0)
		return rc;
	
	fsg_dbg( "set interface %d\n", altsetting);
	
	/* Enable the endpoints */
	d = ep_desc(fsg->gadget, &fs_bulk_in_desc, &hs_bulk_in_desc);	
	if ((rc = enable_endpoint(fsg, fsg->bulk_in, d)) != 0)
		goto reset;
	fsg->bulk_in_enabled = 1;

	d = ep_desc(fsg->gadget, &fs_bulk_out_desc, &hs_bulk_out_desc);
	if ((rc = enable_endpoint(fsg, fsg->bulk_out, d)) != 0)
		goto reset;
	fsg->bulk_out_enabled = 1;
	fsg->bulk_out_maxpacket = le16_to_cpu(d->wMaxPacketSize);
	clear_bit(IGNORE_BULK_OUT, &fsg->atomic_bitflags);

	/* Allocate the requests */
	for (i = 0; i < NUM_BUFFERS; ++i) {
		struct fsg_buffhd	*bh = &fsg->buffhds[i];
		if ((rc = alloc_request(fsg, fsg->bulk_in, &bh->inreq)) != 0)
			goto reset;
		if ((rc = alloc_request(fsg, fsg->bulk_out, &bh->outreq)) != 0)
			goto reset;
		bh->inreq->buf = bh->outreq->buf = bh->buf;
		bh->inreq->context = bh->outreq->context = bh;
		bh->inreq->complete = bulk_in_complete;
		bh->outreq->complete = bulk_out_complete;
	}

	fsg->running = 1;
	for (i = 0; i < fsg->nluns; ++i)
		fsg->luns[i].unit_attention_data = SS_RESET_OCCURRED;

	return rc;
}


/*
 * Change our operational configuration.  This code must agree with the code
 * that returns config descriptors, and with interface altsetting code.
 *
 * It's also responsible for power management interactions.  Some
 * configurations might not work with our current power sources.
 * For now we just assume the gadget is always self-powered.
 */
static int do_set_config(struct fsg_dev *fsg, u8 new_config)
{
	int	rc = 0;
	
	/* Disable the single interface */
	if (fsg->config != 0) {
		fsg_dbg( "reset config\n");
		fsg->config = 0;
		rc = do_set_interface(fsg, -1);
	}

	/* Enable the interface */
	if (new_config != 0) {
		fsg->config = new_config;
		if ((rc = do_set_interface(fsg, 0)) != 0){
			fsg->config = 0;	// Reset on errors
		}	
		else {
			char *speed;
			switch (fsg->gadget->speed) {
			case USB_SPEED_LOW:	speed = "low";	break;
			case USB_SPEED_FULL:	speed = "full";	break;
			case USB_SPEED_HIGH:	speed = "high";	break;
			default: 		speed = "?";	break;
			}
			fsg_info("%s speed set config #%d sucess!\n", speed, fsg->config);
		}
	}
	return rc;
}


/*-------------------------------------------------------------------------*/

static void handle_exception(struct fsg_dev *fsg)
{
	siginfo_t		info;
	int			sig;
	int			i;
	int			num_active;
	struct fsg_buffhd	*bh;
	enum fsg_state		old_state;
	u8			new_config;
	struct lun		*curlun;
	unsigned int		exception_req_tag;
	int			rc;
	struct am_sys_msg msg;
	/* Clear the existing signals.  Anything but SIGUSR1 is converted
	 * into a high-priority EXIT exception. */
	for (;;) {
		sig = dequeue_signal_lock(current, &current->blocked, &info);
		if (!sig)
			break;
		if (sig != SIGUSR1) {
			if (fsg->state < FSG_STATE_EXIT)
				fsg_dbg( "Main thread exiting on signal\n");
			raise_exception(fsg, FSG_STATE_EXIT);
		}
	}

	/* Cancel all the pending transfers */
	if (fsg->intreq_busy)
		usb_ep_dequeue(fsg->intr_in, fsg->intreq);
	
	for (i = 0; i < NUM_BUFFERS; ++i) {
		bh = &fsg->buffhds[i];
		if (bh->inreq_busy)
			usb_ep_dequeue(fsg->bulk_in, bh->inreq);
		if (bh->outreq_busy)
			usb_ep_dequeue(fsg->bulk_out, bh->outreq);
	}

	/* Wait until everything is idle */
	for (;;) {
		num_active = fsg->intreq_busy;
		for (i = 0; i < NUM_BUFFERS; ++i) {
			bh = &fsg->buffhds[i];
			num_active += bh->inreq_busy + bh->outreq_busy;
		}
		if (num_active == 0)
			break;
		if (sleep_thread(fsg))
			return;
	}

	/* Clear out the controller's fifos */
	if (fsg->bulk_in_enabled)
		usb_ep_fifo_flush(fsg->bulk_in);
	if (fsg->bulk_out_enabled)
		usb_ep_fifo_flush(fsg->bulk_out);
	if (fsg->intr_in_enabled)
		usb_ep_fifo_flush(fsg->intr_in);

	/* Reset the I/O buffer states and pointers, the SCSI
	 * state, and the exception.  Then invoke the handler. */
	spin_lock_irq(&fsg->lock);

	for (i = 0; i < NUM_BUFFERS; ++i) {
		bh = &fsg->buffhds[i];
		bh->state = BUF_STATE_EMPTY;
	}
	fsg->next_buffhd_to_fill = fsg->next_buffhd_to_drain =
			&fsg->buffhds[0];

	exception_req_tag = fsg->exception_req_tag;
	new_config = fsg->new_config;
	old_state = fsg->state;

	if (old_state == FSG_STATE_ABORT_BULK_OUT)
		fsg->state = FSG_STATE_STATUS_PHASE;
	else {
		for (i = 0; i < fsg->nluns; ++i) {
			curlun = &fsg->luns[i];
			curlun->prevent_medium_removal = 0;
			curlun->sense_data = curlun->unit_attention_data =
					SS_NO_SENSE;
			curlun->sense_data_info = 0;
			curlun->info_valid = 0;
		}
		fsg->state = FSG_STATE_IDLE;
	}
	spin_unlock_irq(&fsg->lock);

	/* Carry out any extra actions required for the exception */
	switch (old_state) {
	default:
		break;

	case FSG_STATE_ABORT_BULK_OUT:
		send_status(fsg);
		spin_lock_irq(&fsg->lock);
		if (fsg->state == FSG_STATE_STATUS_PHASE)
			fsg->state = FSG_STATE_IDLE;
		
		spin_unlock_irq(&fsg->lock);
		break;

	case FSG_STATE_RESET:
		/* In case we were forced against our will to halt a
		 * bulk endpoint, clear the halt now.  (The SuperH UDC
		 * requires this.) */
		if (test_and_clear_bit(IGNORE_BULK_OUT, &fsg->atomic_bitflags))
			usb_ep_clear_halt(fsg->bulk_in);

		if (fsg->ep0_req_tag == exception_req_tag)
			ep0_queue(fsg);	// Complete the status stage
	
		/* Technically this should go here, but it would only be
		 * a waste of time.  Ditto for the INTERFACE_CHANGE and
		 * CONFIG_CHANGE cases. */
		// for (i = 0; i < fsg->nluns; ++i)
		//	fsg->luns[i].unit_attention_data = SS_RESET_OCCURRED;
		break;

	case FSG_STATE_INTERFACE_CHANGE:
		rc = do_set_interface(fsg, 0);
		if (fsg->ep0_req_tag != exception_req_tag)
			break;
		if (rc != 0)			// STALL on errors
			fsg_set_halt(fsg, fsg->ep0);
		else				// Complete the status stage
			ep0_queue(fsg);
		break;

	case FSG_STATE_CONFIG_CHANGE:
		rc = do_set_config(fsg, new_config);
		if (fsg->ep0_req_tag != exception_req_tag)
			break;
		if (rc != 0)			// STALL on errors
			fsg_set_halt(fsg, fsg->ep0);
		else				// Complete the status stage
			ep0_queue(fsg);
		break;
		
	case FSG_STATE_DISCONNECT:
		fsync_all(fsg);
		do_set_config(fsg, 0);		// Unconfigured state
		
		/*let out msg to inform user*/
		fsg_info("file-storage:disconnect!!\n");
		msg.type = SYSMSG_USB;
		msg.subtype = DEVICE_USB_OUT;
		msg.dataload = DEVICE_MASS_STORAGE;
	
		if(mod_data.msg_identify==0)
			msg.reserved[0]=0xa0;
		if(mod_data.msg_identify==1)
			msg.reserved[0]=0xa1;
	     
		am_put_sysmsg(msg);
		break;
		
	case FSG_STATE_SOFTDISCON:
		send_status(fsg);	
		msleep(10);
		usb_gadget_disconnect(fsg->gadget);
		
		/*let out msg to inform user*/
		fsg_info("put sys soft switch msg\n");
		msg.type = SYSMSG_USB;
		msg.subtype = DEVICE_USB_VENDOR;
		msg.dataload = DEVICE_MASS_2_SUBDISP;

		if(mod_data.msg_identify==0)
			msg.reserved[0]=0xa0;
		if(mod_data.msg_identify==1)
			msg.reserved[0]=0xa1;
	
		am_put_sysmsg(msg);
		break;

	case FSG_STATE_TODEBUG:
		send_status(fsg);	
		msleep(10);
		usb_gadget_disconnect(fsg->gadget);
		
		/*let out msg to inform user*/
		fsg_info("put sys soft switch msg to debug\n");
		msg.type = SYSMSG_USB;
		msg.subtype = DEVICE_USB_VENDOR;
		msg.dataload = DEVICE_MASS_2_DEBUG;

		if(mod_data.msg_identify==0)
			msg.reserved[0]=0xa0;
		if(mod_data.msg_identify==1)
			msg.reserved[0]=0xa1;

		am_put_sysmsg(msg);
		break;
		
	case FSG_STATE_EXIT:
	case FSG_STATE_TERMINATED:
		do_set_config(fsg, 0);			// Free resources
		spin_lock_irq(&fsg->lock);
		fsg->state = FSG_STATE_TERMINATED;	// Stop the thread
		spin_unlock_irq(&fsg->lock);
		break;
	}
}


/*-------------------------------------------------------------------------*/

static int fsg_main_thread(void *fsg_)
{
	struct fsg_dev		*fsg = fsg_;
	/* Allow the thread to be killed by a signal, but set the signal mask
	 * to block everything but INT, TERM, KILL, and USR1. */
	allow_signal(SIGINT);
	allow_signal(SIGTERM);
	allow_signal(SIGKILL);
	allow_signal(SIGUSR1);

	/* Allow the thread to be frozen */
	set_freezable();

	/* Arrange for userspace references to be interpreted as kernel
	 * pointers.  That way we can pass a kernel pointer to a routine
	 * that expects a __user pointer and it will work okay. */
	set_fs(get_ds());

	/* The main loop */
	while (fsg->state != FSG_STATE_TERMINATED) {
		if (exception_in_progress(fsg) || signal_pending(current)) {
			handle_exception(fsg);
			continue;
		}

		if (!fsg->running) {
			sleep_thread(fsg);
			continue;
		}

		if (get_next_command(fsg))
			continue;

		spin_lock_irq(&fsg->lock);
		if (!exception_in_progress(fsg))
			fsg->state = FSG_STATE_DATA_PHASE;
		spin_unlock_irq(&fsg->lock);

		if (do_scsi_command(fsg) || finish_reply(fsg))
			continue;

		spin_lock_irq(&fsg->lock);
		if (!exception_in_progress(fsg))
			fsg->state = FSG_STATE_STATUS_PHASE;
		spin_unlock_irq(&fsg->lock);

		if (send_status(fsg))
			continue;

		spin_lock_irq(&fsg->lock);
		if (!exception_in_progress(fsg))
			fsg->state = FSG_STATE_IDLE;
		spin_unlock_irq(&fsg->lock);
	}

	spin_lock_irq(&fsg->lock);
	fsg->thread_task = NULL;
	spin_unlock_irq(&fsg->lock);
	
	if(fsg->adfu_update)
		am_setup_watchdog(2);//delay 1.4s before reboot
	else{
	/* In case we are exiting because of a signal, unregister the
	 * gadget driver and close the backing file. */
		if (test_and_clear_bit(REGISTERED, &fsg->atomic_bitflags)) {
			usb_gadget_unregister_driver(&fsg_driver);
			close_all_backing_files(fsg);
		}
	}

	/* Let the unbind and cleanup routines know the thread has exited */
	complete_and_exit(&fsg->thread_notifier, 0);
}


static struct lun *  get_probe_lun(struct fsg_dev *fsg, const char *name)
{
	char *pathbuf;
	char *p=NULL;
	struct lun *curlun;
	struct lun *probe_lun;
	int i;
	
	pathbuf = kmalloc(PATH_MAX, GFP_KERNEL);
	if(!pathbuf){
		fsg_err("%s:malloc failed\n",__FUNCTION__);
		return NULL;
	}
	
	curlun = probe_lun = NULL;
	for (i = 0; i < fsg->nluns; ++i){
		curlun = &fsg->luns[i];
		if (backing_file_is_open(curlun)) {		
			p = d_path(&curlun->filp->f_path,
					   pathbuf, PATH_MAX);			
			if (IS_ERR(p)){
				p = NULL;
				continue;
			}	
			
			if(!strcmp(p,name)){
				fsg_info( " file: %s\n", (p ? p : "(error)"));
				probe_lun = curlun;
				break;
			}	
		}
	}
	
	kfree(pathbuf);
	return probe_lun;
}


static struct lun * find_idle_lun(struct fsg_dev *fsg)
{
	int i;
	struct lun *curlun;
	
	for (i = 0; i < fsg->nluns; ++i){
		curlun = &fsg->luns[i];
		if(!curlun->media_present 
			&& !backing_file_is_open(curlun)){
			return curlun;
		}
	}
	return NULL;
}

static inline int get_media_plugstatus(struct am_sys_msg *msg)
{
	/**/
	return (msg->subtype >>16) & 0xffff;
}

#define MAX_FILENAME		20
static void media_change_event(struct am_sys_msg *msg,void *param)
{
	struct fsg_dev *fsg = (struct fsg_dev *)param;
	char *pname;
	char filename[MAX_FILENAME];
	struct lun *curlun ;
	
	int is_in = get_media_plugstatus(msg);
	pname = (char *)msg->dataload;
	
	if(copy_from_user(filename,pname,MAX_FILENAME) < 0){
		fsg_info("transmit name from userspace failed\n");
		return;
	}
	
	if(is_in){
		curlun = find_idle_lun(fsg);
		if(!curlun){
			fsg_info("cannot find a idle lun\n");
			return;
		}
		
		down_write(&fsg->filesem);	
		if(open_backing_file(curlun,filename) < 0){
			fsg_info("cannot open file :%s\n",filename);
			up_write(&fsg->filesem);
			return;
		}
		
		curlun->unit_attention_data = SS_NOT_READY_TO_READY_TRANSITION;
		up_write(&fsg->filesem);
	}else{
		curlun = get_probe_lun(fsg,filename);
		if(!curlun){
			fsg_info("cannot find a media lun\n");
			return;
		}
		
		down_write(&fsg->filesem);
		close_backing_file(curlun);
		curlun->unit_attention_data = SS_MEDIUM_NOT_PRESENT;
		up_write(&fsg->filesem);	
	}
}


static int check_media_ops(struct media_ops *ops)
{
	if(ops &&(!ops->get_media_cap ||!ops->media_write
		||!ops->media_read ))
		return -EINVAL;
	
	return 0;
}
/*-------------------------------------------------------------------------*/

/* If the next two routines are called while the gadget is registered,
 * the caller must own fsg->filesem for writing. */

static int open_backing_file(struct lun *curlun, const char *filename)
{	
	char 				*ptr;
	struct file			*filp = NULL;
	struct inode			*inode = NULL;

	int					ro;
	int					rc = -EINVAL;
	loff_t				size;	
	loff_t				num_sectors;

	/* R/W if we can, R/O if we must */
	ro = curlun->ro;
	if (!ro) {
		filp = filp_open(filename, O_RDWR |O_LARGEFILE, 0);
		if (-EROFS == PTR_ERR(filp))
			ro = 1;
	}
	
	if (ro)
		filp = filp_open(filename, O_RDONLY |O_LARGEFILE, 0);

//printk("[debug %s %d]---------filp:0x%p ro:%d\n",__func__,__LINE__,filp,ro);
	if (IS_ERR(filp)) {
		fsg_info("unable to open backing file: %s\n", filename);
		return PTR_ERR(filp);
	}

	fsg_dbg(" %s %d file name1: %s\n", __FILE__,__LINE__,filename);
	if (!(filp->f_mode & FMODE_WRITE))
		ro = 1;
	
	if (filp->f_path.dentry)
		inode = filp->f_path.dentry->d_inode;
	
	if (inode && S_ISBLK(inode->i_mode)) {
		if (bdev_read_only(inode->i_bdev))
			ro = 1;
	} else if (!inode || !S_ISREG(inode->i_mode)) {
		fsg_info("invalid file type: %s\n", filename);
		goto out;
	}else if(!filp->f_op ){
		fsg_info("invalid file type: %s\n", filename);
		goto out;
	}
#if 1	
	if(filp->f_op->unlocked_ioctl){	
		curlun->media_ops = kzalloc(sizeof(struct media_ops),GFP_KERNEL);
		if(!curlun->media_ops){
			fsg_err("%s:malloc failed %s\n", __FUNCTION__,filename);
			goto out;
		}
		
		rc = filp->f_op->unlocked_ioctl(filp,IOCTL_GET_MEDIAOPS,(unsigned long)curlun->media_ops);
		if(rc != 0){
			fsg_err("%s:ioctl failed,rc:%d\n",__FUNCTION__,rc);
			kfree(curlun->media_ops);
			curlun->media_ops = NULL;
			goto fs_standard;
		}

		if( (rc = check_media_ops(curlun->media_ops)) != 0){
			fsg_err("media ops invalid\n");
			kfree(curlun->media_ops);
			curlun->media_ops = NULL;
			goto fs_standard;
		}
		
		fsg_info( "%s:ops valid\n",filename);
		
		ptr = strrchr(filename,'/');
		if(!ptr){
			fsg_err( "filename:%s invalid\n", filename);
			kfree(curlun->media_ops);
			curlun->media_ops = NULL;
			rc = -EINVAL;
			goto out;		
		}	
		
		//except '/'
		strcpy(curlun->lun_name,++ptr);
		fsg_dbg("lun_name:%s \n", curlun->lun_name);		
		num_sectors = curlun->media_ops->get_media_cap(curlun->lun_name);
		if(num_sectors <= 0 ){
			fsg_err( "media capacties too small\n");
			kfree(curlun->media_ops);
			curlun->media_ops = NULL;
			rc = -ETOOSMALL;
			goto out;
		}
		//private media interface check wp
		if(curlun->media_ops->media_wpdetect)
			ro = curlun->media_ops->media_wpdetect();	
		//file default in 512 sector
		size = num_sectors << 9;	
		curlun->use_media_ops = 1;
	}
	else	
#endif
	{
fs_standard:
		if ( !(filp->f_op->read || filp->f_op->aio_read)) {
			fsg_info( "file not readable: %s\n", filename);
			goto out;
		}

		if (!(filp->f_op->write || filp->f_op->aio_write))
			ro = 1;

		size = i_size_read(inode->i_mapping->host);
		if (size < 0) {
			fsg_info( "unable to find file size: %s\n", filename);
			rc = (int) size;
			goto out;
		}
	
		num_sectors = size >> 9;	// File size in 512-byte sectors
		if (num_sectors == 0) {
			fsg_info("file too small: %s\n", filename);
			rc = -ETOOSMALL;
			goto out;
		}
	}
	
	get_file(filp);
	curlun->ro = ro;
	curlun->filp = filp;
	curlun->file_length = size;
	curlun->num_sectors = num_sectors;	
	rc = 0;
out:
	//filp_close(filp, NULL);
	filp_close(filp, current->files);
	return rc;
}


static void close_backing_file(struct lun *curlun)
{
	if (curlun->filp) {
		fsg_dbg( "close backing file\n");
		fput(curlun->filp);
		curlun->filp = NULL;
		curlun->media_present   = 0; 
	}

	if(curlun->use_media_ops && curlun->media_ops){
		fsg_dbg( "destory lun media ops\n");
		kfree(curlun->media_ops);
		curlun->media_ops = NULL;
		curlun->use_media_ops = 0;
	}		
}

static void close_all_backing_files(struct fsg_dev *fsg)
{
	int	i;

	for (i = 0; i < fsg->nluns; ++i)
		close_backing_file(&fsg->luns[i]);
}


static ssize_t show_ro(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lun	*curlun = dev_to_lun(dev);
	return sprintf(buf, "%d\n", curlun->ro);
}

static ssize_t show_file(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct lun	*curlun = dev_to_lun(dev);
	struct fsg_dev	*fsg = dev_get_drvdata(dev);
	char		*p;
	ssize_t		rc;

	down_read(&fsg->filesem);
	if (backing_file_is_open(curlun)) {	// Get the complete pathname
		p = d_path(&curlun->filp->f_path, buf, PAGE_SIZE - 1);
		if (IS_ERR(p))
			rc = PTR_ERR(p);
		else {
			rc = strlen(p);
			memmove(buf, p, rc);
			buf[rc] = '\n';		// Add a newline
			buf[++rc] = 0;
		}
	} else {				// No file, return 0 bytes
		*buf = 0;
		rc = 0;
	}
	up_read(&fsg->filesem);
	return rc;
}


static ssize_t show_iswritten(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lun	*curlun = dev_to_lun(dev);
	return sprintf(buf, "%d\n", curlun->is_written);
}

static ssize_t show_ustate(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct fsg_dev	*fsg = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", fsg->status);
}

static ssize_t store_iswritten(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct lun	*curlun = dev_to_lun(dev);
	struct fsg_dev	*fsg = dev_get_drvdata(dev);
	int		is_written;

	if (sscanf(buf, "%d", &is_written) != 1)
		return -EINVAL;

	down_read(&fsg->filesem);
	curlun->is_written= !!is_written;
	fsg_info( "iswritten flag set to %d\n", curlun->is_written);
	up_read(&fsg->filesem);
	return 0;
}

static ssize_t store_ro(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	ssize_t		rc = count;
	struct lun	*curlun = dev_to_lun(dev);
	struct fsg_dev	*fsg = dev_get_drvdata(dev);
	int		i;

	if (sscanf(buf, "%d", &i) != 1)
		return -EINVAL;

	/* Allow the write-enable status to change only while the backing file
	 * is closed. */
	down_read(&fsg->filesem);
	if (backing_file_is_open(curlun)) {
		fsg_dbg( "read-only status change prevented\n");
		rc = -EBUSY;
	} else {
		curlun->ro = !!i;
		fsg_dbg( "read-only status set to %d\n", curlun->ro);
	}
	up_read(&fsg->filesem);
	return rc;
}

static ssize_t store_file(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct lun	*curlun = dev_to_lun(dev);
	struct fsg_dev	*fsg = dev_get_drvdata(dev);
	int		rc = 0;

	if (curlun->prevent_medium_removal && backing_file_is_open(curlun)) {
		fsg_dbg( "eject attempt prevented\n");
		return -EBUSY;				// "Door is locked"
	}

	/* Remove a trailing newline */
	if (count > 0 && buf[count-1] == '\n')
		((char *) buf)[count-1] = 0;		// Ugh!

	/* Eject current medium */
	down_write(&fsg->filesem);
	if (backing_file_is_open(curlun)) {
		close_backing_file(curlun);
		curlun->unit_attention_data = SS_MEDIUM_NOT_PRESENT;
	}

	/* Load new medium */
	if (count > 0 && buf[0]) {
		rc = open_backing_file(curlun, buf);
		if (rc == 0)
			curlun->unit_attention_data = SS_NOT_READY_TO_READY_TRANSITION;
		else
			fsg_err("cannot open device: %s\n",buf);
	}
	up_write(&fsg->filesem);
	return (rc < 0 ? rc : count);
}


/* The write permissions and store_xxx pointers are set in fsg_bind() */
static DEVICE_ATTR(ro, 0444, show_ro, NULL);
static DEVICE_ATTR(file, 0444, show_file, NULL);
static DEVICE_ATTR(iswritten, 0644, show_iswritten, store_iswritten);
static DEVICE_ATTR(ustate, 0444, show_ustate, NULL);
/*-------------------------------------------------------------------------*/

static void fsg_release(struct kref *ref)
{
	struct fsg_dev	*fsg = container_of(ref, struct fsg_dev, ref);

	kfree(fsg->luns);
	kfree(fsg);
}

static void lun_release(struct device *dev)
{
	struct fsg_dev	*fsg = dev_get_drvdata(dev);

	kref_put(&fsg->ref, fsg_release);
}

static void /* __init_or_exit */ fsg_unbind(struct usb_gadget *gadget)
{
	struct fsg_dev		*fsg = get_gadget_data(gadget);
	int			i;
	struct lun		*curlun;
	struct usb_request	*req = fsg->ep0req;

	fsg_dbg( "unbind\n");
	clear_bit(REGISTERED, &fsg->atomic_bitflags);

	/* Unregister the sysfs attribute files and the LUNs */
	for (i = 0; i < fsg->nluns; ++i) {
		curlun = &fsg->luns[i];
		if (curlun->registered) {
			device_remove_file(&curlun->dev, &dev_attr_ro);
			device_remove_file(&curlun->dev, &dev_attr_file);
			device_remove_file(&curlun->dev, &dev_attr_iswritten);
			device_remove_file(&curlun->dev, &dev_attr_ustate);
			device_unregister(&curlun->dev);
			curlun->registered = 0;
		}	
	}
	
	/* If the thread isn't already dead, tell it to exit now */
	if (fsg->state != FSG_STATE_TERMINATED) {
		raise_exception(fsg, FSG_STATE_EXIT);
		wait_for_completion(&fsg->thread_notifier);
		/* The cleanup routine waits for this completion also */
		complete(&fsg->thread_notifier);
	}

	/* Free the data buffers */
	for (i = 0; i < NUM_BUFFERS; ++i){
		free_pages((unsigned long)fsg->buffhds[i].buf,mod_data.buflen);
	}
	/* Free the request and buffer for endpoint 0 */
	if (req) {
		kfree(req->buf);
		usb_ep_free_request(fsg->ep0, req);
	}
	del_timer(&fsg->status_t);
	set_gadget_data(gadget, NULL);
}

static void autoconfig_usb_info(struct fsg_dev * fsg,char *config_name)
{
	int rc;
	char * tmp,*buffer;
	int n;
	struct lun * lun;
	
#define MAX_CONFIG_SIZE		512	
	buffer = vmalloc(MAX_CONFIG_SIZE);
	if(!buffer){
		fsg_err("vmalloc failed \n");
		return;
	}
	
	rc = am_get_config(config_name,buffer,0,MAX_CONFIG_SIZE);
	if(rc < 0){
		fsg_err("read config from %s failed\n",config_name);
		vfree(buffer);
		buffer=NULL;
		return;
	}
	
	tmp = buffer;
	/*config vid pid*/
	mod_data.vendor = get_le16(tmp);
	tmp +=2;

	mod_data.product = get_le16(tmp);
	tmp +=2;

	/*config manufacturer, product-name,serial_number*/
	sprintf(mod_data.manufacturer,"%s",tmp);
	tmp += sizeof(mod_data.manufacturer);

	sprintf(mod_data.productname,"%s",tmp);
	tmp +=sizeof(mod_data.productname);

	sprintf(mod_data.serials,"%s",tmp);
	tmp +=sizeof(mod_data.serials);

	sprintf(mod_data.inquiry_vendor,"%s",tmp);
	tmp +=sizeof(mod_data.inquiry_vendor);

	mod_data.release = get_le16(tmp);
	tmp+=2;	
	//for(n = 0; n < MAX_LUNS ; n++ ){
	for(n = 0; n < mod_data.num_filenames; n++ ){
		sprintf(mod_data.inquiry_products[n],"%s",tmp);
//printk("[debug %s %d]  >>>> ---------------------inquiry_products:%s\n",__func__,__LINE__,mod_data.inquiry_products[n]);
		tmp += sizeof(mod_data.inquiry_products[n]);
	}

	fsg_info( "VendorID=x%04x, ProductID=x%04x, Release=x%04x\n",
			mod_data.vendor, mod_data.product, mod_data.release);
	fsg_info("manufacturer:%s\n",mod_data.manufacturer);
	fsg_info("productname:%s\n",mod_data.productname);
	fsg_info("serial:%s\n",mod_data.serials);
	fsg_info("inquiry vendor name:%s\n",mod_data.inquiry_vendor);
	
	//for(n = 0; n < MAX_LUNS ; n++ ){
	for(n = 0; n < mod_data.num_filenames; n++ ){
		lun = &fsg->luns[n];
		lun->inquiry_name = mod_data.inquiry_products[n];
		fsg_info("inquiry product name:%s\n",lun->inquiry_name);
	}
	
	vfree(buffer);
}

static void handle_ustatus(void)
{
	struct fsg_dev *fsg;
	fsg = the_fsg;

	if(fsg==NULL)
		return;
	fsg->status = set_sub_status(fsg->status,FSG_SUB_STATUS_IDLE);
}
/*
*if you want to modify udisk number whick display in pc computer system ,you should :
*#1,insmod g_file_storage.ko file=/dev/partitions/udisk,/dev/loops ;display udisk and cdrom 
*#2.modify linux tool  usb mass storage lunx products options 
*/
static int __init fsg_bind(struct usb_gadget *gadget)
{
	struct fsg_dev		*fsg = the_fsg;
	int			rc;
	int			i;
	struct lun		*curlun=NULL;
	struct usb_ep		*ep;
	struct usb_request	*req;
	char			*pathbuf, *p;
#if 0
	if(mod_data.luns==4){
		static const char 	*default_lunname[] = {"internal-nand","mmc/ms/sd/xd","cf","cdrom"};
	}else if(mode_data.luns==2){
		static const char 	*default_lunname[] = {"internal-nand","cdrom-udisk"};
	}
#endif
	fsg->gadget = gadget;
	set_gadget_data(gadget, fsg);
	fsg->ep0 = gadget->ep0;
	fsg->ep0->driver_data = fsg;
	
	if (mod_data.removable) {	// Enable the store_xxx attributes
		dev_attr_ro.attr.mode = dev_attr_file.attr.mode = 0644;
		dev_attr_ro.store = store_ro;
		dev_attr_file.store = store_file;
	}

	/* Find out how many LUNs there should be */
	printk("[%s %d] num_filename=%d \n",__FUNCTION__,__LINE__,mod_data.num_filenames);
	i = mod_data.nluns;
	if (i == 0)
		i = max(mod_data.num_filenames, 1u);
	if (i > MAX_LUNS) {
		fsg_err("invalid number of LUNs: %d\n", i);
		rc = -EINVAL;
		goto out;
	}	
	
	/* Create the LUNs, open their backing files, and register the
	 * LUN devices in sysfs. */
	fsg->luns = kzalloc(i * sizeof(struct lun), GFP_KERNEL);
	if (!fsg->luns) {
		rc = -ENOMEM;
		goto out;
	}
	
	fsg->nluns = i;

	for (i = 0; i < fsg->nluns; ++i) {
		if(mod_data.cdrom==0){
			curlun = &fsg->luns[i];
			curlun->cdrom = !!mod_data.cdrom;
			curlun->ro = mod_data.ro[i] ||mod_data.cdrom;
			curlun->autorun = mod_data.autorun[i];
			curlun->dev.release = lun_release;
			curlun->dev.parent = &gadget->dev;
			curlun->dev.driver = &fsg_driver.driver;
			dev_set_drvdata(&curlun->dev, fsg);
			dev_set_name(&curlun->dev,"%s-lun%d",dev_name(&gadget->dev), i);
		}else if(mod_data.cdrom==1){
			curlun = &fsg->luns[i];
			if(i==fsg->nluns-1){ // bq:default the last lun is cdrom udisk 
				curlun->cdrom = !!mod_data.cdrom;
				curlun->ro = mod_data.ro[i] ||mod_data.cdrom;
				curlun->autorun = mod_data.autorun[i];
			}else {
				curlun->ro = mod_data.ro[i];
			}
			curlun->dev.release = lun_release;
			curlun->dev.parent = &gadget->dev;
			curlun->dev.driver = &fsg_driver.driver;
			dev_set_drvdata(&curlun->dev, fsg);
			dev_set_name(&curlun->dev,"%s-lun%d",dev_name(&gadget->dev), i);
		}
		if ((rc = device_register(&curlun->dev)) != 0) {
			fsg_err("failed to register LUN%d: %d\n", i, rc);
			goto out;
		}
		
		/*for /sysfs */
		if ((rc = device_create_file(&curlun->dev,
					&dev_attr_ro)) != 0 ||
				(rc = device_create_file(&curlun->dev,
					&dev_attr_file)) != 0 ||
				(rc = device_create_file(&curlun->dev,
					&dev_attr_iswritten)) != 0 ||
				(rc = device_create_file(&curlun->dev,
					&dev_attr_ustate)) != 0) {
			device_unregister(&curlun->dev);
			goto out;
		}
		
		curlun->registered = 1;
		kref_get(&fsg->ref);

		if (mod_data.file[i] && *mod_data.file[i]) {
			if ((rc = open_backing_file(curlun,
					mod_data.file[i])) != 0){
				curlun->media_present = 0;
				fsg_err("failed to open device\n");
			}else
				curlun->media_present = 1;	
		} else if (!mod_data.removable) {
			fsg_err("no file given for LUN%d\n", i);
			rc = -EINVAL;
			goto out;
		}	
	}	

	/*load config data from linuxtool*/
	autoconfig_usb_info(fsg,usb_configname);
	
	/* Find all the endpoints we will use */
	usb_ep_autoconfig_reset(gadget);
	ep = usb_ep_autoconfig(gadget, &fs_bulk_in_desc);
	if (!ep)
		goto autoconf_fail;
	ep->driver_data = fsg;		// claim the endpoint
	fsg->bulk_in = ep;
	
	ep = usb_ep_autoconfig(gadget, &fs_bulk_out_desc);
	if (!ep)
		goto autoconf_fail;
	ep->driver_data = fsg;		// claim the endpoint
	fsg->bulk_out = ep;

	/*have 2 eps for bulk only*/
	i = 2;
	device_desc.bMaxPacketSize0 = fsg->ep0->maxpacket;
	device_desc.idVendor = cpu_to_le16(mod_data.vendor);
	device_desc.idProduct = cpu_to_le16(mod_data.product);
	device_desc.bcdDevice = cpu_to_le16(mod_data.release);

	intf_desc.bNumEndpoints = i;
	intf_desc.bInterfaceSubClass = mod_data.protocol_type;
	intf_desc.bInterfaceProtocol = mod_data.transport_type;
	fs_function[i + FS_FUNCTION_PRE_EP_ENTRIES] = NULL;
	
#ifdef   CONFIG_USB_GADGET_DUALSPEED
	hs_function[i + HS_FUNCTION_PRE_EP_ENTRIES] = NULL;
	/* Assume ep0 uses the same maxpacket value for both speeds */
	dev_qualifier.bMaxPacketSize0 = fsg->ep0->maxpacket;
	/* Assume endpoint addresses are the same for both speeds */
	hs_bulk_in_desc.bEndpointAddress =fs_bulk_in_desc.bEndpointAddress;
	hs_bulk_out_desc.bEndpointAddress =fs_bulk_out_desc.bEndpointAddress;
#endif

	if (gadget_is_otg(gadget))
		otg_desc.bmAttributes |= USB_OTG_HNP;

	rc = -ENOMEM;
	
	/* Allocate the request and buffer for endpoint 0 */
	fsg->ep0req = req = usb_ep_alloc_request(fsg->ep0, GFP_KERNEL);
	if (!req)
		goto out;
	req->buf = kzalloc(EP0_BUFSIZE, GFP_KERNEL);
	if (!req->buf)	
		goto out;
	req->complete = ep0_complete;

	
	/* Allocate the data buffers */
	for (i = 0; i < NUM_BUFFERS; ++i) {
		struct fsg_buffhd	*bh = &fsg->buffhds[i];
		/* Allocate for the bulk-in endpoint.  We assume that
		 * the buffer will also work with the bulk-out (and
		 * interrupt-in) endpoint. */
	//	bh->buf = __gkmalloc(mod_data.buflen, GFP_KERNEL);
		bh->buf = (void *)__get_free_pages(GFP_KERNEL,mod_data.buflen);
		if (!bh->buf){
			fsg_dbg("%s:alloc failed\n",__FUNCTION__);
			goto out;		
		}
		fsg_info("%s:alloc pages sucess ,addr :%x\n",__FUNCTION__,(u32)bh->buf);
		bh->next = bh + 1;
	}
	
	fsg->buffhds[NUM_BUFFERS - 1].next = &fsg->buffhds[0];
	/* This should reflect the actual gadget power source */
	usb_gadget_set_selfpowered(gadget);
#if 0
	/*liucan:set default lun-name*/
	//for(i =0;i<MAX_LUNS;i++){
	for(i =0;i<mod_data.luns;i++){
		//printk("[%s %d]-----lunname:%s\n",__func__,__LINE__,default_lunname[i]);
		strcpy(mod_data.inquiry_products[i],default_lunname[i]);
	}
#endif
	fsg->thread_task = kthread_create(fsg_main_thread, fsg,
			"mass-storage-gadget");
	if (IS_ERR(fsg->thread_task)) {
		rc = PTR_ERR(fsg->thread_task);
		goto out;
	}

	fsg_info(DRIVER_DESC ", version: " DRIVER_VERSION "\n");
	fsg_info("Number of LUNs=%d\n", fsg->nluns);
	
	pathbuf = kmalloc(PATH_MAX, GFP_KERNEL);
	for (i = 0; i < fsg->nluns; ++i) {
		curlun = &fsg->luns[i];
		if (backing_file_is_open(curlun)) {
			p = NULL;
			if (pathbuf) {
				p = d_path(&curlun->filp->f_path,
					   pathbuf, PATH_MAX);
				if (IS_ERR(p))
					p = NULL;
			}
			fsg_info("ro=%d, file: %s\n",curlun->ro, (p ? p : "(error)"));
		}
	}
	kfree(pathbuf);
	
	fsync_all(fsg);
	/*register ustatus timer*/
	init_timer(&fsg->status_t);
	fsg->status_t.expires = 0;
	fsg->status_t.function = (void*)handle_ustatus;
	add_timer(&fsg->status_t);
	
	set_bit(REGISTERED, &fsg->atomic_bitflags);
	/* Tell the thread to start working */
	wake_up_process(fsg->thread_task);
	return 0;
	
autoconf_fail:
	fsg_err( "unable to autoconfigure all endpoints\n");
	rc = -ENOTSUPP;

out:
	fsg_err( "bind failed,%d \n",rc);
	fsg->state = FSG_STATE_TERMINATED;	// The thread is dead
	fsg_unbind(gadget);
	close_all_backing_files(fsg);
	return rc;
}


/*-------------------------------------------------------------------------*/

static void fsg_suspend(struct usb_gadget *gadget)
{
	struct fsg_dev		*fsg = get_gadget_data(gadget);
	fsg_dbg( "suspend\n");
	fsg->status = set_sub_status(fsg->status,FSG_SUB_STATUS_SUSPEND);
	set_bit(SUSPENDED, &fsg->atomic_bitflags);
}

static void fsg_resume(struct usb_gadget *gadget)
{
	struct fsg_dev		*fsg = get_gadget_data(gadget);
	fsg_dbg( "resume\n");
	fsg->status = set_sub_status(fsg->status,FSG_SUB_STATUS_IDLE);
	clear_bit(SUSPENDED, &fsg->atomic_bitflags);
}


/*-------------------------------------------------------------------------*/
static struct usb_gadget_driver		fsg_driver = {
#ifdef CONFIG_USB_GADGET_DUALSPEED
	.speed		= USB_SPEED_HIGH,
#else
	.speed		= USB_SPEED_FULL,
#endif
	.function	= (char *) longname,
	.bind		= fsg_bind,
	.unbind		= fsg_unbind,
	.disconnect	= fsg_disconnect,
	.setup		= fsg_setup,
	
	.suspend		= fsg_suspend,
	.resume		= fsg_resume,

	.driver		= {
		.name		= (char *) shortname,
		.owner		= THIS_MODULE,
		
		// .release = ...
		// .suspend = ...
		// .resume = ...
	},
};


static int __init fsg_alloc(void)
{
	struct fsg_dev		*fsg;
	
	fsg = kzalloc(sizeof *fsg, GFP_KERNEL);
	if (!fsg){
		printk(KERN_ALERT "alloc failed\n");
		return -ENOMEM;
	}
	
	spin_lock_init(&fsg->lock);
	init_rwsem(&fsg->filesem);
	kref_init(&fsg->ref);
	init_completion(&fsg->thread_notifier);
	the_fsg = fsg;
	fsg_dbg("fsg alloc sucess!\n");
	return 0;
}


static int __init fsg_init(void)
{
	int		rc;
	struct fsg_dev	*fsg;

	if ((rc = fsg_alloc()) != 0)
		return rc;
	
	fsg = the_fsg;	
	if ((rc = usb_gadget_register_driver(&fsg_driver)) != 0){
		kref_put(&fsg->ref, fsg_release);
		goto error;
	}
	
	rc = am_register_sysmsg(media_change_event,CARD_MSG_EN,(void*)fsg);
	if(rc != 0){
		fsg_err("media change handler error\n");
		goto error;
	}
	return 0;
	
error:
	kfree(fsg);
	the_fsg = NULL;
	return rc;

}
module_init(fsg_init);


static void __exit fsg_cleanup(void)
{
	struct fsg_dev	*fsg = the_fsg;

	/* Unregister the driver iff the thread hasn't already done so */
	if (test_and_clear_bit(REGISTERED, &fsg->atomic_bitflags))
		usb_gadget_unregister_driver(&fsg_driver);

	/* Wait for the thread to finish up */
	wait_for_completion(&fsg->thread_notifier);

	if( am_unregister_sysmsg(media_change_event) < 0)
		fsg_err("unregister sysmsg failedr\n");
	
	close_all_backing_files(fsg);
	kref_put(&fsg->ref, fsg_release);
}
module_exit(fsg_cleanup);
