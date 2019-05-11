/*
 * Actions ATJ213x 's AOTG  USB device controllers
 *
 */
#undef	VERBOSE_DEBUG
#undef	PACKET_TRACE
	 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/otg.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/gpio.h>

#include "actions_regs.h"
#include "aotg_regs_next.h"
#include "aotg_io_next.h"
#include "am7x_udc.h"
#include "dma.h"

#define	DRIVER_VERSION   	"Version1.0-March-2010"
#define	DRIVER_DESC			"Actions USB Device Controller Driver"

#define DMA_MODE_IN			(0x01bc0084)
#define DMA_MODE_OUT		(0x008401bc)

/*-----------------------------------------------------------------------*/

static const char driver_name [] = "aotg-usb-am7x_next";
static const char ep0name [] = "ep0";
static unsigned use_dma  = 0;
/*-----------------------------------------------------------------------*/

module_param_named(use_dma, use_dma, bool, S_IRUGO);
MODULE_PARM_DESC(use_dma, "true to use dma for data transfer");

static void udc_enable(struct aotg_udc *dev,int isenable);
static void aotg_ep_reset(u8 ep_mask,u8 type_mask);
static void aotg_ep_setup(struct aotg_ep * ep, u8 type,u8 buftype);
static int aotg_udc_event_notify(void *target,int event);
static void done(struct aotg_ep *ep, struct aotg_request *req, int status);
static int write_packet (struct aotg_ep *ep, struct aotg_request *req,unsigned max);
static int write_fifo(struct aotg_ep *ep, struct aotg_request *req);
static int read_fifo(struct aotg_ep *ep, struct aotg_request *req);
static int aotg_udc_irq(int irqnum,void *_udc );
static u32  aotg_udc_kick_dma(struct aotg_ep * ep, struct aotg_request * req);
static void aotg_udc_cancel_dma(struct aotg_ep * ep);

/*----------------------------------------------------------------------*/

#ifdef CONFIG_USB_GADGET_DEBUG_FILES
#include <linux/seq_file.h>
static const char debug_filename[] = "driver/udc1";
static void proc_ep_show(struct seq_file *s, struct aotg_ep *ep)
{
	struct aotg_request	*req;
	struct aotg_udc		*udc;
	unsigned long	flags;
	unsigned char tmp;
	const char  *type[]  = {"","iso","bulk","int"};
	const char  *buf[]   = {"single","double","triple","quad"};
	udc = s->private;
	seq_printf(s, "\n%s, maxpacket %d %s\n",
			ep->ep.name,
			ep->ep.maxpacket,
			ep->stopped ? " stopped" : "");
	
	if(!strcmp(ep->ep.name,ep0name)){
		const char *ep0_string[]= {"wait_fo_setup","data_in","data_out","trans_end","stall"};
		seq_printf(s,"\tep0state:%s\n",ep0_string[udc->ep0state]);
		return;
	}
	
	spin_lock_irqsave(&udc->lock,flags);
	tmp = aotg_readb(ep->reg_udccon);
	seq_printf(s,"\tudccon(%lx):%02x\n",ep->reg_udccon,tmp);
	seq_printf(s, "\tbuf_conf:%s,ep_type:%s,stall:%d,valid:%d\n",
			buf[tmp&0x03],type[(tmp>>2)&0x03],
			(tmp>>6)&0x01,(tmp>>7)&0x01);
	
	seq_printf(s,"\tRegcs(0x%lx):0x%x,FifoAddr(0x%lx): 0x%x,udcbc(0x%lx):%x\n",
				ep->reg_udccs,aotg_readb(ep->reg_udccs),
				ep->reg_udcfifo,aotg_readb(ep->reg_udcfifo),
				ep->reg_udcbc,aotg_readb(ep->reg_udcbc));
	
	spin_unlock_irqrestore(&udc->lock,flags);
	if (list_empty (&ep->queue))
		seq_printf(s, "\t(queue empty)\n");
	else{
		list_for_each_entry (req, &ep->queue, queue) {		
			seq_printf(s, "\treq %p len %d/%d buf %p\n",
				&req->req,
				req->req.actual,
				req->req.length, 
				req->req.buf);
		}	
	}
}

static void proc_dma_show(struct seq_file *s)
{
	struct aotg_udc *udc = s->private;
	seq_printf(s,"\ndma_channel:%d\n",udc->dma_channel);
	seq_printf(s,"\tdma irq:%d\n",am_get_dma_irq(udc->dma_channel,1,0));
}


static int proc_udc_show(struct seq_file *s, void *unused)
{
	struct aotg_udc	*udc = s->private;
	struct aotg_ep	*ep;
	char *state_string[] = {"unknown","idle","active","suspend","disable"};

	seq_printf(s, "\n%s: version %s\n", driver_name, DRIVER_VERSION);
	seq_printf(s, "\ngadget:%s\n", udc->driver ? udc->driver->driver.name : "(none)");
	seq_printf(s, "\nudcstate:%s\n",state_string[udc->state]);
	
	/* don't access registers when interface isn't clocked */
	seq_printf(s, "\nFSM: 0x%02x,OTGSTATUS:0x%02x USBESTA:0x%02x\n", 
		aotg_readb(OTGSTATE) & 0x0f,aotg_readb(OTGSTATUS),
		aotg_readb(USBESTA));

	seq_printf(s, "\nOTGIEN: 0x%02x: USBIEN:0x%02x\n",
		aotg_readb(OTGIEN),aotg_readb(USBIEN));

	seq_printf(s,"\n***************************\n");
	if (udc->enabled) {
		proc_ep_show(s, &udc->ep[0]);
		list_for_each_entry (ep, &udc->gadget.ep_list, ep.ep_list) {
			if (ep->desc)
				proc_ep_show(s, ep);
		}
	}
	seq_printf(s,"\n********************************\n");
	if(use_dma)
		proc_dma_show(s);
	return 0;
}


static int proc_udc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_udc_show, PDE(inode)->data);
}

static const struct file_operations proc_ops = {
	.owner		= THIS_MODULE,
	.open		= proc_udc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};


static void create_debug_file(struct aotg_udc *udc)
{
	udc->pde = proc_create_data(debug_filename, 0, NULL, &proc_ops, udc);
}

static void remove_debug_file(struct aotg_udc *udc)
{
	if (udc->pde)
		remove_proc_entry(debug_filename, NULL);
}

#else
static inline void create_debug_file(struct aotg_udc *udc) {}
static inline void remove_debug_file(struct aotg_udc *udc) {}
#endif


/*----------------------------------------------------*/
#ifdef  DEBUG_UDC
static void dump_ep_regs(struct aotg_ep *ep)
{
	int is_in = ((ep->bEndpointAddress & USB_DIR_IN) != 0)?1:0;

	unsigned long ien = is_in ? IN07IEN : OUT07IEN ;
	unsigned long irq = is_in ? IN07IRQ : OUT07IRQ;
	
	DMSG_UDC("*******%s:******\n",ep->ep.name);
	DMSG_UDC("epcon=0x%x\n",aotg_readb(ep->reg_udccon));
	DMSG_UDC("epcs=0x%x\n",aotg_readb(ep->reg_udccs));
	DMSG_UDC("epbc=0x%x\n",aotg_readw(ep->reg_udcbc));
	DMSG_UDC("IE=0x%x\n",aotg_readb(ien));
	DMSG_UDC("IRQ=0x%x\n",aotg_readb(irq));	
}
#else
static inline void dump_ep_regs(struct aotg_ep *ep){}
#endif


#ifdef  CHECK_CSW
static int is_csw(struct aotg_request *req)
{
	int i,result = 1;
	u8 * ptr = req->req.buf;
	unsigned char csw_tag[4] = {0x55,0x53,0x42,0x53};
	if(req->req.length == 13){
		for(i = 0 ;i < 4; i++){
			if(csw_tag[i] != *ptr++){
				result = 0;
				break;
			}		
		}
	}
	return result;
}
#else
static inline int is_csw(struct aotg_request *req){ return 0; }
#endif



/*--------------------------------------------------*/
static void ep_irq_disable(u8 mask)
{
	u8  is_in = mask & USB_UDC_IN_MASK;
	u8 ep_num = mask & 0x0f;
	
	DMSG_XFERS("<PIO_IRQ_DISABLE> ep mask %02x pio irq disable\n",mask);
	if(is_in)
		aotg_clearbits(1<<ep_num,IN07IEN);
	else
		aotg_clearbits(1<<ep_num,OUT07IEN);
}

static void ep_irq_enable(u8 mask)
{
	u8  is_in = mask & USB_UDC_IN_MASK;
	u8 ep_num = mask & 0x0f;

	DMSG_XFERS("<PIO_IRQ_ENABLE>ep mask %02x pio irq enable\n",mask);
	if(is_in)
		aotg_setbits(1<<ep_num,IN07IEN);
	else
		aotg_setbits(1<<ep_num,OUT07IEN);
}


static void ep_irq_clear(u8 mask)
{
	u8  is_in = mask & USB_UDC_IN_MASK;
	u8 ep_num = mask & 0x0f;

	DMSG_XFERS("<PIO_IRQ_CLEAR>ep mask %02x pio irq clear\n",mask);
	if(is_in)
		aotg_setbits(1<<ep_num,IN07IRQ);
	else
		aotg_setbits(1<<ep_num,OUT07IRQ);
}

static int pullup(struct aotg_udc  *udc, int  is_active)
{
	is_active =( is_active && udc->softconnect) ? 1 : 0;
	DMSG_UDC("<PULL_UP> %s\n",is_active ? "active" : "inactive");
	udc_enable(udc,is_active);
	return 0;
}

static void done(struct aotg_ep *ep, struct aotg_request *req, int status)
{
	unsigned stopped = ep->stopped;
	list_del_init(&req->queue);
	if (req->req.status == -EINPROGRESS)
		req->req.status = status;
	else
		status = req->req.status;

	if (status && status != -ESHUTDOWN)
		DMSG_XFERS("<REQ RELEASE>complete %s req %p stat %d len %u/%u\n",
			ep->ep.name, &req->req, status,req->req.actual, req->req.length);
	
	/* don't modify queue heads during completion callback */
	ep->stopped = 1;
	req->req.complete(&ep->ep, &req->req);
	ep->stopped = stopped;
}

static void nuke(struct aotg_ep *ep, int status)
{
	struct aotg_request *req;
	//struct aotg_udc *udc =(struct aotg_udc *)ep->dev;
	u32 dma_channel = ep->dma_nr;
	
	DMSG_XFERS("<EP NUKE> %s is nuked with status  %d\n",ep->ep.name,status);  
	if(use_dma && dma_channel > 0){
	 	if(ep->dma_working && ! ep->stopped){
			INFO("cancel ep used dma\n");
			aotg_udc_cancel_dma(ep);
	 	}
	}
	
	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next,struct aotg_request,queue);
		done(ep, req, status);
	}
	
	if (ep->desc)
		ep_irq_disable (ep->mask);
}


static void aotg_set_fifoauto(struct aotg_ep * ep,int is_on)
{
	u8 val = is_on  ? ( FIFOAUTO |ep->mask ) : ep->mask;
	aotg_writeb(val,FIFOCTRL);			
}

/*
  *	EP0 related operations
  */
static inline int write_ep0_packet(struct aotg_request *req, unsigned max)
{
	u32		*buf;
	unsigned	length, count;
	unsigned long addr = EP0INDAT0;

	/* how big will this packet be? */
	/*fixme: the buf may not align,will cause cpu gan*/
	buf = (u32 *)(req->req.buf + req->req.actual);
	length = min(req->req.length - req->req.actual, max);
	req->req.actual += length;

	count = length>>2; 
	if(length & 0x03)
		count ++;
	
	/*wirte in DWORD*/
	while (count--) {
		__aotg_writel(*buf++,addr);
		addr += 4;	
	}
	
	/*arm IN EP0 for the next IN transaction*/
	aotg_writeb(length,IN0BC);
	return length;
}



static int read_ep0_fifo (struct aotg_ep *ep, struct aotg_request *req)
{
	u8		*buf;
	unsigned	bufferspace,count,length;
	unsigned long     addr;

	if(aotg_readb(EP0CS) & EP0CS_OUTBSY)   /*data is not ready*/
		return 0; 
	else {
		/*fifo can be accessed validly*/
		buf = req->req.buf + req->req.actual;
		bufferspace = req->req.length - req->req.actual;

		length = count = aotg_readb(OUT0BC);
		addr = EP0OUTDAT0;
		while(count--) {		
			if (bufferspace == 0) {	/* driver's buffer smaller than data host sent.discard the extra data.*/	
				if (req->req.status != -EOVERFLOW)
					DMSG_CTRL("%s overflow\n",ep->ep.name);
				req->req.status = -EOVERFLOW;
				break;
			} else {
				*buf++ = aotg_readb(addr);	
				req->req.actual++;
				bufferspace--;
				addr++;
		 	  }
		}
	}
	
	DMSG_CTRL("ep0out %d bytes %s %d left %p\n", length,
		 (req->req.actual >= req->req.length)? "/L" : "",
		 req->req.length - req->req.actual, req);
	
	if(req->req.actual  >= req->req.length)
		return 1;

	return 0;	
}


static int write_ep0_fifo (struct aotg_ep *ep, struct aotg_request *req)
{
	unsigned	count;
	int		is_last;

	count = write_ep0_packet(req,EP0_PACKET_SIZE);
	/*last packet must be a short packet or zlp*/
	if(count != EP0_PACKET_SIZE)
		is_last = 1;
	else {
		if ((req->req.length != req->req.actual) ||req->req.zero)
			is_last = 0;
		else
			is_last = 1;
	}
	
	DMSG_CTRL("ep0in %d bytes %s %d left %p\n", count,
		 is_last? "/L" : "",req->req.length - req->req.actual, req);
	
	return is_last;
}



static inline void handle_status(void)
{
	aotg_setbits(EP0CS_HSNAK,EP0CS);
	DMSG_CTRL("<CTRL>ACK the status stage\n");
}


union setup{
	struct usb_ctrlrequest	r;
	u8					raw [8];
} ;

static void handle_setup(struct aotg_udc *udc)
{
	struct aotg_ep *ep0,*ep;
	union setup u;
	int i,status = 0;
	int reciptype;
	unsigned long  addr;
	unsigned long  ackval = 0;

	ep0 = &udc->ep[0];
	addr= SETUPDAT0;
	
	if(udc->ep0state != EP0_WAIT_FOR_SETUP) {
		nuke(ep0, -ESHUTDOWN);
		udc->ep0state = EP0_WAIT_FOR_SETUP;
	}else{ 
		nuke (ep0, -EPROTO);
	}
	
	/*read standard 8bytes setup data*/
	for( i = 0;i<8;i++) 
		u.raw[i] = aotg_readb(addr++);
	
#define w_index		le16_to_cpu(u.r.wIndex)
#define w_value		le16_to_cpu(u.r.wValue)
#define w_length		le16_to_cpu(u.r.wLength)

	DMSG_CTRL( "<CTRL> SETUP %02x.%02x v%04x i%04x l%04x\n",
				u.r.bRequestType, u.r.bRequest,w_value,w_index,w_length);

	/* Delegate almost all control requests to the gadget driver,
	 * except for a handful of ch9 status/feature requests that
	 * hardware doesn't autodecode and the gadget API hides.
	 */ 
	udc->req_config = 0;
	udc->req_pending = 1;
	ep0->stopped = 0;
	reciptype = (u.r.bRequestType & USB_RECIP_MASK);

	/*liucan: important !!special process for non-standard ctrl request*/
#if 1
	 /* USB_CDC_SEND_ENCAPSULATED_COMMAND */
	 if ((u.r.bRequestType == 0x21) && (u.r.bRequest == 0x00)) 
		 goto delegate;
	 
	 /* USB_CDC_GET_ENCAPSULATED_RESPONSE */
	 if ((u.r.bRequestType  == 0xa1) && (u.r.bRequest == 0x01)) 
		 goto delegate;
#endif	 
 
	switch (u.r.bRequest) {
		case USB_REQ_GET_STATUS:
			DMSG_CTRL("<CTRL>USB_REQ_GET_STATUS\n");
			if(reciptype ==  USB_RECIP_INTERFACE) {
				/*according to USB spec, this does nothing but return 0*/
				;
			}else if(reciptype == USB_RECIP_DEVICE) {
			/*return self powered and remote wakeup status
			* we are self powered ,so just check wakeup character 
			*/
				if(udc->rwk) 
					ackval = 0x03;
				else
					ackval = 0x01;
			}else if(reciptype == USB_RECIP_ENDPOINT) {
				for(i=0;i<AOTG_UDC_NUM_ENDPOINTS;i++){
					ep = &udc->ep[i];
					if(ep->bEndpointAddress == u.r.wIndex)
						break;
				}

				DMSG_CTRL("GET STATUS,name:%s:ep addr:%x\n",
					ep->ep.name, ep->bEndpointAddress);
				
				if((i ==AOTG_UDC_NUM_ENDPOINTS)  || u.r.wLength > 2 ) 
					goto stall;
				
				if(ep->bEndpointAddress == 0)
					ackval |= ((aotg_readb(EP0CS) & EP0CS_STALL)>> 1);
				else
					ackval |=  ((aotg_readb(ep->reg_udccon) & EPCON_STALL) >> 6);
			}else 
				goto stall;
			
			/*back the status*/
			/*FIXME not check whether ep0 fifo is empty*/
			aotg_writel(ackval,EP0INDAT0);
			aotg_writeb(2,IN0BC);
			udc->ep0state = EP0_END_XFER;
			return;
			
		case USB_REQ_CLEAR_FEATURE:
			DMSG_CTRL("<CTRL> USB_REQ_CLEAR_FEATURE\n");
			if((u.r.bRequestType == USB_RECIP_DEVICE) && (u.r.wValue == USB_DEVICE_REMOTE_WAKEUP)){
				DMSG_CTRL("<CTRL> clear remote wakeup feature\n");
				/*clear the remote wakeup character*/
				udc->rwk = 0;   
			}
			else if((u.r.bRequestType == USB_RECIP_ENDPOINT) && (u.r.wValue == USB_ENDPOINT_HALT)) {			
				//ep_num 	= u.r.wIndex & 0x0f;
				//is_in		=(u.r.wIndex &USB_DIR_IN) ? 1: 0;

				for(i=0;i<AOTG_UDC_NUM_ENDPOINTS;i++){
					ep = &udc->ep[i];
					if(ep->bEndpointAddress == u.r.wIndex)
						break;
				}
				
				if((i ==AOTG_UDC_NUM_ENDPOINTS) || u.r.wLength > 2 ) 
					goto stall;
				
				if(ep->bEndpointAddress == 0)
					aotg_clearbits(EP0CS_STALL,EP0CS);	
				else{
					aotg_clearbits(EPCON_STALL,ep->reg_udccon);
					ep->stopped = 0;	
					/*reset the ep toggle*/
					aotg_ep_reset(ep->mask,ENDPRST_TOGRST); 
				}	
				DMSG_CTRL("<CTRL> clear %s halt feature\n",ep->ep.name);
			}else 
				goto stall;
			/*ACK the status stage*/
			handle_status();
			return;
		case  USB_REQ_SET_FEATURE:
			DMSG_CTRL("<CTRL> USB_REQ_SET_FEATURE\n");
			if((u.r.bRequestType == USB_RECIP_DEVICE)) {
				switch(u.r.wValue) {
				case USB_DEVICE_REMOTE_WAKEUP:
					udc->rwk = 1;   /*clear the remmote wakeup character*/
					break;
				 case USB_DEVICE_B_HNP_ENABLE:
				 	udc->gadget.b_hnp_enable = 1;
					set_b_hnp_en();
				 	break;
				 case USB_DEVICE_A_HNP_SUPPORT:
				 	udc->gadget.a_hnp_support = 1;
				 	break;
					  case USB_DEVICE_A_ALT_HNP_SUPPORT:
				  	udc->gadget.a_alt_hnp_support = 1;
				  	break;
				default:
					goto stall;
				}
			}else if((u.r.bRequestType == USB_RECIP_ENDPOINT) && (u.r.wValue == USB_ENDPOINT_HALT)) {
				//ep_num 	= u.r.wIndex & 0x0f;
				//is_in	 = (u.r.wIndex &USB_DIR_IN) ? 1: 0;

				for(i=0;i<AOTG_UDC_NUM_ENDPOINTS;i++){
					ep = &udc->ep[i];
					if(ep->bEndpointAddress == u.r.wIndex)
						break;
				}
		
				if((i ==AOTG_UDC_NUM_ENDPOINTS) || u.r.wLength > 2 ) 
					goto stall;
				
				if(ep->bEndpointAddress == 0) {
					aotg_setbits(EP0CS_STALL,EP0CS);
					udc->ep0state = EP0_STALL;
				}
				else
					aotg_setbits(EPCON_STALL,ep->reg_udccon);
			}else 
				goto stall;
			
			/*ACK the status stage*/
			handle_status();
			return;
		case USB_REQ_SET_ADDRESS:
			/* automatically reponse by hardware,so hide it to software*/
			DMSG_CTRL("<CTRL>USB_REQ_SET_ADDRESS\n");
			udc->req_pending = 0; 
			return;

		/*delegate sub request to the upper gadget driver*/		
		case USB_REQ_SET_INTERFACE:
			DMSG_CTRL("<CTRL>USB_REQ_SET_INTERFACE\n");
			udc->req_config = 1;
			if (w_length != 0)
				goto stall;
			goto delegate;
		case USB_REQ_SET_CONFIGURATION:
			DMSG_CTRL("<CTRL>USB_REQ_SET_CONFIGURATION\n");
			if (u.r.bRequestType == USB_RECIP_DEVICE) {
				if (w_length != 0)
					goto stall;
				udc->req_config = 1;
				if(w_value == 0) {/*enter address state and all endpoint should be disabled except for endpoint0*/
					DMSG_CTRL("<CTRL>disable all ep\n");
					for(i = 1;i < AOTG_UDC_NUM_ENDPOINTS-1; i++)
						aotg_clearbits(EPCON_VAL,(&udc->ep[i])->reg_udccon);
				}else {
					/*enter configured state*/
					DMSG_CTRL("<CTRL>enter configured state\n");
					for(i = 1;i < AOTG_UDC_NUM_ENDPOINTS-1; i++)
						aotg_setbits(EPCON_VAL,udc->ep[i].reg_udccon);
				}
			}else
				goto stall;
			/*delegate to the upper gadget driver*/ 
			break;
		default:
			/*delegate to the upper gadget driver*/
			break;
	}
	/* gadget drivers see class/vendor specific requests,
	 * {SET,GET}_{INTERFACE,DESCRIPTOR,CONFIGURATION},
	 * and more
	 * The gadget driver may return an error here,
	 * causing an immediate protocol stall.
	 *
	 * Else it must issue a response, either queueing a
	 * response buffer for the DATA stage, or halting ep0
	 * (causing a protocol stall, not a real halt).  A
	 * zero length buffer means no DATA stage.
	 *
	 * It's fine to issue that response after the setup()
	 * call returns.
	 */

delegate:			
	DMSG_CTRL("<CTRL>delegate\n");
	if (u.r.bRequestType & USB_DIR_IN)
		udc->ep0state = EP0_IN_DATA_PHASE;
	else
		udc->ep0state = EP0_OUT_DATA_PHASE;

	status = udc->driver->setup (&udc->gadget, &u.r); /*delegate*/
	if(status < 0) {
stall:
		DMSG_CTRL("<CTRL> req %02x.%02x protocol STALL, err  %d\n",
			u.r.bRequestType, u.r.bRequest, status);
		if(udc->req_config)
			DMSG_UDC("<CTRL>config change erro\n");
		aotg_setbits(EP0CS_STALL,EP0CS);
		udc->req_pending = 0;
		udc->ep0state = EP0_STALL;
	}
	return;			
}



static void handle_ep0(struct aotg_udc *_udc)
{
	struct aotg_udc * udc = _udc;
	struct aotg_ep		*ep0 = &udc->ep[0]; 
	struct aotg_request *req;

	if(unlikely(list_empty(&ep0->queue)))
		req = NULL;
	else
		req = list_entry(ep0->queue.next,struct aotg_request,queue);
	
	DMSG_CTRL("<CTRL>ep0 irq handler,state is %d,queue is %s\n",
		udc->ep0state,(req == NULL)?"empty":"not empty");
	
	switch (udc->ep0state) {
		case EP0_IN_DATA_PHASE:
			if(req) {
				if(write_ep0_fifo(ep0,req))
					udc->ep0state = EP0_END_XFER;
			}
			break;
		case EP0_END_XFER:
			/*ACK*/
			handle_status();
			/*cleanup*/
			udc->req_pending = 0;
			udc->ep0state = EP0_WAIT_FOR_SETUP;
			if(req) {
				done(ep0, req, 0);
				req = NULL;
			}
			break;
			
		case EP0_OUT_DATA_PHASE:
			if(req) {
				if(read_ep0_fifo(ep0,req)) {
					/*ACK*/
					handle_status();
					/*cleanup*/
					udc->req_pending = 0;
					udc->ep0state = EP0_WAIT_FOR_SETUP;
					done(ep0, req, 0);
					req = NULL;
				}else
					aotg_writeb(0,OUT0BC);  /*write OUT0BC with any value to enable next OUT transaction*/
			}else
				DMSG_CTRL("<CTRL>ep0out irq error,queue is empty but state is EP0_OUT_DATA_PHASE\n");/*never enter this branch*/
			break;
			
		case EP0_STALL:
			if(req) {
				done(ep0,req,-ESHUTDOWN);
				req = NULL;
			}
			break;
			
		default:
			DMSG_CTRL("<CTRL>ep0in irq error,odd state %d\n",udc->ep0state);

	}	
}


/* non-EP0 related operations*/
static int write_packet (struct aotg_ep *ep, struct aotg_request *req,unsigned max)
{
	u32		*buf;
	u16	length,count;

	/*FIXME-liucan:whether the buffer is 32bit aligned ?*/
	buf = (u32 *)(req->req.buf + req->req.actual);
	length = min(req->req.length - req->req.actual, max);
	req->req.actual += length;

	/*wirte in DWORD*/
	count = length>>2;
	if(length & 0x03)
		count ++;

	while (count--)
		aotg_writel(*buf++,ep->reg_udcfifo);
	
	/*write count to counter register*/
	aotg_writew(length,ep->reg_udcbc);
	
	/*arm IN EP for the next IN transaction*/
	aotg_writeb(0,ep->reg_udccs); 		
	return length;
}


static int write_fifo(struct aotg_ep *ep, struct aotg_request *req)
{
 	 /*list two variables here  to tell whether they are equal,just for debugging*/
	unsigned  descmax; /*maxpacket assigned by the upper driver*/
	unsigned  max;   /*maxpacket assigned by udc driver*/
	
	descmax = ep->desc->wMaxPacketSize;
	max = ep->maxpacket;
	while (!(aotg_readb(ep->reg_udccs) & EPCS_BUSY)){
		unsigned	count;
		int		is_last;
		count = write_packet(ep,req,max);
		/* last packet is usually short (or a zlp) */
		if (count != max)
			is_last =  1;
		else {
			if (req->req.length != req->req.actual
					|| req->req.zero)
				is_last = 0;
			else
				is_last = 1;
		}
		
		/* requests complete when all IN data is in the FIFO */
		if (is_last) {
			done (ep, req, 0);
			if (list_empty(&ep->queue)){
				DMSG_BULK("%s queue is empty now,do nothing\n",ep->ep.name);
				ep_irq_disable (ep->mask);		
			}
			DMSG_BULK("data transferred? reg_udccs %02x\n",aotg_readb(ep->reg_udccs));
			return 1;
		}
	}
	
	return 0;	
}


static int read_fifo(struct aotg_ep *ep, struct aotg_request *req)
{
	u8           *bytebuf;
	u32		*buf;
	unsigned	 bufferspace,count,remain,length,is_short;
	unsigned long fifoaddr;
	int i;
	
        while(!(aotg_readb(ep->reg_udccs) & EPCS_BUSY)){
		buf = (u32 *)(req->req.buf + req->req.actual);
		bufferspace = req->req.length - req->req.actual;
		count = length = aotg_readw(ep->reg_udcbc);

		BUG_ON(length > bufferspace);
		
		is_short = (length < ep->maxpacket);
		remain = count &0x03;
		count  = count>>2; 

		/*read in DWORD*/
		while(count--) {  
			*buf++ =  __aotg_readl(ep->reg_udcfifo);	
			req->req.actual += 4;
		}
		
		/*read in BYTE*/
		if(remain != 0) {  
			bytebuf = req->req.buf + req->req.actual;
			fifoaddr = ep->reg_udcfifo;
			for(i = 0;i < (int)remain;i++) {
				*bytebuf++ = aotg_readb(fifoaddr++);
				req->req.actual++;
			}
		}

		/*arm ep for next OUT transaction*/
		aotg_writeb(0,ep->reg_udccs); 
		
		DMSG_BULK("%s read  %d bytes%s %s actual %d bytes claim %d/bytes\n",
			ep->ep.name,length,is_short ? "/S" : "",
			(req->req.length == req->req.actual) ? "/L" : "",req->req.actual, req->req.length);
		
		if(is_short ||req->req.length == req->req.actual) {
			done (ep, req, 0);
			if (list_empty(&ep->queue)){
				DMSG_BULK("%s queue is empty now,do nothing\n",ep->ep.name);
				ep_irq_disable (ep->mask);
			}
			return 1;
		}
        }
	return 0;
}

static void handle_ep(struct aotg_udc * dev, u8 index)
{
	struct aotg_udc *udc =dev;
	struct aotg_ep *ep;
	struct aotg_request *req;
	u8  completed;
	int is_in ;
	
	ep = &udc->ep[index];
	is_in = ep->bEndpointAddress & USB_DIR_IN;
	
	do {
		completed = 0;
		if (!list_empty(&ep->queue))
			req = list_entry(ep->queue.next,struct aotg_request, queue);
		else {
			DMSG_BULK("<BULK>%s queue is empty,do nothing\n",
				ep->ep.name);
			req = NULL;
			return;
		}

		if(is_in)
			completed = write_fifo(ep,req);	
		else{
			if(!(aotg_readb(ep->reg_udccs) & EPCS_BUSY)){
				DMSG_BULK("<BULK>%s queue and fifo are all not empty,reading, bytes %d\n",ep->ep.name,req->req.length);
				completed = read_fifo(ep,req);	
			}
		}
	}while(completed);
}


/* ---------------------------------------------------------------------------
 * 	endpoint related parts of the api to the usb controller hardware,
 *	used by gadget driver; and the inner talker-to-hardware core.
 * ---------------------------------------------------------------------------
 */
static int aotg_ep_enable (struct usb_ep *_ep,
		const struct usb_endpoint_descriptor *desc)
{
	struct aotg_ep		*ep;
	struct aotg_udc	*udc;
		
	ep = container_of(_ep,struct aotg_ep,ep);
	/*sanity check*/
	if (!_ep || !desc || ep->desc || _ep->name == ep0name
		|| desc->bDescriptorType != USB_DT_ENDPOINT
		|| ep->bEndpointAddress != desc->bEndpointAddress
		|| ep->maxpacket < desc->wMaxPacketSize) {
		ERR("%s, bad ep or descriptor\n", _ep->name);
		return -EINVAL;
	}

	/*not support ISO yet*/
	if (desc->bmAttributes == USB_ENDPOINT_XFER_ISOC) {
		ERR("%s, ISO nyet\n", _ep->name);
		return -EDOM;
	}
	
	/* xfer types must match, except that interrupt ~= bulk */
	if (ep->bmAttributes != desc->bmAttributes
		&& ep->bmAttributes != USB_ENDPOINT_XFER_BULK
		&& desc->bmAttributes != USB_ENDPOINT_XFER_INT) {
		ERR("%s: %s type mismatch\n", __FUNCTION__, _ep->name);
		return -EINVAL;
	}

	if ((desc->bmAttributes == USB_ENDPOINT_XFER_BULK
		&& desc->wMaxPacketSize != ep->maxpacket)
		|| !desc->wMaxPacketSize) {
		ERR("%s: bad %s maxpacket\n", __FUNCTION__, _ep->name);
		return -ERANGE;
	}

	udc = ep->dev;
	if (!udc->driver || udc->gadget.speed == USB_SPEED_UNKNOWN) {
		ERR("%s: bogus device state\n", __FUNCTION__);		
		return -ESHUTDOWN;
	}

	ep->desc = desc;
	ep->stopped = 0;
	ep->ep.maxpacket = desc->wMaxPacketSize;
	ep->dma_working = 0;
	aotg_setbits(EPCON_VAL,ep->reg_udccon);
    	aotg_ep_reset(ep->mask,ENDPRST_TOGRST |ENDPRST_FIFOREST); 	
	
	DMSG_UDC("<EP ENABLE>%s enable,reg_udccon is %02x\n",
			_ep->name,aotg_readb(ep->reg_udccon));
	
	return 0;
}


static int aotg_ep_disable (struct usb_ep *_ep)
{
	struct aotg_ep  *ep;
	struct aotg_udc *udc;
	unsigned long flags;

	ep = container_of(_ep,struct aotg_ep,ep);
	if (!_ep || !ep->desc) {
		DMSG_UDC("<EP DISABLE> %s not enabled\n",
			_ep ? ep->ep.name : NULL);
		return -EINVAL;
	}
	udc = ep->dev;

	local_irq_save(flags);
	nuke (ep, -ESHUTDOWN);
	aotg_clearbits(EPCON_VAL,ep->reg_udccon);/*disable the ep*/
	ep_irq_disable(ep->mask);
	ep->desc = NULL;
	ep->stopped = 1;
	ep->dma_working = 0;
	local_irq_restore(flags);

	DMSG_UDC("<EP DISABLE>%s disable\n",_ep->name);
	return 0;
}


static struct usb_request *aotg_ep_alloc_request(struct usb_ep *_ep, gfp_t gfp_flags)
{
	struct aotg_request *req =  kzalloc(sizeof (struct aotg_request), gfp_flags);
	if (!req)
		return NULL;
	
	INIT_LIST_HEAD(&req->queue);
	return &req->req;
}


static void aotg_ep_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct aotg_request *req;
	
	req = container_of(_req, struct aotg_request, req);
	BUG_ON(!list_empty(&req->queue));
	
	kfree(req);
}



static int
aotg_ep_queue(struct usb_ep *_ep, struct usb_request *_req,gfp_t gfp_flags)
{
	struct aotg_ep 		*ep;
	struct aotg_udc 	*udc;
	struct aotg_request	*req;
	unsigned long flags;
	int retval;
	
       /*sanity check*/
	req = container_of(_req, struct aotg_request, req);
	if (!_req || !_req->complete || !_req->buf
			|| !list_empty(&req->queue)) {
		DMSG_XFERS("<EP QUEUE>bad params\n");
		return -EINVAL;
	}

	ep = container_of(_ep, struct aotg_ep, ep);
	if (!_ep || (!ep->desc && ep->ep.name != ep0name)) {
		DMSG_XFERS("<EP QUEUE> bad ep\n");
		return -EINVAL;
	}

	udc = ep->dev;
	if (!udc->driver || udc->gadget.speed == USB_SPEED_UNKNOWN) {
		DMSG_XFERS("<EP QUEUE> bogus device state\n");
		return -ESHUTDOWN;
	}

	/*not support ISO now*/
	if(ep->bmAttributes ==  USB_ENDPOINT_XFER_ISOC)
		return -EOPNOTSUPP;

	if(is_csw(req)){
		DMSG_XFERS("<EP QUEUE>%s queue req %p, len %d buf %p,ep = %p,stopped = %d\n",
	    		 _ep->name, _req, _req->length, _req->buf,ep,ep->stopped);
		dump_ep_regs(ep);
	}
	
	local_irq_save(flags);
	_req->status = -EINPROGRESS;
	_req->actual = 0;
	
	/*
	* only if the req queue of ep is empty and 
	* ep is working ,we kick start the queue
	*/
	
	if(list_empty(&ep->queue) && ! ep->stopped) {
		if(ep->dma_nr<0){
#if CONFIG_AM_CHIP_ID == 1211			
			ep->dma_nr = udc->dma_channel;
			set_bit(DMA_CHANNEL1_USED,&udc->dma_used);
			//INFO("ep addr %x ; select dma channel %d\n",ep->bEndpointAddress,ep->dma_nr);
#elif CONFIG_AM_CHIP_ID == 1220
			if(!test_bit(DMA_CHANNEL1_USED,&udc->dma_used)){
				ep->dma_nr = udc->dma_channel;
				set_bit(DMA_CHANNEL1_USED,&udc->dma_used);
			}
			else if(!test_bit(DMA_CHANNEL2_USED,&udc->dma_used)){
				ep->dma_nr = udc->dma_channel2;
				set_bit(DMA_CHANNEL2_USED,&udc->dma_used);
			}
			//INFO("ep addr %x ; select dma channel %d\n",ep->bEndpointAddress,ep->dma_nr);
#endif
		}

		/*imediately process this request*/
		if(ep->bEndpointAddress == 0) {
			/*ep0*/
			if(!udc->req_pending) {
				DMSG_XFERS("<EP QUEUE> something wrong with Control Xfers,req_pending is missing\n");				
				retval =  -EL2HLT;
				goto endl;
			}

			switch(udc->ep0state) {
			case EP0_OUT_DATA_PHASE:
				/*No-Data Control Xfer*/
				if(!req->req.length) {
					/*ACK*/
					DMSG_CTRL("no data ,enter status stage\n");
					handle_status();
					/*cleanup*/
					udc->req_pending = 0;
					udc->ep0state = EP0_WAIT_FOR_SETUP;
					done(ep, req, 0);
					req = NULL;
						retval = 0;
						goto endl;
			 	/* Control Write Xfer */
				}else {
					/*in this case, we just arm the OUT EP0 for first OUT transaction during the data stage
					  *hang this req at the tail of  queue aossciated with EP0
					  *expect OUT EP0 interrupt to advance the i/o queue
					  */
					aotg_writeb(0,OUT0BC);
				 }
			 	break;
			case EP0_IN_DATA_PHASE: 
				/* Control Read Xfer */
				if(write_ep0_fifo(ep,req)) 
					udc->ep0state = EP0_END_XFER;
				break;
			default:
				DMSG_XFERS("<EP QUEUE> ep0 i/o, odd state %d\n",udc->ep0state);
				retval =  -EL2HLT;
				goto endl;
			}
		}
		else  if(use_dma && (udc->dma_channel > 0)){		
			if(aotg_udc_kick_dma(ep , req) == 0){
				//INFO("req.length:%x ,req.actual:%x req : %x \n",req->req.length ,req->req.actual, req);
				/*if we finish transfer,need not queue request*/
				if((req->req.length != 0) && (req->req.length == req->req.actual)){
					DMSG_DMA("req.length:%x ,req.actual:%x req :%x \n",
						req->req.length ,req->req.actual, (u32)req);
					req = NULL;
				}
			}else{
				DMSG_DMA("req.length:%x ,req.actual:%x req : %x \n",
					req->req.length ,req->req.actual, (u32)req);
				req =NULL;
			}
		}else{
			if ((ep->bEndpointAddress & USB_DIR_IN) != 0){
				if(!(aotg_readb(ep->reg_udccs) & EPCS_BUSY)) {/*fifo can access*/
					DMSG_XFERS("<EP QUEUE>%s fifo is all ready,wrtie immediately\n",ep->ep.name);
					ep_irq_clear(ep->mask);/*avoid nonsensical interrupt*/
					if(write_fifo(ep,req))
						req = NULL;
				}
			}else{
				if(!(aotg_readb(ep->reg_udccs) & EPCS_BUSY)) {
					/* in multi-buffer mode there may be some packets in fifo already*/
					DMSG_XFERS("<EP QUEUE>%s fifo has data already,read immediately\n",ep->ep.name);
					ep_irq_clear(ep->mask);/*avoid nonsensical interrupt*/
					if(read_fifo(ep,req))
						req = NULL;
				}
			}	
			
			if( (ep->dma_working == 0)&&req && ep->desc)
				ep_irq_enable(ep->mask);	
		}
	}
	
	if (req != NULL){
		list_add_tail(&req->queue, &ep->queue);
		DMSG_UDC("<EP QUEUE>%s:req not done completely,queueing and wait\n",ep->ep.name);
	}
	retval = 0;
endl:	
	local_irq_restore(flags);
	return retval;
	
}

static int aotg_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct aotg_ep *ep ;
	struct aotg_udc *udc;
	struct aotg_request *req;
	unsigned long flags;

	DMSG_XFERS("<EP DEQUEUE> %s dequeues one req %p\n",_ep->name,_req);
	ep = container_of(_ep,struct aotg_ep,ep);
	udc = ep->dev;
	
	if (!_ep || ep->ep.name == ep0name)
		return -EINVAL;
	
	spin_lock_irqsave(&udc->lock,flags);
	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	
	if (&req->req != _req) {
		spin_unlock_irqrestore(&udc->lock,flags);
		return -EINVAL;
	}

	if(use_dma){
		if(ep->dma_working && !ep->stopped){
			//judge if dequeue request is processing
			req = list_entry(ep->queue.next,struct aotg_request, queue);
			if(&req->req == _req ){
				aotg_udc_cancel_dma(ep);
				done(ep, req, -ECONNRESET);	
				spin_unlock_irqrestore(&udc->lock,flags);
				
				//process next request
				if(!list_empty(&ep->queue)){
					req = list_entry(ep->queue.next,struct aotg_request, queue);
					aotg_udc_kick_dma(ep,req);
				}	
			}	
			
			spin_unlock_irqrestore(&udc->lock,flags);
			return 0;
		}
	}
	
	done(ep, req, -ECONNRESET);
	spin_unlock_irqrestore(&udc->lock,flags);
	return 0;
}


static int aotg_ep_set_halt(struct usb_ep *_ep, int value)
{
	struct aotg_ep  *ep;
	struct aotg_udc *udc;
	unsigned long flags;
	
	int retval = -EOPNOTSUPP;

	ep = container_of(_ep,struct aotg_ep,ep);
	if (!_ep || (!ep->desc && ep->ep.name != ep0name)) {
		DMSG_UDC("<EP HALT>, bad ep\n");
		return -EINVAL;
	}	
	
	udc = ep->dev;
	spin_lock_irqsave(&udc->lock,flags);
	
	/*EP0*/
	if(ep->bEndpointAddress == 0) {
		if(value) {
			aotg_setbits(EP0CS_STALL,EP0CS);
			udc->req_pending = 0;
			udc->ep0state = EP0_STALL;
			retval = 0;
		}else {
			aotg_clearbits(EP0CS_STALL,EP0CS);
			udc->ep0state = EP0_WAIT_FOR_SETUP;
			retval = 0;
		}
	}else if(ep->bmAttributes != USB_ENDPOINT_XFER_ISOC && ep->desc) {	
		if(value) {
			aotg_setbits(EPCON_STALL,ep->reg_udccon); /*set the stall bit*/
			ep->stopped = 1;
		}else {
			aotg_clearbits(EPCON_STALL,ep->reg_udccon); /*clear the stall bit*/
			ep->stopped = 0;	
		}
		retval = 0;
	}
	spin_unlock_irqrestore(&udc->lock,flags);
	
	return retval;
}


static void  aotg_ep_fifo_flush(struct usb_ep* _ep)
{
	struct aotg_ep *ep = container_of(_ep,struct aotg_ep,ep); 
	if (!_ep || (!ep->desc && ep->ep.name != ep0name)) {
		DMSG_UDC("<FIFO FLUSH>bad ep\n");
		return;
	}	
	aotg_ep_reset(ep->mask, ENDPRST_FIFOREST);
}


static struct usb_ep_ops aotg_ep_ops = {
	.enable		= aotg_ep_enable,
	.disable		= aotg_ep_disable,

	.alloc_request	= aotg_ep_alloc_request,
	.free_request	= aotg_ep_free_request,
	
	.queue		= aotg_ep_queue,
	.dequeue		= aotg_ep_dequeue,

	.set_halt		= aotg_ep_set_halt,
	.fifo_flush	= aotg_ep_fifo_flush, /*not sure*/
};


/* ---------------------------------------------------------------------------
 * 	device operations  related parts of the api to the usb controller hardware, which don't involve 
 *	 endpoints (or i/o),used by gadget driver; and the inner talker-to-hardware core.
 * ---------------------------------------------------------------------------
 */
static int aotg_udc_get_frame(struct usb_gadget *_gadget)
{
	struct aotg_udc *udc;
	u16 frmnum;
	
	udc = container_of(_gadget,struct aotg_udc,gadget);
	if(udc->highspeed)
		frmnum = aotg_readw(FRMNRL); /*read FRMNRL and FRMNRH two registers*/
	else
		frmnum = aotg_readw(FRMNRL)>>3;
	DMSG_UDC("<UDC_GET_FRAME>Frame No.: %d\n", frmnum);
	return frmnum;
	
}

static int aotg_udc_wakeup(struct usb_gadget *_gadget)
{
	struct aotg_udc *udc ;
	int  retval = -EHOSTUNREACH;
	
	udc = container_of(_gadget,struct aotg_udc,gadget);
	if(udc->rwk) {
		aotg_writeb(aotg_readb(USBCS) | USBCS_SIGRSUME,USBCS);
		retval = 0;
	}
	
        DMSG_UDC("<UDC_WAKEUP>host  %s support \n",udc->rwk?"can" : "can't");
	return retval;
}

static int aotg_udc_vbus_session(struct usb_gadget *_gadget, int is_active)
{
	struct aotg_udc *udc;
	unsigned long flags;

	DMSG_UDC("<UDC_VBUS_SESSION> VBUS %s\n", is_active ? "on" : "off");
	udc = container_of(_gadget,struct aotg_udc,gadget);
	
	local_irq_save(flags);
	pullup(udc,is_active);
	local_irq_restore(flags);
	return 0;	
}

static int aotg_udc_pullup(struct usb_gadget *_gadget, int is_on)
{
	struct aotg_udc *udc;
	udc = container_of(_gadget, struct aotg_udc, gadget);
	return pullup(udc,is_on);
}


static struct usb_gadget_ops aotg_udc_ops = {
	.get_frame		= aotg_udc_get_frame,
	.wakeup			= aotg_udc_wakeup,
	.vbus_session		= aotg_udc_vbus_session,
	.pullup			= aotg_udc_pullup,
};

/* ---------------------------------------------------------------------------
 *	operations for UDC management 
 * ---------------------------------------------------------------------------
 */
static void udc_enable(struct aotg_udc *dev,int isenable)
{  
	struct aotg_udc *udc = dev;
	if(isenable){
		dev->ep0state = EP0_WAIT_FOR_SETUP;
		dev->state = UDC_ACTIVE;
		DMSG_UDC("<UDC_ENABLE> %p,AOTG enters :%d state\n",dev,aotg_readb(OTGSTATE));
		dplus_up();
		mb();
	}
	else{
		DMSG_UDC("<UDC_DISABLE> %p\n", dev);
		/*FIX ME: clear some irqs*/
		/*Pull down D+ */
		dplus_down();
		mb();
		/*Clear software state*/
		dev->ep0state = EP0_WAIT_FOR_SETUP;
		dev->state = 	UDC_DISABLE;
	}
	udc->enabled = isenable;
}

/*
  * initialize UDC software state
  */
static void udc_reinit(struct aotg_udc *dev)
{
	unsigned i;
	struct aotg_ep *ep ;
	
	/* device/ep0 records init */
	INIT_LIST_HEAD(&dev->gadget.ep_list);
	INIT_LIST_HEAD(&dev->gadget.ep0->ep_list);
	
	dev->ep0state = EP0_WAIT_FOR_SETUP;
	dev->gadget.speed = USB_SPEED_UNKNOWN;
	dev->state = UDC_UNKNOWN;
	dev->dma_channel = -1;
	dev->dma_channel2 = -1;
	dev->dma_used = 0;
	dev->private = NULL;

	dev->gadget.b_hnp_enable = 0;
	dev->gadget.a_hnp_support = 0;
	dev->gadget.a_alt_hnp_support = 0;		
	
	/* basic endpoint records init */
	for (i = 0; i < AOTG_UDC_NUM_ENDPOINTS; i++) {
		ep = &dev->ep[i];
		if (i != 0)
			list_add_tail(&ep->ep.ep_list, &dev->gadget.ep_list);
		ep->desc = 0;
		ep->stopped = 0;
		INIT_LIST_HEAD(&ep->queue);
	}
}

static void udc_quiesce(struct aotg_udc *udc)
{
	/* prevent new request submissions, kill any outstanding requests  */
	int i ;
	udc->state = UDC_DISABLE;
	for (i = 0 ; i < AOTG_UDC_NUM_ENDPOINTS; i++) {
		struct aotg_ep *ep = &udc->ep[i];	
		ep->stopped = 1;
		nuke(ep, -ESHUTDOWN);
	}

	/* report disconnect; the driver is already quiesced */
	if( /*(udc->gadget.speed != USB_SPEED_UNKNOWN) &&*/ udc->driver
				&& udc->driver->disconnect) {
		udc->driver->disconnect(&udc->gadget);
		udc->gadget.speed = USB_SPEED_UNKNOWN;
	}  
}


static int aotg_udc_event_notify(void *target,int event)
{
	struct aotg_udc * udc = (struct aotg_udc*)target;
	switch(event){
		case AOTG_PERIPHERAL_DISCONN_EVENT:
			DMSG_UDC("discon from host ,udcstate:%d\n",udc->state);
			disable_udc_interrupt();
			if(use_dma  && udc->dma_channel > 0){
				DMSG_UDC("disable dma irq\n");
				am_clear_dma_irq(udc->dma_channel,1,1);
			}
			
			if(udc->state != UDC_UNKNOWN){
				DMSG_UDC("do disconnect\n");
				udc->state = UDC_UNKNOWN;
				udc_quiesce(udc);
			}
			break;
		case AOTG_SOFT_CONN_EVENT:
			INFO("soft connect ,udcstate:%d\n",udc->state);
			break;		
		default:
			break;
	}
	return  0;
}

/*---------------------------------------------------------------------------------*/
static void nop_release(struct device *dev)
{
	DMSG_UDC("%s %s\n", __FUNCTION__, dev_name(dev));
}


static struct aotg_udc  controller = {
	.gadget = {
		.ops		= &aotg_udc_ops,
		.speed 	= USB_SPEED_HIGH,
		.ep0		= &controller.ep[0].ep,
		.name	= driver_name,
		.dev = {
			.bus_id	= "gadget",
			.release 	= nop_release,
		},
	},
	/* control endpoint */
	.ep[0] = {
		.ep = {
			.name		= ep0name,
			.ops			= &aotg_ep_ops,
			.maxpacket	= EP0_PACKET_SIZE,
		},
		.dev				= &controller,
		.maxpacket            = EP0_PACKET_SIZE,
		.bEndpointAddress = 0,
		.pkt_msk			= 6,
	},
	/* bulk in endpoint */
	.ep[1] = {
		.ep = {
			.name		= "ep1out-bulk",
			.ops		= &aotg_ep_ops,
			.maxpacket	= BULK_HS_PACKET_SIZE,
		},
		.dev		= &controller,
		.maxpacket	= BULK_HS_PACKET_SIZE,
		.pkt_msk		= BULK_HS_PACKET_MASK,
		.bEndpointAddress =  1,
		.bmAttributes	= USB_ENDPOINT_XFER_BULK,
		.mask = 1,
		.reg_udccs  =  OUT1CS,
               .reg_udccon = OUT1CON,
		.reg_udcbc = OUT1BCL,
		.reg_udcfifo = FIFO1DAT0,
		.dma_nr = -1,
	},
	/* bulk out endpoint */
	.ep[2] = {
		.ep = {
			.name		= "ep1in-bulk",
			.ops		= &aotg_ep_ops,
			.maxpacket	= BULK_HS_PACKET_SIZE,
		},
		.dev		= &controller,
		.maxpacket	= BULK_HS_PACKET_SIZE,
		.pkt_msk		= BULK_HS_PACKET_MASK,
		.bEndpointAddress = USB_DIR_IN | 1,
		.bmAttributes	= USB_ENDPOINT_XFER_BULK,
		.mask = USB_UDC_IN_MASK | 1,
		.reg_udccs  =  IN1CS,
               .reg_udccon = IN1CON,
		.reg_udcbc = IN1BCL,
		.reg_udcfifo = FIFO1DAT0,
		.dma_nr = -1,
	},	
	
	/* used for pictbridge */
#ifdef SUPPORT_INTIN
	.ep[3] = {
		.ep = {
			.name		= "ep2in-int",
			.ops		= &aotg_ep_ops,
			.maxpacket	= BULK_FS_PACKET_SIZE,
		},
		.dev		= &controller,
		.maxpacket	= BULK_FS_PACKET_SIZE,
		.pkt_msk		= BULK_FS_PACKET_MASK,
		.bEndpointAddress = USB_DIR_IN | 2,
		.bmAttributes	= USB_ENDPOINT_XFER_INT,
		.mask = USB_UDC_IN_MASK | 2,
		.reg_udccs  =  IN2CS,
               .reg_udccon = IN2CON,
		.reg_udcbc = IN2BCL,
		.reg_udcfifo = FIFO2DAT0,
	},	
#endif
};


/* ---------------------------------------------------------------------------
 * 	export for gadget driver modules registeration and unregister 
 * ---------------------------------------------------------------------------
 */
int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
  	struct aotg_udc * udc = &controller;
	int  retval = -ENODEV;
	struct port_handler *process;

	DMSG_UDC("gadget register\n");
	if (!driver
			|| driver->speed < USB_SPEED_FULL
			|| !driver->bind
			|| !driver->unbind
			|| !driver->setup
			|| !driver->disconnect)
		return -EINVAL;

	if (udc->driver) {
		ERR("one gadget drvier has already registered\n");
		retval = -EBUSY;
		return retval;
	}

	/*inform otg layer to be device role*/
	retval = otg_set_peripheral(udc->transceiver, &udc->gadget);
	if (retval) {
		ERR("%s: bind to otg driver %s  error %d\n", udc->gadget.name,
		       udc->transceiver->label, retval);
		
		driver->unbind(&udc->gadget);
		udc->driver = NULL;
		udc->gadget.dev.driver_data = NULL;
		udc->softconnect = 0;
		udc->gadget.is_otg = 0;
		return retval;
	}

	/*liucan:register port hander ,for the hotplug detect 
		is developed  at otg layer
	*/
	process = kzalloc(sizeof(struct port_handler),GFP_KERNEL);
	if(!process){
		ERR("%s:alloc failed\n",__FUNCTION__);
		otg_set_peripheral(udc->transceiver,NULL);
		return -ENOMEM;
	}
	
	udc->private = (void*)process;
	process->data       = (unsigned long)udc;
	process->function = aotg_udc_event_notify;
	retval = otg_set_porthandler_next(udc->transceiver,process);
	if(retval < 0 ){
		ERR("<UDC_PROBE>otg set porthandler failed\n");
		otg_set_peripheral(udc->transceiver,NULL);
		return retval;
	}

	/* hook up the driver */
	udc->driver = driver;
	udc->gadget.dev.driver_data = &driver->driver;
       udc->gadget.is_dualspeed = 1;
	udc->gadget.is_otg = 0;
	udc->softconnect = 1;
	retval = driver->bind(&udc->gadget);  /*bind UDC with a Gadget driver*/
	if (retval) {
		ERR("%s: bind to driver %s  error %d\n", udc->gadget.name,
		       driver->driver.name, retval);
		udc->driver = NULL;
		udc->gadget.dev.driver_data = NULL;
		udc->softconnect = 0;
		udc->gadget.is_otg = 0;
		otg_set_peripheral(udc->transceiver,NULL);
		kfree(process);
		return retval;
	}
	
	pullup(udc,1);
	DMSG_UDC("register ok,pull up to let pc see us\n");

	return 0;
}
EXPORT_SYMBOL (usb_gadget_register_driver);


int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct aotg_udc * udc = &controller;
	int retval;
	
	if(!driver ||(udc->driver != driver))
		return -EINVAL;
	
	/*pull down*/
	pullup(udc, 0);	
	
        /*Unhook the driver*/
	driver->unbind(&udc->gadget);
	udc->gadget.dev.driver_data = NULL;
	udc->gadget.is_otg = 0;
	udc->driver = NULL;
	udc->softconnect = 0;

	retval = otg_set_porthandler_next(udc->transceiver,NULL);
	if(retval < 0){
		ERR("unset porthandler failed\n");
		return retval;
	}

	retval = otg_set_peripheral(udc->transceiver,NULL);	
	if(retval){
		ERR("set peripheral failed\n");
		return retval;
	}
	
	if(udc->private){
		kfree(udc->private);
		udc->private = NULL;
	}
	
	DMSG_UDC("%s: unregistered gadget driver '%s'\n", 
		udc->gadget.name,driver->driver.name);		
	
	return 0;
}
EXPORT_SYMBOL (usb_gadget_unregister_driver);

static inline void udc_resume_event(void)
{
	struct aotg_udc * udc = &controller;
	spin_lock_irq(&udc->lock);
	udc->state = UDC_ACTIVE;
	udc->driver->resume(&udc->gadget);
	spin_unlock_irq(&udc->lock);
}

static inline void udc_suspend_event(void)
{
	struct aotg_udc * udc = &controller;	
	spin_lock_irq(&udc->lock);
	udc->state = 	UDC_SUSPEND;
	udc->driver->suspend(&udc->gadget);
	spin_unlock_irq(&udc->lock);
}

/* ---------------------------------------------------------------------------
 * 	handle  interrupt 
 * ---------------------------------------------------------------------------
 */
static irqreturn_t aotg_udc_irq(int irqnum,void *_udc)
{
	struct aotg_udc *udc = (struct aotg_udc *)_udc;
	u8	is_high_speed = 0;
	u8	irqvector;

	irqvector = aotg_readb(IVECT);	
	if(irqvector == UIV_OTGIRQ){
		DMSG_UDC("the irq not belongs to uoc\n");
		return IRQ_HANDLED;
	}

	DMSG_UDC("----irqvector=%x----\n",irqvector);
	switch(irqvector){			
		case UIV_USBRESET:
			aotg_writeb(aotg_readb(OTGIRQ),OTGIRQ);	
			aotg_writeb(aotg_readb(IN07IRQ),IN07IRQ);
			aotg_writeb(aotg_readb(IN07IRQ),OUT07IRQ);
			
			/*enable ep0IN interrupt*/
			aotg_writeb(EP0_IN_IRQ,IN07IEN);
			aotg_writeb(EP0_OUT_IRQ,OUT07IEN);				
			DMSG_UDC("gadget %s,USB reset done\n",udc->driver->driver.name);

			/*recover speed to FS*/
			is_high_speed = aotg_readb(USBIRQ) & USBIRQ_HS;
			if(is_high_speed){		   
				INFO("uhsr\n");
				udc->gadget.speed = USB_SPEED_HIGH;
				udc->highspeed = 1;	
			
				udc->ep[1].ep.maxpacket = BULK_HS_PACKET_SIZE;
				udc->ep[1].maxpacket = BULK_HS_PACKET_SIZE;
				udc->ep[1].pkt_msk = BULK_HS_PACKET_MASK;
				udc->ep[2].ep.maxpacket = BULK_HS_PACKET_SIZE;
				udc->ep[2].maxpacket = BULK_HS_PACKET_SIZE;
				udc->ep[2].pkt_msk = BULK_HS_PACKET_MASK;	
				
				aotg_writew(BULK_HS_PACKET_SIZE,OUT1MAXPCKL);  
				aotg_writew(BULK_HS_PACKET_SIZE,IN1MAXPCKL);	
			}else{
				INFO("ufsr\n");
				udc->gadget.speed = USB_SPEED_FULL;
				udc->highspeed = 0;	
				
				udc->ep[1].ep.maxpacket = BULK_FS_PACKET_SIZE;
				udc->ep[1].maxpacket = BULK_FS_PACKET_SIZE;
				udc->ep[1].pkt_msk = BULK_FS_PACKET_MASK;
				udc->ep[2].ep.maxpacket = BULK_FS_PACKET_SIZE;
				udc->ep[2].maxpacket = BULK_FS_PACKET_SIZE;
				udc->ep[2].pkt_msk = BULK_FS_PACKET_MASK;

				aotg_writew(BULK_FS_PACKET_SIZE,OUT1MAXPCKL); 
				aotg_writew(BULK_FS_PACKET_SIZE,IN1MAXPCKL);
			}
			/*pictbridge/rndis interface needs int ep*/			
#ifdef  SUPPORT_INTIN
			aotg_writew(BULK_FS_PACKET_SIZE,IN1MAXPCKL);
#endif
			aotg_ep_setup(&udc->ep[1],EPCON_TYPE_BULK,EPCON_BUF_QUAD);
			aotg_ep_setup(&udc->ep[2],EPCON_TYPE_BULK,EPCON_BUF_QUAD);
			/*fifo only 64 bytes left ,so config ep in2 int to single*/
#ifdef  SUPPORT_INTIN
			aotg_ep_setup(&udc->ep[3],EPCON_TYPE_INT,EPCON_BUF_SINGLE);
#endif
			aotg_clear_pendbits(USBIRQ_URES,USBIRQ);
			udc_resume_event();
			break;

		case UIV_HSPEED:
			INFO("uhsi\n");	
			udc->gadget.speed = USB_SPEED_HIGH;
			udc->highspeed = 1;
			
			udc->ep[1].ep.maxpacket = BULK_HS_PACKET_SIZE;
			udc->ep[1].maxpacket = BULK_HS_PACKET_SIZE;
			udc->ep[1].pkt_msk = BULK_HS_PACKET_MASK;
			udc->ep[2].ep.maxpacket = BULK_HS_PACKET_SIZE;
			udc->ep[2].maxpacket = BULK_HS_PACKET_SIZE;
			udc->ep[2].pkt_msk = BULK_HS_PACKET_MASK;	
			
			aotg_writew(BULK_HS_PACKET_SIZE,OUT1MAXPCKL);	
			aotg_writew(BULK_HS_PACKET_SIZE,IN1MAXPCKL);
			
			aotg_ep_setup(&udc->ep[1],EPCON_TYPE_BULK,EPCON_BUF_QUAD);
			aotg_ep_setup(&udc->ep[2],EPCON_TYPE_BULK,EPCON_BUF_QUAD);
#ifdef  SUPPORT_INTIN
			aotg_ep_setup(&udc->ep[3],EPCON_TYPE_INT,EPCON_BUF_SINGLE);
#endif
			aotg_clear_pendbits(USBIRQ_HS,USBIRQ); /*clear highspeed irq*/	
			udc_resume_event();
			break;
			
		case  UIV_SUSPEND:	
			INFO("suspend\n");
			printk("debug---%s %d\n",__FUNCTION__,__LINE__);
			aotg_clear_pendbits(USBIRQ_SUSP,USBIRQ); 
			/*let uplayer do suspend event*/
			udc_suspend_event();
			
			/*	our IC will suspend when plugin,because 
			 *	windows OS NOT support resume,so we do
			 *  	should do nothing here
			 */	 
			//if(udc->transceiver)
			//	otg_set_suspend(udc->transceiver,1);
			break;
			
  		case UIV_EP1IN:
			//INFO("EP1IRQ\n");
			aotg_writeb(EP1_IN_IRQ,IN07IRQ); /*clear ep1in irq*/
    			handle_ep(udc,2);
    			break;
			
#ifdef SUPPORT_INTIN
  		case UIV_EP2IN:
    			aotg_writeb(EP2_IN_IRQ,IN07IRQ); /*clear ep2in irq*/
   	 		handle_ep(udc,3);
  	  		break;
#endif
		case UIV_EP1OUT:
			aotg_writeb(EP1_OUT_IRQ,OUT07IRQ); /*clear ep1out irq*/
			handle_ep(udc,1);
			break;
		case UIV_SUDAV:
			aotg_clear_pendbits(USBIRQ_SUDAV,USBIRQ); /*clear sudav irq*/
			handle_setup(udc);
			break;
		case UIV_EP0IN:
			aotg_writeb(EP0_IN_IRQ,IN07IRQ); /*clear ep0in irq*/
			handle_ep0(udc);
			break;
		case UIV_EP0OUT:
			aotg_writeb(EP0_OUT_IRQ,OUT07IRQ); /*clear ep0out irq*/
			handle_ep0(udc);
			break;
		default:
			break;		
	}
	
	return IRQ_HANDLED;
}


/*if this function called ,means transfer data size is bigger than maxpacketsize*/
static void aotg_udc_start_dma(struct aotg_ep * ep, struct aotg_request * req , int is_in)
{
	//struct aotg_udc * udc = ep->dev;
	u32 dcmd,buf_addr,fifo_addr;
	u32	dma_channel;
	if(ep->stopped ){
		ERR("%s:ep is stopped\n",__FUNCTION__);
		return ;
	}

	buf_addr = (u32)req->req.buf  +  req->req.actual ;
	fifo_addr = ep->reg_udcfifo;
	dcmd = (req->req.length - req->req.actual) - 
		( (req->req.length - req->req.actual) % ep->desc->wMaxPacketSize ) ;
	ep->dma_bytes = dcmd;	
	dma_channel = ep->dma_nr;
	DMSG_DMA("dma:%x->%x len:%d\n", buf_addr,fifo_addr,dcmd);

	/*set fifo auto*/
	dma_cache_wback_inv(buf_addr,dcmd);
	aotg_set_fifoauto(ep,1);

	if(is_in){
		/*select ep for dma */
		aotg_writeb((ep->bEndpointAddress&0xf)<<1,DMAEPSEL);		//select EP IN	
		/*config dma*/
		am_dma_cmd(dma_channel, DMA_CMD_RESET);
		am_set_dma_addr(dma_channel,buf_addr,fifo_addr);
		am_dma_config(dma_channel,DMA_MODE_IN,dcmd);	
	}else{
		aotg_writeb(((ep->bEndpointAddress&0xf)<<1) + 1,DMAEPSEL); 	//select EP OUT
		/*config dma*/
		am_dma_cmd(dma_channel,DMA_CMD_RESET);
		am_set_dma_addr(dma_channel,fifo_addr, buf_addr);	
		am_dma_config(dma_channel,DMA_MODE_OUT,dcmd);	
	}
	
	am_set_dma_irq(dma_channel,1,0);
	am_dma_start(dma_channel,0);
	DMSG_DMA("<start_dma>:arm end........\n");
}



static void aotg_udc_cancel_dma(struct aotg_ep * ep)
{
	//struct aotg_udc * udc = ep->dev;	
	u32	dma_channel;
	DMSG_DMA("<UDC_CANCEL_DMA>\n");
	dma_channel = ep->dma_nr;
	ep->dma_working = 0;
	
	am_dma_cmd(dma_channel,DMA_CMD_PAUSE);
	am_dma_cmd(dma_channel,DMA_CMD_RESET);
	aotg_set_fifoauto(ep,0);
	aotg_ep_reset(ep->mask,ENDPRST_FIFOREST); 	
}


static unsigned int  aotg_udc_kick_dma(struct aotg_ep * ep, struct aotg_request * req)
{
	unsigned int remains =  req->req.length - req->req.actual;
	
	DMSG_DMA("<kick_dma>length:%d,actual:%d\n",req->req.length,req->req.actual);	
	
	if(ep->bEndpointAddress & USB_DIR_IN){
		if((remains == 0) || (remains < ep->maxpacket)){
			DMSG_DMA("<IN>little pkt\n");
			ep_irq_enable(ep->mask);
			if(!(aotg_readb(ep->reg_udccs) & EPCS_BUSY)){
				return write_fifo(ep,req);
			}else
				DMSG_DMA("otg ep busy now\n");		
		}else{
			DMSG_DMA("<kick_dma>:EP IN DMA,remains:%d\n",remains);
			ep->dma_working = 1;
			ep_irq_disable(ep->mask);
			aotg_udc_start_dma(ep ,req ,USB_DIR_IN);
		}		
	}else{
		/*for CBW,we queue request of length:512 ,but actual 
		   host send data length is 31,so is <= not <*/
		if((remains == 0) ||(remains <= ep->maxpacket)){
			DMSG_DMA("<OUT>little pkt\n");
			ep_irq_enable(ep->mask);
			if(!(aotg_readb(ep->reg_udccs) & EPCS_BUSY))
				return read_fifo(ep , req);
		}else{
			DMSG_DMA("<kick_dma>:OUT: DMA trans\n");
			ep->dma_working = 1;
			ep_irq_disable (ep->mask);
			aotg_udc_start_dma(ep ,req,USB_DIR_OUT);
		}
	}
	
	return 0;
}



static irqreturn_t aotg_udc_dma_handler(int irqnum,void *_udc)
{
	struct aotg_udc * udc = (struct aotg_udc *)_udc;
	int i , completed;
	struct aotg_ep *  ep = NULL ;
	struct aotg_request * req;
	u32 remains;
	u32 dma_channel;
	if(!am_get_dma_irq(udc->dma_channel,1,0)){
#if CONFIG_AM_CHIP_ID==1220
		if(!am_get_dma_irq(udc->dma_channel2,1,0)){	
			DMSG_DMA( "%s:irq may not belongs to us!\n",__FUNCTION__);			
			return IRQ_NONE;
		}
		else
			dma_channel = udc->dma_channel2;

#elif CONFIG_AM_CHIP_ID==1211
		DMSG_DMA( "%s:irq may not belongs to us!\n",__FUNCTION__);			
		return IRQ_NONE;
#endif

	}
	else
		dma_channel = udc->dma_channel;
	
	/*find the ep,skip ep0*/
		
	for(i = 1; i <AOTG_UDC_NUM_ENDPOINTS-1; i++){
		if(udc->ep[i].dma_working == 1 && udc->ep[i].dma_nr==dma_channel){
			ep = & udc->ep[i];
			break;
		}
	}
	
	if(!ep){
		ERR("odd ,no ep dma is working!\n");
		return IRQ_HANDLED;
	}
	
	/*clear dma irq pend*/
	am_clear_dma_irq(dma_channel,1,0);
	
	/*liucan:clear fifoauto, next req will not use dma*/
	aotg_set_fifoauto(ep,0);
	
	req = list_entry(ep->queue.next,struct aotg_request,queue);
	req->req.actual += ep->dma_bytes;
	remains = req->req.length - req->req.actual;
	completed = 0;
	
	DMSG_DMA("remains:%d,length:%d,actual:%d,maxpacket:%d\n",
				remains,req->req.length,req->req.actual,ep->maxpacket);
	
	if(ep->bEndpointAddress & USB_DIR_IN){
		if( ((remains > 0) &&( remains < ep->maxpacket) ) ||
				(( remains == 0 ) && req->req.zero)){
			/*transfer remains use cpu*/	
			if(aotg_readb(ep->reg_udccs) & EPCS_BUSY){
				DMSG_DMA("<dma_handler>:IN BUSY..\n");
				ep_irq_enable(ep->mask);	
				ep->dma_working = 0;
			}else{
				DMSG_DMA("<dma_handler>:IN NOT busy..\n");
				write_packet(ep,req,ep->maxpacket);
				done(ep, req, 0);
				ep->dma_working = 0;
				completed = 1;
			}
		}else if(remains >= ep->maxpacket){
			 /*too much bytes,needs another dma transfer*/
			DMSG_DMA("<dma_handler>:remains:%d\n",remains);
			aotg_udc_kick_dma(ep , req);	
		}else{ 
			//complete
			DMSG_DMA("<dma_handler>: req ok\n");
			done(ep, req, 0);
			ep->dma_working = 0;
			completed = 1;
		}
	}else {
		if ( (remains > 0) && ( remains < ep->maxpacket) ){
			DMSG_DMA("<dma_hander>:OUT short packet\n");
			ep_irq_clear(ep->mask);
			ep_irq_enable(ep->mask);
			read_fifo(ep,req);
			ep->dma_working = 0;
		}else if(remains >= ep->maxpacket){
			DMSG_DMA("<dma_handler>:remains:%d\n",remains);
			aotg_udc_kick_dma(ep , req);	
		}else{
			DMSG_DMA("<dma_hander>:packet compeleted\n");	
			ep->dma_working = 0;
			done(ep ,req ,0);	
			completed = 1;
		}		
	}
	
	if(completed){	
		if (!list_empty(&ep->queue)){
			req = list_entry(ep->queue.next, struct aotg_request, queue);
			aotg_udc_kick_dma(ep , req);
		}
	}else
		ep_irq_clear(ep->mask);
	
	DMSG_DMA( "exit dma handler\n");
	return IRQ_HANDLED;
}


static void aotg_ep_reset(u8 ep_mask,u8 type_mask)
{
	u8 val ;
	aotg_writeb(ep_mask,ENDPRST); /*select ep*/
	val = ep_mask |type_mask;
	aotg_writeb(val,ENDPRST);/*reset ep*/
}


/*
  * setup ep 
 */
static void aotg_ep_setup(struct aotg_ep *ep, u8 type,u8 buftype)
{
	ep->buftype = buftype;
	aotg_writeb(type |buftype,ep->reg_udccon);
	aotg_ep_reset(ep->mask, ENDPRST_TOGRST | ENDPRST_FIFOREST);
}

static void config_ep_fifo_addr(void)
{
	aotg_writel(EP1OUTSTARTADD,OUT1STARTADDRESS);
	aotg_writel(EP1INSTARTADD,IN1STARTADDRESS);
	aotg_writel(EP2INSTARTADD,IN2STARTADDRESS);

	DMSG_UDC("ep1out fifoaddr:%x\n",aotg_readl(OUT1STARTADDRESS));
	DMSG_UDC("ep1in fifoaddr:%x\n",aotg_readl(IN1STARTADDRESS));
	DMSG_UDC("ep2in fifoaddr:%x\n",aotg_readl(IN2STARTADDRESS));
}


/*
 * 	Install
 */
 static int __init  am7x_udc_probe(struct platform_device *pdev)
{
	struct aotg_udc  *udc ;
	int retval = 0;
	struct device	*dev = &pdev->dev;
	unsigned long dma_weight;

	udc= &controller;
	udc_reinit(udc);
	spin_lock_init(&udc->lock);
	udc->transceiver = otg_get_transceiver_next();
	if(!udc->transceiver) {
		ERR("<UDC_PROBE>no transceciver\n");
		retval = -ENODEV;
		goto end1;
	}
	
	config_ep_fifo_addr();
	/*request dma*/
	if(use_dma){
		retval = am_request_dma(DMA_CHAN_TYPE_SPECIAL,
				"udc",aotg_udc_dma_handler,
				0,(void *)udc);
		if(retval <0 ){
			ERR("<UDC_PROBE>request DMA failed\n");
			goto end1;
		}	
		
		udc->dma_channel = retval;
		am_dma_cmd(udc->dma_channel,DMA_CMD_RESET);
		am_set_dma_irq(udc->dma_channel,1,0);
		
		/*set dma never timeout */
		dma_weight = am_get_dma_weight(udc->dma_channel);
		am_set_dma_weight(udc->dma_channel,dma_weight |(1UL<<28));
		INFO("<UDC_PROBE>DMA request success ,chnl:%d\n",udc->dma_channel);

#if CONFIG_AM_CHIP_ID==1220
		retval = am_request_dma(DMA_CHAN_TYPE_SPECIAL,
				"udc",aotg_udc_dma_handler,
				0,(void *)udc);
		if(retval <0 ){
			ERR("<UDC_PROBE>request DMA failed\n");
			goto end1;
		}	
		
		udc->dma_channel2 = retval;
		am_dma_cmd(udc->dma_channel2,DMA_CMD_RESET);
		am_set_dma_irq(udc->dma_channel2,1,0);
		
		/*set dma never timeout */
		dma_weight = am_get_dma_weight(udc->dma_channel2);
		am_set_dma_weight(udc->dma_channel2,dma_weight |(1UL<<28));
		INFO("<UDC_PROBE>DMA request success ,chnl:%d\n",udc->dma_channel2);
#endif
	}
	
	retval =  request_irq(IRQ_USB2, aotg_udc_irq,
				IRQF_SHARED |IRQF_DISABLED, udc->gadget.name, 
				(void *)udc);
	if (retval < 0){
		ERR("<UDC_PROBE>request irq %d failed\n",IRQ_USB2);
		goto end2;
	}
	printk("debug---%s #####@@@@ :%d",__FUNCTION__,IRQ_USB2);
	/*register driver*/
	retval = device_register(&udc->gadget.dev);
	if (retval < 0){
		ERR("<UDC_PROBE>register device driver failed!\n");
		goto end3;
	}
	
	create_debug_file(udc);
	dev_set_drvdata(dev,udc);
	INFO("%s:driver probe success\n",driver_name);
	return 0;
	
end3:
	free_irq(IRQ_USB2,udc);
end2:
	if(use_dma && udc->dma_channel > 0){
		am_free_dma(udc->dma_channel);
		if(udc->dma_channel2 > 0)
			am_free_dma(udc->dma_channel2);	
	}
		
end1:	
	return retval;
}

/*
 * 	Remove - frees resources when the driver is unloaded
 */
 static int __exit am7x_udc_remove(struct platform_device *pdev)
{
	int i;
	struct aotg_ep *hep = NULL;
	struct aotg_udc * udc = platform_get_drvdata(pdev);
	if(udc->driver){
		WARN("driver is still used\n");
		return -EBUSY;
	}
	
	if(use_dma &&udc->dma_channel > 0 ){
		for(i=0;i < AOTG_UDC_NUM_ENDPOINTS;i++){
			hep =&udc->ep[i];
			if(!hep ||( hep->dma_working == 0)) continue;
			aotg_udc_cancel_dma(hep);
			INFO("free dma %d\n",udc->dma_channel);	
			am_free_dma(hep->dma_nr);
			hep->dma_nr = -1;
		}
		udc->dma_channel = -1;
		udc->dma_channel2 = -1;
		udc->dma_used  =0;
	}	
	
	free_irq(IRQ_USB2,udc);
	
	remove_debug_file(udc);
	device_unregister(&udc->gadget.dev);
	INFO("udc remove success\n");
	return 0;
}


static void am7x_udc_shutdown(struct platform_device *dev)
{
	/* force disconnect on reboot */
	pullup(platform_get_drvdata(dev), 0);
}

static struct platform_driver am7x_udc_driver = {
	.remove		= __exit_p(am7x_udc_remove),
	.shutdown	= am7x_udc_shutdown,
	.suspend		= NULL,
	.resume		= NULL,
	.driver		= {
		.name	= (char *) driver_name,
		.owner	= THIS_MODULE,
	},
};


static int __init udc_init_module(void)
{
	return platform_driver_probe(&am7x_udc_driver, am7x_udc_probe);
}
module_init(udc_init_module);


static void __exit udc_exit_module(void)
{
	platform_driver_unregister(&am7x_udc_driver);
}
module_exit(udc_exit_module);



MODULE_DESCRIPTION("aotg udc driver");
MODULE_AUTHOR("actions, liucan");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:aotg_udc");

