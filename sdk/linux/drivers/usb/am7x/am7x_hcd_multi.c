#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/usb/otg.h>
#include <linux/interrupt.h>
#include <linux/usb.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/freezer.h>
#include <linux/kthread.h>


#include "../core/hcd.h"
#include "../../../include/linux/am7x_mconfig.h"
#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
#include "../../../include/linux/usb/hotplug.h"// T932
#include "am7x_uoc.h"// T166,William 20150814
#endif
#include "actions_regs.h"
#include "aotg_regs.h"
#include "aotg_io.h"
#include "am7x_hcd.h"
#include "dma.h"

//#define  DEBUG_TXRX
#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
#ifdef MODULE_CONFIG_EZCASTPRO_MODE
#undef MODULE_CONFIG_EZCASTPRO_MODE
#endif
#endif
#define	EAOTGINIT					0xffff

#define   AM_MAX_ROOT_HUB			1

#define 	DMA_LEAST_BYTE				256
#define 	DMA_DEFAULT_USE			0


#define	DRIVER_VERSION	"Version1.0-Apirl-2007"
#define	DRIVER_DESC	"ACTIONS USB Host Controller Driver"
#define   DRIVER_INFO DRIVER_VERSION " " DRIVER_DESC

static const char hcd_name [] = "aotg-usb-am7x-";
static unsigned  use_dma  = DMA_DEFAULT_USE;

#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
// T166,William 20150814
extern int get_cur_usb_inout_status(void);
extern void set_cur_usb_inout_status(int status);
#endif

module_param_named(use_dma, use_dma, bool, S_IRUGO);
MODULE_PARM_DESC(use_dma, "true to use dma for data transfer");

static void invalidate_ep(struct aotg_hcep *ep);
static void validate_ep(struct aotg_hcep *ep);
static void dump_periodic_sched(struct aotg_hcd	*acthcd);
static inline int compute_frame_remainder(struct aotg_hcep *ep,struct urb* urb);
static int start_periodic_transfer(struct aotg_hcd *acthcd,struct aotg_hcep	*mep);
static int  start_async_transfer(struct aotg_hcd *acthcd,struct aotg_hcep *ep);
static void aotg_hcd_endpoint_disable(struct usb_hcd *hcd,struct usb_host_endpoint *hep);
static irqreturn_t aotg_hcd_dma_handler(int irq, void *_dev);
static void 	    aotg_hcd_cancel_dma(struct aotg_hcep * ep , struct aotg_hcd *acthcd);
static void do_sof_irq(struct aotg_hcd *acthcd);
static void start_schedule(struct aotg_hcd *acthcd,struct aotg_hcep* ep );
static void kick_ep_thread();
static int inline ep_has_exist_async_list(struct aotg_hcep	*hep,struct list_head *head);
static void Send_token_timeout_timer_func(unsigned long data);
static void aotg_hcep_reset(u8 ep_mask,u8 type_mask);




#ifdef CONFIG_PM
static int aotg_hcd_suspend(struct usb_hcd *hcd);
static int aotg_hcd_resume(struct usb_hcd *hcd);
#endif

/*--------------------------------------------------------------------------*/
//default ep setting
static DECLARE_WAIT_QUEUE_HEAD(am7x_hcd_wait);
static LIST_HEAD(am7x_hcd_event_list);
static LIST_HEAD(am7x_hcd_event);
static struct usb_hcd * am7x_hcd =NULL;
static struct task_struct *am7x_hcd_task;
static int irq_cnt = 0;

static int urb_index=0;

//#define TEST_POINT 1
//static int sofEnable;


/*hcep_map hcep_map_reserv 是全局数组，在insmod的时候应该会初始化为0,所以不必zeror it*/
#define PIPE_UNKOWN 4
struct aotg_hcep hcep_map_reserv[6];

struct aotg_hcep hcep_map[] = {
	{//control pipe
		.index 	 = 0, 
		.mask 	 = 0,
		.inuse   = 0,
		.type 	 = PIPE_CONTROL,	
	},	

	{ //bulk in 
		.index 	 = 1, 
		.mask 	 = 0x01,
		.inuse   = 0,
		.type 	 = PIPE_UNKOWN,
		.buftype	 = EPCON_BUF_UNKOWN,
	},	
	
	{//bulk out
		.index 	 = 2, 
		.mask 	 = 0x11 ,
		.inuse   = 0,
		.type 	 = PIPE_UNKOWN ,
		.buftype	 = EPCON_BUF_UNKOWN,
	},
	
	{//int in
		.index 	 = 3, 
		.mask 	 = 0x02 ,
		.inuse   = 0,
		.type 	 = PIPE_UNKOWN ,
		.buftype	 = EPCON_BUF_UNKOWN,
	},
	
	{//bulk in
		.index 	 = 4, 
		.mask 	 = 0x12,
		.inuse   = 0,
		.type 	 = PIPE_UNKOWN ,
		.buftype	 = EPCON_BUF_UNKOWN,
	},
#if CONFIG_AM_CHIP_ID ==1213
	{//int 
		.index 	 = 5, 
		.mask 	 = 0x03 ,
		.inuse   = 0,
		.type 	 = PIPE_UNKOWN ,
		.buftype	 = EPCON_BUF_UNKOWN,
	},
	
	{//bulk 
		.index 	 = 6, 
		.mask 	 = 0x13,
		.inuse   = 0,
		.type 	 = PIPE_UNKOWN ,
		.buftype	 = EPCON_BUF_UNKOWN,
	},
	{//bulk 
		.index 	 = 7, 
		.mask 	 = 0x04,
		.inuse   = 0,
		.type 	 = PIPE_UNKOWN ,
		.buftype	= EPCON_BUF_UNKOWN,
	},
	{//bulk 
		.index 	 = 8, 
		.mask 	 = 0x14,
		.inuse   = 0,
		.type 	 = PIPE_UNKOWN ,
		.buftype	 = EPCON_BUF_UNKOWN,
	},
	{//bulk 
		.index 	 = 9, 
		.mask 	 = 0x05,
		.inuse   = 0,
		.type 	 = PIPE_UNKOWN ,
		.buftype	= EPCON_BUF_UNKOWN,
	},
	{//bulk 
		.index 	 = 10, 
		.mask 	 = 0x15,
		.inuse   = 0,
		.type 	 = PIPE_UNKOWN ,
		.buftype	 = EPCON_BUF_UNKOWN,
	},
	{//bulk 
		.index 	 = 11, 
		.mask 	 = 0x06,
		.inuse   = 0,
		.type 	 = PIPE_UNKOWN ,
		.buftype	= EPCON_BUF_UNKOWN,
	},
	{//bulk 
		.index 	 = 12, 
		.mask 	 = 0x16,
		.inuse   = 0,
		.type 	 = PIPE_UNKOWN ,
		.buftype	 = EPCON_BUF_UNKOWN,
	},
#endif
};

#if 1
//DEFINE_TIMER(Send_token_timeout_timer, Send_token_timeout_timer_func, 0, 0);
//static token_timer_lock=0;
static void start_timer(){
	irq_cnt=0;
#if 0
	if(token_timer_lock == 0){
		token_timer_lock = 1;
		mod_timer(&Send_token_timeout_timer,jiffies+msecs_to_jiffies(1));// test for jjf
	}else{
		printk("<enter err branch >%s %d \n",__func__,__LINE__);
	}
#endif
}
//static int vector;
static void stop_timer(u8 irqvector){
	irq_cnt=0;
#if 0
	if(token_timer_lock == 1){
		token_timer_lock=0;
		del_timer(&Send_token_timeout_timer);
	//	vector = irqvector;
	}else{
		//printk("<enter err branch >%s %d %x %x\n",__func__,__LINE__,vector,irqvector);
	}
#endif
}

static void release_timer_lock(){
#if 0
	if(token_timer_lock == 1){
		token_timer_lock=0;
	}else{
		printk("<enter err branch >%s %d \n",__func__,__LINE__);
	}
#endif
}



static int ep_thread(void *__unused)
{
	/* khubd needs to be freezable to avoid intefering with USB-PERSIST
	 * port handover.  Otherwise it might see that a full-speed device
	 * was gone before the EHCI controller had handed its port over to
	 * the companion full-speed controller.
	 */
	
	int empty;
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	set_freezable();
	
	do{
#if 1
		
		start_schedule(acthcd,NULL);
		//empty = list_empty(&am7x_hcd_event_list);
		

		wait_event_freezable(am7x_hcd_wait,
				!list_empty(&am7x_hcd_event_list) ||
				kthread_should_stop());
		
#endif
	} while (!kthread_should_stop() || !list_empty(&acthcd->async));
	return 0;
}



static void kick_ep_thread(){
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);

	if(!list_empty(&acthcd->async)){
		if(list_empty(&am7x_hcd_event_list)){
			list_add(&am7x_hcd_event, &am7x_hcd_event_list);
		}
		wake_up(&am7x_hcd_wait);
	}
}


static u8 is_ep_busy(){
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	return acthcd->busy;
}

static void set_ep_busy(){
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	acthcd->busy=1;
}


static void set_ep_idle(){
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	acthcd->busy=0;
	kick_ep_thread();
}

static void set_ep_idle_simple(){
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	acthcd->busy=0;
	
}

static void Send_token_timeout_timer_func(unsigned long data){
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	struct aotg_hcep *ep;
	struct urb *urb;

	
	
	if(acthcd->cur_ep != NULL){
		
		if(ep_has_exist_async_list(acthcd->cur_ep,&acthcd->async)){
			
			ep = acthcd->cur_ep;
			if(!list_empty(&(ep->hep->urb_list))){
				
				urb= container_of(ep->hep->urb_list.next,struct urb,urb_list);
				
				if(urb != NULL){
					/*need reset cur ep ,but I don't know how to do this*/
					
					if(ep->index != 0){
						if(usb_pipeout(urb->pipe)){
							printk("<timeout_%d_%d_%d_%x_%x_%x_%x>\n",urb->num,usb_pipedevice(urb->pipe),usb_pipeout(urb->pipe),ep->reg_hcepcs,aotg_readb(ep->reg_hcepcs),aotg_readb(HCOUT04IEN),aotg_readb(HCOUT04IRQ));
						}else{
							printk("<timeout_%d_%d_%d_%x_%x_%x_%x>\n",urb->num,usb_pipedevice(urb->pipe),usb_pipeout(urb->pipe),ep->reg_hcepcs,aotg_readb(ep->reg_hcepcs),aotg_readb(HCIN04IEN),aotg_readb(HCIN04IRQ));
						}
						
						//printk("<timeout_%x_%x_%x_%x_%x_%x_%x>\n",ep->reg_hcepcon,aotg_readb(ep->reg_hcepcon),aotg_readb(ep->reg_hcin_auto_ctrl_up),aotg_readb(ep->reg_hcepbc),aotg_readb(HCIN04IRQ),aotg_readb(ep->reg_hceperr),aotg_readb(HCIN04IEN));
						//aotg_hcep_reset(ep->mask,ENDPRST_FIFOREST);
						
						//invalidate_ep(ep);
						//printk("<timeout_%d_%d_%d_%d_%d_%x_%x>\n",urb->num,usb_pipedevice(urb->pipe),usb_pipeendpoint(urb->pipe),usb_pipetype(urb->pipe),usb_pipeout(urb->pipe),ep->reg_hcepcs,aotg_readb(ep->reg_hcepcs));
					}
					set_ep_idle();
				}else{
					printk("<enter err branch >%s %d \n",__func__,__LINE__);	
				}
			}else{
				printk("<enter err branch >%s %d \n",__func__,__LINE__);
			}
		}else{
			/*the urb has been processed,do nothing,arm the thread for next transfer */
			printk("%s %d ep is disabled or urb is dequeued \n",__func__,__LINE__);
			set_ep_idle();

			//ep = acthcd->cur_ep;
			//urb= container_of(acthcd->cur_ep->hep->urb_list.next,struct urb,urb_list);;
			//if(urb != NULL){
				//set_ep_idle();
			//}
			//printk("timeout NO data transfer:%d %x %x!\n",urb->num,ep,urb);
		}
	}else{
		printk("<enter err branch >%s %d \n",__func__,__LINE__);
	}
	release_timer_lock();
	
}


static void dump_urb_info(){
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	struct aotg_hcep *ep;
	struct urb *urb;

	if(acthcd->cur_ep != NULL){
		
		if(ep_has_exist_async_list(acthcd->cur_ep,&acthcd->async)){
			
			ep = acthcd->cur_ep;
			if(!list_empty(&(ep->hep->urb_list))){
				
				urb= container_of(ep->hep->urb_list.next,struct urb,urb_list);
				
				if(urb != NULL){
					/*need reset cur ep ,but I don't know how to do this*/
					if(ep->index != 0){
						if(usb_pipeout(urb->pipe)){
							printk("<timeout_%d_%d_%d_%x_%x_%x_%x>\n",urb->num,usb_pipedevice(urb->pipe),usb_pipeout(urb->pipe),ep->reg_hcepcs,aotg_readb(ep->reg_hcepcs),aotg_readb(HCOUT04IEN),aotg_readb(HCOUT04IRQ));
						}else{
							printk("<timeout_%d_%d_%d_%x_%x_%x_%x>\n",urb->num,usb_pipedevice(urb->pipe),usb_pipeout(urb->pipe),ep->reg_hcepcs,aotg_readb(ep->reg_hcepcs),aotg_readb(HCIN04IEN),aotg_readb(HCIN04IRQ));
						}
					}else{
						printk("Time out ep0 urb num is:%d\n",urb->num);
					}
				}else{
					printk("<enter err branch >%s %d \n",__func__,__LINE__);	
				}
			}else{
				printk("<enter err branch >%s %d \n",__func__,__LINE__);
			}
		}else{
			/*the urb has been processed,do nothing,arm the thread for next transfer */
			printk("%s %d ep is disabled or urb is dequeued \n",__func__,__LINE__);
		}
	}else{
		printk("<enter err branch >%s %d \n",__func__,__LINE__);
	}
}

static void submit_ep(struct aotg_hcep	*ep)
{
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	//printk("%s %d num:%x\n",__func__,__LINE__,ep);
	if(ep_has_exist_async_list(ep,&acthcd->async)){
		//printk("submit_ep ep has exist!\n");
	}else{
#if 1
		if(ep->type == PIPE_INTERRUPT){
			if(acthcd->cur_ep != NULL){
				if(ep_has_exist_async_list(acthcd->cur_ep,&acthcd->async)){
					list_add(&ep->schedule,&acthcd->cur_ep->schedule);
					}else{
					list_add(&ep->schedule,&acthcd->async);
				}
			}
			else{
				list_add(&ep->schedule,&acthcd->async);
			}
		}else{
			list_add_tail(&ep->schedule, &acthcd->async);
		}
#else
		list_add_tail(&ep->schedule, &acthcd->async);
#endif
		
	}

	if(is_ep_busy()){
		//printk("The Current controler is busy!\n");
	}else{
		//printk("<enqueue_%d_end_%x>\n",urb_index,ep);
		kick_ep_thread();
	}
}

#endif



/*----------------------------------------------------------------------------------*/
#ifdef CONFIG_USB_HCD_DEBUG_FILES
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
static const char debug_filename[] = "driver/hcd";

static void dump_aotg_ep0_reg(struct seq_file *s)
{


	seq_printf(s,"<am7x_hcd>@@@--- ############################################\n");
	seq_printf(s,"<am7x_hcd>@@@---hcep0ctrl=%02x hcout0err=%02x hcin0err=%02x \n",
		aotg_readb(0xb00e00c0),aotg_readb(0xb00e00c1),aotg_readb(0xb00e00c3));
	seq_printf(s,"<am7x_hcd>@@@---hcout04errien=%02x HCIN04ERRIEN=%02x\n",aotg_readb(0xb00e01ba),aotg_readb(0xb00e01b8));
	seq_printf(s,"<am7x_hcd>@@@---hcin0bc=%02x hcout0bc=%02x hcep0cs=%02x\n",aotg_readb(0xb00e0000),aotg_readb(0xb00e0001),aotg_readb(0xb00e0002));
	seq_printf(s,"<am7x_hcd>@@@---HCIN04IEN=%02x HCOUT04IEN=%02x\n",aotg_readb(0xb00e0196),aotg_readb(0xb00e0194));
	seq_printf(s,"<am7x_hcd>@@@---HCIN04IRQ=%02x HCOUT04IRQ=%02x\n",aotg_readb(HCIN04IRQ),aotg_readb(HCOUT04IRQ));
}


static void dump_aotg_ep_reg(struct seq_file *s, struct aotg_hcep  *ep)
{
	int is_out = (ep->mask & USB_HCD_OUT_MASK) ? 1 : 0;
	seq_printf(s,"<am7x_hcd>@@@---############################################\n");
	seq_printf(s,"<am7x_hcd>@@@---ep->maxp=%d,ep type:%d,ep mask:%x,epnum:%x\n",ep->maxpacket,ep->type,
					ep->mask,ep->epnum);
	seq_printf(s,"<am7x_hcd>@@@---<HCEP CONFIG>config ep%d  %s mask is %x index is %d successfully\n"
		,ep->epnum,is_out?"OUT":"IN",ep->mask,ep->index);
	seq_printf(s,"<am7x_hcd>@@@---<HCEP CONFIG>REGISTER HCEPCON addr %lx val %02x\n",
			ep->reg_hcepcon,aotg_readb(ep->reg_hcepcon));	
	seq_printf(s,"<am7x_hcd>@@@---<HCEP CONFIG>REGISTER HCEPCS addr %lx val %02x\n",
			ep->reg_hcepcs,aotg_readb(ep->reg_hcepcs));
	seq_printf(s,"<am7x_hcd>@@@---<HCEP CONFIG>REGISTER HCEPBC addr %lx val %04x\n",
			ep->reg_hcepbc,aotg_readw(ep->reg_hcepbc));
	seq_printf(s,"<am7x_hcd>@@@---<HCEP CONFIG>REGISTER HCEPCTRL addr %lx val %02x\n",
			ep->reg_hcepctrl,aotg_readb(ep->reg_hcepctrl));
	seq_printf(s,"<am7x_hcd>@@@---<HCEP CONFIG>REGISTER HCEPMAXPACKET addr %lx val %04x\n",
			ep->reg_hcmaxpck,aotg_readw(ep->reg_hcmaxpck));
	seq_printf(s,"<am7x_hcd>@@@---<HCEP CONFIG>REGISTER HCEPADDR addr %lx val %08x\n",
			ep->reg_hcepaddr,aotg_readl(ep->reg_hcepaddr));
}


static int proc_acthcd_show(struct seq_file *s, void *unused)
{
	struct aotg_hcd	*acthcd = s->private;
	struct aotg_hcep	*ep;
	int i;
	
	seq_printf(s,"\n<am7x_hcd>@@@---********************************\n");
	seq_printf(s, "\n<am7x_hcd>@@@---%s: version %s\n", hcd_name, DRIVER_VERSION);
	seq_printf(s, "\n<am7x_hcd>@@@---hcdspeed:%d\n",acthcd->speed);
	seq_printf(s, "\n<am7x_hcd>@@@---portstate:%d\n",acthcd->portstate);
	seq_printf(s, "\n<am7x_hcd>@@@---usedma:%d\n",use_dma);
	
	/* don't access registers when interface isn't clocked */
	dump_aotg_ep0_reg(s);
	for(i = 1;i < ARRAY_SIZE(hcep_map);i++){
		ep = &(hcep_map[i]);
		dump_aotg_ep_reg(s,ep);
	}
	
	seq_printf(s," \n<am7x_hcd>@@@---********************************\n");
	return 0;
}

static int proc_acthcd_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_acthcd_show, PDE(inode)->data);
}


static const struct file_operations proc_ops = {
	.owner		= THIS_MODULE,
	.open		= proc_acthcd_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release		= single_release,
};

static void create_debug_file(struct aotg_hcd *acthcd)
{
	acthcd->pde = proc_create_data(debug_filename, 0, NULL, &proc_ops, acthcd);
}

static void remove_debug_file(struct aotg_hcd *acthcd)
{
	if (acthcd->pde)
		remove_proc_entry(debug_filename, NULL);
}
#else
static inline void create_debug_file(struct aotg_hcd *acthcd) {}
static inline void remove_debug_file(struct aotg_hcd *acthcd) {}
#endif


#ifdef DEBUG_HCD
static const char* show_hub_req(u16 typereq)
{
	switch(typereq){
		case ClearHubFeature: 
			return  "ClearHubFeature";
		case SetHubFeature:	
			return "SetHubFeature";
		case GetHubDescriptor:
			return "GetHubDescriptor";
		case GetHubStatus: 	
			return "GetHubStatus";
		case ClearPortFeature: 
			return "ClearPortFeature";
		case GetPortStatus:	
			return "GetPortStatus";
		case SetPortFeature:	
			return "SetPortFeature";
		default:				
			return "unknow port ctrlreq";
	}
}
#else
static const char* show_hub_req(u16 typereq){return NULL;}
#endif


static void modify_hcep_buftype(int epnums)
{
	int i,totalnums ;

	totalnums = ARRAY_SIZE(hcep_map);
	if(epnums >= totalnums){
		INFO("<am7x_hcd>@@@---<WARN>EP more than we can support!!");
		return;
	}else if(epnums <= totalnums/2){
		for(i=0;i < epnums; i++)
			hcep_map[i+1].buftype = EPCON_BUF_QUAD;	
	}else
		hcep_map[1].buftype = EPCON_BUF_QUAD;	
}

static int aotg_reinit_hcep(struct aotg_hcep* ep,int is_out)
{
	int ep_num = ep->mask & 0xf;
	if(ep->type ==PIPE_CONTROL) 
		return -1;
	
#ifdef USE_EPREG_EX		
	ep->reg_hcin_auto_ctrl_usb 	= get_hcin_auto_reg(is_out, 0, HCIN1AUTOCTLUSB, ep_num);
	ep->reg_hcin_frmpck_nb		= get_hcin_auto_reg(is_out, 0, HCIN1FRMPCKNB, ep_num);
	ep->reg_hcin_start_frm		= get_hcin_auto_reg(is_out, 0, HCIN1STARTFRM, ep_num);
	ep->reg_hcin_frm_interval		= get_hcin_auto_reg(is_out, 0, HCIN1FRMINTERVAL, ep_num);
	ep->reg_hcin_sumpck_nb		= get_hcin_auto_reg(is_out, 0, HCIN1SUMPCKNB, ep_num);
	ep->reg_hcin_sumpck_rcnt		= get_hcin_auto_reg(is_out, 0, HCIN1SUMPCKRCNT, ep_num);
#endif
	ep->reg_hcepcon 				= get_hcepcon_reg(is_out, HCOUT1CON, HCIN1CON, ep_num);
	ep->reg_hcepcs 				= get_hcepcs_reg(is_out,HCOUT1CS,HCIN1CS,ep_num);
	ep->reg_hcepbc 				= get_hcepbc_reg(is_out, HCOUT1BCL, HCIN1BCL, ep_num);
	ep->reg_hcepctrl				= get_hcepctrl_reg(is_out, HCOUT1CTRL, HCIN1CTRL, ep_num);
	ep->reg_hcmaxpck 			= get_hcepmaxpck_reg(is_out, HCOUT1MAXPACKL, HCIN1MAXPCKL, ep_num);
	ep->reg_hcepaddr 			= get_hcepaddr_reg(is_out,IN1STARTADDRESS,OUT1STARTADDRESS,ep_num);
	ep->reg_hceperr 				= get_hceperr_reg(is_out,HCOUT1ERR,HCIN1ERR,ep_num);
	ep->reg_hcin_auto_ctrl_up		= get_hcin_auto_reg(is_out, 0, HCIN1AUTOCTLUP, ep_num);
	ep->reg_hcfifo 				= get_hcfifo_reg(FIFO1DAT0, ep_num);

	ep->mask = (is_out<<4) | (ep->mask);
	return 0;
}


static void aotg_init_hcep(void)
{
	struct aotg_hcep *ep;
	int i, is_out,ep_num;
	
	for(i = 0;i < ARRAY_SIZE(hcep_map);i++){
		ep = &(hcep_map[i]);	
		if(ep->type ==PIPE_CONTROL) 
			continue;
			
		is_out  	= (ep->mask  & USB_HCD_OUT_MASK) ? 1 : 0;
		ep_num	= ep->mask  & 0x0f;
		
#ifdef USE_EPREG_EX		
		ep->reg_hcin_auto_ctrl_usb 	= get_hcin_auto_reg(is_out, 0, HCIN1AUTOCTLUSB, ep_num);
		ep->reg_hcin_frmpck_nb		= get_hcin_auto_reg(is_out, 0, HCIN1FRMPCKNB, ep_num);
		ep->reg_hcin_start_frm		= get_hcin_auto_reg(is_out, 0, HCIN1STARTFRM, ep_num);
		ep->reg_hcin_frm_interval		= get_hcin_auto_reg(is_out, 0, HCIN1FRMINTERVAL, ep_num);
		ep->reg_hcin_sumpck_nb		= get_hcin_auto_reg(is_out, 0, HCIN1SUMPCKNB, ep_num);
		ep->reg_hcin_sumpck_rcnt		= get_hcin_auto_reg(is_out, 0, HCIN1SUMPCKRCNT, ep_num);
#endif
		ep->reg_hcepcon 				= get_hcepcon_reg(is_out, HCOUT1CON, HCIN1CON, ep_num);
		ep->reg_hcepcs 				= get_hcepcs_reg(is_out,HCOUT1CS,HCIN1CS,ep_num);
		ep->reg_hcepbc 				= get_hcepbc_reg(is_out, HCOUT1BCL, HCIN1BCL, ep_num);
		ep->reg_hcepctrl				= get_hcepctrl_reg(is_out, HCOUT1CTRL, HCIN1CTRL, ep_num);
		ep->reg_hcmaxpck 			= get_hcepmaxpck_reg(is_out, HCOUT1MAXPACKL, HCIN1MAXPCKL, ep_num);
		ep->reg_hcepaddr 			= get_hcepaddr_reg(is_out,IN1STARTADDRESS,OUT1STARTADDRESS,ep_num);
		ep->reg_hceperr 				= get_hceperr_reg(is_out,HCOUT1ERR,HCIN1ERR,ep_num);
		ep->reg_hcin_auto_ctrl_up		= get_hcin_auto_reg(is_out, 0, HCIN1AUTOCTLUP, ep_num);
		ep->reg_hcfifo 				= get_hcfifo_reg(FIFO1DAT0, ep_num);
	}
}





static struct aotg_hcep * find_an_idle_hcep(int type,int dir)
{	
	int i, is_out ;
	struct aotg_hcep *ep ;
	struct aotg_hcep *paired_ep;
		printk("%s %d %x %x\n",__func__,__LINE__,type,dir);
	if(type == PIPE_CONTROL) return &(hcep_map[0]);
	for(i = 1;i < ARRAY_SIZE(hcep_map);i++){	
		ep =&(hcep_map[i]);	
		is_out = (ep->mask & USB_HCD_OUT_MASK) ? 1 : 0;

		if((ep->inuse) ||(is_out != dir )|| ((ep->type != PIPE_UNKOWN)&&(ep->type != type))) 
				continue;


		if(is_out){
			paired_ep=ep-1;
		}else{
			paired_ep=ep+1;
		}
		if(ep->type == PIPE_UNKOWN && paired_ep->type == PIPE_UNKOWN){
			ep->type = paired_ep->type = type;
			ep->buftype=paired_ep->buftype=EPCON_BUF_SINGLE;
		}

		


		printk("find_an_idle_hcep:type:%x mask:%x\n",ep->type,ep->mask);
		printk("find_an_idle_hcep:type1:%x mask1:%x\n",paired_ep->type,paired_ep->mask);
		
		ep->inuse = 1;

		return ep;
	}	
	return NULL;
}



static struct aotg_hcep * find_an_idle_hcep_next(int type,int dir)
{	
	int i, is_out ;
	struct aotg_hcep *ep ;
		printk("%s %d \n",__func__,__LINE__);
	if(type == PIPE_CONTROL) return &(hcep_map[0]);
	for(i = 1;i < ARRAY_SIZE(hcep_map);i++){	
		ep =&(hcep_map[i]);	
		is_out = (ep->mask & USB_HCD_OUT_MASK) ? 1 : 0;

		if((is_out != dir )|| ((ep->type != PIPE_UNKOWN)&&(ep->type != type))) 
				continue;
		return ep;
	}	
	return NULL;
}





static struct aotg_hcep * find_an_idle_hcep_reserve(){
	int i;
	struct aotg_hcep *ep ;
		printk("%s %d \n",__func__,__LINE__);
	for(i=0;i<ARRAY_SIZE(hcep_map_reserv);i++){
		ep =&(hcep_map_reserv[i]);	
		if(ep->inuse)
			continue;

		ep->inuse = 1;
		return ep;
	}
	return NULL;
}



static struct aotg_hcep* get_matched_hcep(int epmask)
{
	int i;
	struct aotg_hcep * ep = NULL;
		printk("%s %d \n",__func__,__LINE__);
	for(i = 1;i < ARRAY_SIZE(hcep_map);i++){	
		ep = &hcep_map[i];
		if(ep->mask == epmask)
			break;
	}
	return ep;
}



/*--------------------------------------------------------------------------*/
static  void sofirq_on(struct aotg_hcd *acthcd,struct aotg_hcep *ep)
{
	if (!acthcd->periodic_count)
		return;
	
	DMSG_HCD("<am7x_hcd>@@@---sof irq on\n");
	//validate_ep(ep);
	aotg_setbits(USBIEN_SOF, USBIEN);
}


static void sof_enable(){
	aotg_setbits(USBIEN_SOF, USBIEN);
}

static void sof_disable(){
	aotg_clearbits(USBIEN_SOF, USBIEN);
}


static  void sofirq_off(struct aotg_hcd *acthcd,struct aotg_hcep *ep)
{
	if (acthcd->periodic_count)
		return;
	
	DMSG_HCD("<am7x_hcd>@@@---ep%d before inval\n",ep->index);
	invalidate_ep(ep);
	
	aotg_clearbits(USBIEN_SOF, USBIEN);
	DMSG_HCD("<am7x_hcd>@@@---sof_off\n");
}


static void invalidate_ep(struct aotg_hcep *ep)
{
	u8  reg_val;
	 reg_val = aotg_readb(ep->reg_hcepcon);
	while( reg_val & EPCON_VAL) {
		aotg_writeb(reg_val &~EPCON_VAL,ep->reg_hcepcon);
		reg_val = aotg_readb(ep->reg_hcepcon);
	}	
}

static void invalidate_ep_reg(unsigned long reg_hcepcon )
{
	u8  reg_val;
	reg_val = aotg_readb(reg_hcepcon);
	while( reg_val & EPCON_VAL) {
		aotg_writeb(reg_val &~EPCON_VAL,reg_hcepcon);
		reg_val = aotg_readb(reg_hcepcon);
	}	
}




static void validate_ep(struct aotg_hcep *ep)
{
	u8  reg_val ;
	reg_val = aotg_readb(ep->reg_hcepcon);
	while ( (reg_val & EPCON_VAL) == 0) {
		aotg_writeb(reg_val |EPCON_VAL,ep->reg_hcepcon);
		mb();
		reg_val = aotg_readb(ep->reg_hcepcon);
		mb();
	}
}


static inline void pio_irq_disable(u8 mask)
{
	u8  is_out = mask & USB_HCD_OUT_MASK;
	u8 ep_num = mask & 0x0f;
	u32 reg = is_out ? HCOUT04IEN : HCIN04IEN;
	DMSG_DMA("<am7x_hcd>@@@---<PIO_IRQ_DISABLE> ep mask %02x pio irq disable\n",mask);
	aotg_clearbits(1<<ep_num,reg);
}


static inline void pio_irq_enable(u8 mask)
{
	u8  is_out = mask & USB_HCD_OUT_MASK;
	u8 ep_num = mask & 0x0f;
	u32 reg = is_out ? HCOUT04IEN : HCIN04IEN;
	DMSG_DMA("<am7x_hcd>@@@---<PIO_IRQ_ENABLE> ep mask %02x  pio irq enable\n",mask);
	aotg_setbits(1<<ep_num,reg);
}



static inline void pio_irq_clear(u8 mask)
{
	u8  is_out = mask & USB_HCD_OUT_MASK;
	u8 ep_num = mask & 0x0f;
	u32 reg = is_out ? HCOUT04IRQ : HCIN04IRQ;
	aotg_writeb(1<<ep_num, reg);
}



static void aotg_hcep_reset(u8 ep_mask,u8 type_mask)
{
	u8 val;
	DMSG_HCD("<am7x_hcd>@@@---<HCEP_RESET>ep mask =  %d\n",ep_mask);
	aotg_writeb(ep_mask,ENDPRST); /*select ep*/
	val = ep_mask |type_mask;
	aotg_writeb(val,ENDPRST);/*reset ep*/
}


static void finish_periodic_request(
	struct aotg_hcd	*acthcd,
	struct aotg_hcep	*ep,
	struct urb		*urb
) {
	unsigned		i;
	unsigned stopped = ep->stopped;


	if (urb->status == -EINPROGRESS)
		urb->status = 0;
	
	/*reset frame index to process iso transfer properly*/
	urb->hcpriv = NULL;
	ep->stopped = 1;
	ep->fmindex = 0;	
	usb_hcd_unlink_urb_from_ep(aotg_to_hcd(acthcd),urb);	
	usb_hcd_giveback_urb(aotg_to_hcd(acthcd), urb,urb->status);
	ep->stopped = stopped;
	
	/* leave active endpoints in the schedule */
	if (!list_empty(&ep->hep->urb_list)) 
		return;
	
	/* periodic deschedule */
	//INFO("deschedule epnum %d/period %d branch %d\n", ep->epnum, ep->period, ep->branch);
	for (i = ep->branch; i < PERIODIC_SIZE; i += ep->period) {
		struct aotg_hcep	*temp;
		struct aotg_hcep	**prev = &acthcd->periodic[i];
		while (*prev && ((temp = *prev) != ep))
			prev = &temp->next;
		
		if (*prev)
			*prev = ep->next;
		acthcd->load[i] -= ep->load;
	}
	
	ep->branch = PERIODIC_SIZE;

    if(acthcd->periodic_count > 0)
        acthcd->periodic_count--;

	aotg_to_hcd(acthcd)->self.bandwidth_allocated -= ep->load / ep->period;
	//if (ep == acthcd->next_periodic)
	//	acthcd->next_periodic = ep->next;
	sofirq_off(acthcd,ep);
}




static void finish_unspport_urb_request(struct aotg_hcd	*acthcd,struct urb		*urb,int status){
printk("%s %d \n",__func__,__LINE__);
	urb->status = status;
	usb_hcd_unlink_urb_from_ep(aotg_to_hcd(acthcd),urb);
	usb_hcd_giveback_urb(aotg_to_hcd(acthcd), urb,urb->status);
}



static void finish_async_request(
	struct aotg_hcd	*acthcd,
	struct aotg_hcep	*ep,
	struct urb		*urb
) 
{
	unsigned stopped = ep->stopped;
	u8 errtype;


	//printk("finish ep%d\n",ep->index);

	if (usb_pipecontrol(urb->pipe))
		ep->nextpid = USB_PID_SETUP;
	else if((errtype = get_ep_errtype(ep->reg_hceperr)) !=  EPERR_NONE ){
		INFO("<am7x_hcd>@@@---hcep transfer error,errtype:%d ep->num=%d\n",errtype,ep->epnum);
		if(errtype == EPERR_STALL){
			acthcd->peer_flags |= PEER_STALLED;
			urb->status = -EPIPE;
			ep->stalled = 1;
		}else if(errtype==EPERR_TIMEOUT)	
			urb->status = -ETIMEDOUT;
		//else
		//	urb->status == -EINPROGRESS;
//printk(">>>>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
//		aotg_hcep_reset(ep->mask,ENDPRST_FIFOREST |ENDPRST_TOGRST); 
//printk("<<<<<<<<<<<<<<<<<<<%s %d\n",__func__,__LINE__);
	}	

	if (urb->status == -EINPROGRESS)
		urb->status = 0;

	if(urb->hcpriv != NULL){
		urb->hcpriv = NULL;	
	}else{
		printk("urb->hcpriv is NULL!\n");
		return;//如果这个urb 已经被释放过了 就不要在释放了
	}
	
	
	ep->stopped = 1;
	
	usb_hcd_unlink_urb_from_ep(aotg_to_hcd(acthcd),urb);
	
	usb_hcd_giveback_urb(aotg_to_hcd(acthcd), urb,urb->status);
	ep->stopped = stopped;
	/* take idle endpoints out of the schedule */
	if (!list_empty(&ep->hep->urb_list)){
		//printk(KERN_INFO "<%s %d> ep urb_list is not empty\n",__FUNCTION__,__LINE__,ep->hep->urb_list);
		//set_ep_idle();
		return;
	}

	/* async deschedule */
	if (!list_empty(&ep->schedule)) {
		//printk(KERN_INFO "<%s %d>debug==========\n",__FUNCTION__,__LINE__);
		
		list_del_init(&ep->schedule);
		
		//if(ep->index != 0)
			//invalidate_ep(ep);
		//if(ep->type == PIPE_BULK){
		//	invalidate_ep(ep);
		//	pio_irq_disable(ep->mask);
		//}	
		//printk(KERN_INFO "<%s %d>debug----------\n",__FUNCTION__,__LINE__);
		//set_ep_idle();
		return;
	}

	//printk(KERN_INFO "<%s %d>debug----------\n",__FUNCTION__,__LINE__);
	//set_ep_idle();
}

static void finish_request(
	struct aotg_hcd	*acthcd,
	struct aotg_hcep	*ep,
	struct urb		*urb
) 
{

	DMSG_XFERS("<am7x_hcd>@@@---fin_urb:ep=%d %p %d\n",ep->epnum,urb,urb->actual_length);
	if ( ep->period)
		finish_periodic_request(acthcd,ep,urb);
	else
		finish_async_request(acthcd,ep,urb);
}



static inline void port_reset(void)
{
	aotg_setbits(0x80,VDCTRL);
	aotg_writeb(0x60,HCPORTCTRL);/*portrst & 55ms*/
	mb();
	DMSG_HCD("<am7x_hcd>@@@---<PORT_RESET>\n");
}



static inline void ep_setup(struct aotg_hcep  *ep,u8 type,u8 buftype)
{

	ep->buftype = buftype;
	DMSG_HCD("<am7x_hcd>@@@---<EP_SETUP> ep:%p, type:%d, buffer type = %d\n",ep,type,buftype);
#if 1  //to config ep3 buffer ---quad buffer(ep x in control register bit[0:1])
	aotg_writeb(type |buftype,ep->reg_hcepcon);
#else
	if(ep->epnum!=3)
		aotg_writeb(type |buftype,ep->reg_hcepcon);
	else
		aotg_writeb(0x03|type |buftype,ep->reg_hcepcon);
#endif
	printk("[%s %d]ep_con:0x%02x\n",__func__,__LINE__,aotg_readb(ep->reg_hcepcon));
	mb();
}

static int aotg_hcep_config(
	struct aotg_hcd *acthcd,
	struct aotg_hcep *ep,
	u8 buftype,
	int is_out
){
	u8 type;
	u32 fifo_addr = acthcd->fifo_addr;
	u32 fifo_len;
	
	if(aotg_reinit_hcep(ep,is_out)<0){
		ERR("<am7x_hcd>@@@---hcep is control ep,no need to config\n");
		return -EINVAL;
	}

	invalidate_ep(ep);

	if(ep->maxpacket<64){
		fifo_len = 	(buftype+1)*64;
	}else{
		fifo_len = (buftype+1)*ep->maxpacket;
	}
	

	
	/*caculate fifo addr*/
#if 1 //-- bq add for test.
	printk(KERN_INFO "%s %s %d ep_num=0x%x\n",__FILE__,__FUNCTION__,__LINE__,ep->epnum);
	printk(KERN_INFO "%s %s %d ep_mask=0x%x\n",__FILE__,__FUNCTION__,__LINE__,ep->mask);
	printk(KERN_INFO "%s %s %d ep_type=0x%x\n",__FILE__,__FUNCTION__,__LINE__,ep->type);
	printk(KERN_INFO "%s %s %d fifo_addr=0x%x\n",__FILE__,__FUNCTION__,__LINE__,fifo_addr);
	printk(KERN_INFO "%s %s %d buftype=%d\n",__FILE__,__FUNCTION__,__LINE__,buftype);
	printk(KERN_INFO "%s %s %d ep_maxpacket=%d\n",__FILE__,__FUNCTION__,__LINE__,ep->maxpacket);
	printk(KERN_INFO "%s %s %d len=0x%x\n",__FILE__,__FUNCTION__,__LINE__,fifo_addr + fifo_len);
	printk(KERN_INFO "%s %s %d fifo_end=0x%x\n",__FILE__,__FUNCTION__,__LINE__,HC_FIFO_END_ADDR);
	printk("<HCEP CONFIG>REGISTER HCEPCS addr %x val %02x\n",
			ep->reg_hcepcs,aotg_readb(ep->reg_hcepcs));
#endif
	


	if(fifo_addr + fifo_len > HC_FIFO_END_ADDR ){
		ERR("<am7x_hcd>@@@---hcep fifo is not enough\n");
		return -ENOMEM;
	}
	
	switch(ep->type){
	case PIPE_BULK:
		type = EPCON_TYPE_INT;
		break;
	case PIPE_ISOCHRONOUS:
		type = EPCON_TYPE_ISO;
		break;	
	case PIPE_INTERRUPT:
		type = EPCON_TYPE_INT;
		break;	
	default:
		ERR("<am7x_hcd>@@@---invalid ep type");
		return -EINVAL;
	}
	
	/*allocate buffer address of ep fifo*/
	ep->fifo_addr = fifo_addr;
	aotg_writel(fifo_addr,ep->reg_hcepaddr);
	aotg_writew(ep->maxpacket,ep->reg_hcmaxpck);
	ep_setup(ep,type,buftype);

	
	pio_irq_disable(ep->mask);

	/*reset this ep*/
	aotg_hcep_reset(ep->mask,ENDPRST_FIFOREST |ENDPRST_TOGRST);
	
	/*assign the target ep num*/
	aotg_writeb(ep->epnum, ep->reg_hcepctrl);
	if(!is_out){
		DMSG_HCD("<am7x_hcd>@@@---prepare to set hcep ctrl up\n");	
		aotg_clearbits(1<<0,ep->reg_hcin_auto_ctrl_up);
		DMSG_HCD("<am7x_hcd>@@@---ctrl_up:%x\n",aotg_readb(ep->reg_hcin_auto_ctrl_up));	
	}

	/*hook ep to ep_list*/
	acthcd->ep_list[ep->index] = ep; // 防止ep 自动发送intoken的时候 出错
	#if 1   //to change ep 3 buffer size -- double <---->quad bufer ,so we shouble caculate fifo start addr for the next ep 
	acthcd->fifo_addr = fifo_addr +  (buftype+1)*ep->maxpacket;
	#else
		if(ep->epnum!=3)
			acthcd->fifo_addr = fifo_addr +  fifo_len;
		else
			acthcd->fifo_addr = fifo_addr +  (buftype+3)*ep->maxpacket;
	#endif
#if 1	
	printk("############################################\n");
	printk("ep->maxp=%d,ep type:%d,ep mask:%x,epnum:%x\n",ep->maxpacket,ep->type,
					ep->mask,ep->epnum);
	printk("<HCEP CONFIG>config ep%d  %s mask is %x index is %d successfully\n"
		,ep->epnum,is_out?"OUT":"IN",ep->mask,ep->index);
	printk("<HCEP CONFIG>REGISTER HCEPCON addr %x val %02x\n",
			ep->reg_hcepcon,aotg_readb(ep->reg_hcepcon));	
	printk("<HCEP CONFIG>REGISTER HCEPCS addr %x val %02x\n",
			ep->reg_hcepcs,aotg_readb(ep->reg_hcepcs));
	printk("<HCEP CONFIG>REGISTER HCEPBC addr %x val %04x\n",
			ep->reg_hcepbc,aotg_readw(ep->reg_hcepbc));
	printk("<HCEP CONFIG>REGISTER HCEPCTRL addr %x val %02x\n",
			ep->reg_hcepctrl,aotg_readb(ep->reg_hcepctrl));
	printk("<HCEP CONFIG>REGISTER HCEPMAXPACKET addr %x val %04x\n",
			ep->reg_hcmaxpck,aotg_readw(ep->reg_hcmaxpck));
	printk("<HCEP CONFIG>REGISTER HCEPADDR addr %x val %08x\n",
			ep->reg_hcepaddr,aotg_readl(ep->reg_hcepaddr));
	printk("############################################\n");
#endif
	return 0;
}


static inline void handle_status(int is_out)
{ 
	/*status always DATA1,set 1 to ep0 toggle */
	aotg_setbits(EP0CS_HCSETTOOGLE,HCEP0CS);
	mb();
	
	if(is_out){
		aotg_writeb(0,HCIN0BC);/*control wirte or no-data*/
	}
	else
		aotg_writeb(0,HCOUT0BC);/*control read*/
	mb();
	
	DMSG_CTRL("<am7x_hcd>@@@---<CTRL> STATUS stage start\n");
}


static  int  
write_hcep0_packet(struct urb *urb,  int max)
{
	u32		*buf;
	unsigned long ptr;
	unsigned	length, count;
	unsigned long addr = HCEP0OUTDAT0;

	ptr = (unsigned long)(urb->transfer_buffer + urb->actual_length);	
	buf = (u32 *)(urb->transfer_buffer + urb->actual_length);
	length = min(max,urb->transfer_buffer_length - urb->actual_length);

	/*wirte in DWORD*/
	count = length>>2; 
	if((length&0x03)!= 0)
		count ++;
	
	while (count--) {
		__aotg_writel(*buf,addr);
		buf ++;
		addr += 4;	
	}
	
	aotg_writeb(length,HCOUT0BC);/*arm HC OUT EP0 for the next OUT transaction*/
	return length;
}


static void read_hcep0_fifo(struct aotg_hcep *ep, struct urb *urb)
{
	u8		*buf, byte;
	unsigned   overflag;
	unsigned	length, count;
	struct usb_device *udev;
	unsigned long addr = HCEP0INDAT0;
	

	udev = urb->dev;
	
	if(aotg_readb(HCEP0CS) & EP0CS_HCOUTBSY) {
		printk("<am7x_hcd>@@@---<CTRL>IN data is not ready\n");
		return ;
	}else {
		usb_dotoggle(udev, ep->epnum, 0);
		buf = urb->transfer_buffer + urb->actual_length;

		length = count = aotg_readb(OUT0BC);
		if(length > ep->length) {
			count = ep->length;
			urb->status = -EOVERFLOW;
			overflag = 1;
		}

		urb->actual_length += count;
		while(count--) {
			byte = aotg_readb(addr);
			*buf++ = byte; 
			addr++; 
		}

		if(urb->actual_length == urb->transfer_buffer_length) {
			ep->nextpid = USB_PID_ACK;
		}else if(length < ep->maxpacket) {
			if (urb->transfer_flags & URB_SHORT_NOT_OK) {
				urb->status = -EREMOTEIO;
	        }
			ep->nextpid = USB_PID_ACK;
		}
	}
}


static void write_hcep0_fifo(struct aotg_hcep *ep,struct urb *urb)
{
	unsigned  count;
	count = write_hcep0_packet(urb,ep->maxpacket);
	ep->length = count;
	DMSG_CTRL("#<am7x_hcd># <CTRL> OUT write %dB to fifo,wait ACK\n",ep->length);
}


static int write_packet (struct aotg_hcep *ep, struct urb *urb, int max)
{
	u32		*buf;
	int	length, count;

	buf = (u32 *)(urb->transfer_buffer + urb->actual_length);
	length = min(urb->transfer_buffer_length - urb->actual_length,max);
	ep->length = length;

	if ( length <0 )
		ERR("<am7x_hcd>@@@---%s:length error%d\n",__FUNCTION__,length);

	//printk("write_packet:len:%d tans_len:%d ep_len:%d\n",urb->transfer_buffer_length,\
	//	urb->actual_length,ep->maxpacket);


	if(length == 0){
		printk("<jjf_debug>write_packet :has send all packet\n");
		return length;
	}
		

	/*wirte in DWORD*/
	count = length >> 2; 
	if((length & 0x03)!= 0)
		count ++;
	
	while (count--) {
		__aotg_writel(*buf,ep->reg_hcfifo);
		buf ++;
	}

	/*byte count low resgister + byte count high register <<8;
	*tell the hw how many bytes is loaded actually
	*/
	aotg_writeb(length&0xff,ep->reg_hcepbc);	
	aotg_writeb(length>>8,ep->reg_hcepbc+1);	
	//aotg_writew(length,ep->reg_hcepbc);	/*tell the hw how many bytes is loaded actually*/
	//aotg_writeb(0,ep->reg_hcepcs); 		/*arm HC OUT EP for the next OUT transaction*/
	return length;
}


static int write_packet_busy (struct aotg_hcep *ep, struct urb *urb, int max)
{
	u32		*buf;
	int	length, count;

	buf = (u32 *)(urb->transfer_buffer + urb->actual_length);
	length = min(urb->transfer_buffer_length - urb->actual_length,max);
	ep->length = length;

	if ( length <0 )
		ERR("<am7x_hcd>@@@---%s:length error%d\n",__FUNCTION__,length);

	//printk("write_packet:len:%d tans_len:%d ep_len:%d\n",urb->transfer_buffer_length,\
	//	urb->actual_length,ep->maxpacket);


	//if(length == 0){
		//printk("<jjf_debug>write_packet :has send all packet\n");
		//return length;
	//}
		

	/*wirte in DWORD*/
	//count = length >> 2; 
	//if((length & 0x03)!= 0)
		//count ++;
	
	//while (count--) {
		//__aotg_writel(*buf,ep->reg_hcfifo);
		//buf ++;
	//}

	/*byte count low resgister + byte count high register <<8;
	*tell the hw how many bytes is loaded actually
	*/
	aotg_writeb(length&0xff,ep->reg_hcepbc);	
	aotg_writeb(length>>8,ep->reg_hcepbc+1);	
	//aotg_writew(length,ep->reg_hcepbc);	/*tell the hw how many bytes is loaded actually*/
	//aotg_writeb(0,ep->reg_hcepcs); 		/*arm HC OUT EP for the next OUT transaction*/
	return length;
}



static int is_iso_endpoint(struct aotg_hcep *ep)
{
	return ep->type == PIPE_ISOCHRONOUS;
}


static int is_iso_urb_over(struct aotg_hcep *ep, struct urb *urb)
{
	if ( ep->fmindex >= urb->number_of_packets)
		return 1;
	else
		return 0;
}

/*set current frame offset ,and return the frame number to be updated*
*This function is called before unloading fifo data into urb->transfer_buf*
*/
static int iso_urb_start_update(struct aotg_hcep *ep, struct urb *urb)
{
	int idx;
	unsigned int offset = 0;
	struct usb_iso_packet_descriptor *iso_desc;
	
	for (idx=0;idx<urb->number_of_packets; idx++){
		iso_desc = &urb->iso_frame_desc[idx];
		if ( !iso_desc ){
			ERR("<am7x_hcd>@@@---%s:%d bug not iso?\n", __func__, __LINE__);
			return -1;
		}
		/*FIXME: maybe zero data recieved in the 1st frame*/
		if (idx == ep->fmindex) {
			iso_desc->offset = offset;
			return idx;
		}
		offset += iso_desc->actual_length;
	}

	DMSG_HCD("<am7x_hcd>@@@---%s:%d bug ep->idx=%d\n", __FUNCTION__, __LINE__,ep->fmindex);
	return -1;
}

/*set actual_length and status field of the frame 'frame'
  *This function is called after unloading fifo data into urb->transfer_buf
*/
static int iso_urb_update(struct aotg_hcep *ep, struct urb *urb, int count)
{
	int idx = ep->fmindex;
	struct usb_iso_packet_descriptor *iso_desc;
	unsigned char *buf;
	unsigned long frm,ufrm;

	if ( idx >= urb->number_of_packets ){
		DMSG_HCD("<am7x_hcd>@@@---%s:%d fmindex overflow\n",__FUNCTION__, __LINE__);
		return -1;
	}

	iso_desc = &urb->iso_frame_desc[idx];
	if ( count > iso_desc->length)
		DMSG_HCD("<am7x_hcd>@@@---%s:%d warning \n",__FUNCTION__, __LINE__);
	
	buf = urb->transfer_buffer + iso_desc->offset;
	iso_desc->actual_length = count;

	ufrm = aotg_readw(AOTG_REG(0x1ac));
	frm = (ufrm >> 3)&0x7ff;
	ufrm = ufrm & 0x7;

	iso_desc->status = 0;
	ep->fmindex++;
	return 0;
}



static int  read_hcep_fifo(struct aotg_hcd *acthcd,struct aotg_hcep *ep, struct urb *urb)
{
	u8           *bytebuf,byte;
	u32		*buf;
	unsigned	 count,remain,length;
	unsigned  overflag,is_short,shorterr,is_last;
	u32 		 dword,fifoaddr;
	struct usb_device *udev;
	int which_align,i;

	is_short = is_last = 0;
	overflag = shorterr = 0;

	//printk("%s %d %x \n",__func__,__LINE__,aotg_readb(ep->reg_hcepcs));

	
	if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)){
		udev = urb->dev;	
		usb_dotoggle(udev, ep->epnum, 0);

		bytebuf = (u8*)(urb->transfer_buffer + urb->actual_length); 
		ep->length = min((int)ep->maxpacket,urb->transfer_buffer_length - urb->actual_length);
		length = count = aotg_readw(ep->reg_hcepbc);


		

		
		if(length > ep->length) {
			DMSG_HCD("<am7x_hcd>@@@---act_cnt=%d, req_count=%d\n",count,ep->length);
			count = ep->length;
			urb->status = -EOVERFLOW;
			overflag = 1;
		}

	//	usb_dotoggle(udev, ep->epnum, 0);
		if (is_iso_endpoint(ep)){
			iso_urb_start_update(ep,urb);
		}

		/*read in DWORD*/
		which_align = ((unsigned long)bytebuf & 0x3UL);
		remain = count & 0x03;
		count = count>>2; 
		urb->actual_length += 4*count;

		if (likely(which_align==0)){
			buf = (u32*)bytebuf;	
			while(count--) {
				*buf++  = __aotg_readl(ep->reg_hcfifo);
			}
		}else if (which_align==2){    
		      unsigned short * ppp;
		      ppp = (unsigned short *)bytebuf; 	  
                     while(count--) {
			  dword = aotg_readl(ep->reg_hcfifo); 
                        *ppp++ = ((unsigned short)dword&0xffff);  
			  *ppp++ = ((unsigned short)(dword>>16)&0xffff);
		    }    
		}else{
                     unsigned char * ppp;
		      ppp = (unsigned char *)bytebuf; 	  
                     while(count--) {
			  dword = aotg_readl(ep->reg_hcfifo);
                        *ppp++ = (unsigned char)dword;  
			  *ppp++ = (unsigned char)(dword>>8);
			  *ppp++ = (unsigned char)(dword>>16);
			  *ppp++ = (unsigned char)(dword>>24);			  
		    }    
		}       
		
		/*read remains in BYTE*/
		if(remain != 0) {  
			bytebuf = urb->transfer_buffer + urb->actual_length;
			fifoaddr = ep->reg_hcfifo;
			for(i = 0;i < remain;i++) {
				byte = aotg_readb(fifoaddr++);
				*bytebuf++ = byte;
				urb->actual_length ++;
			}
		}
		
		if (is_iso_endpoint(ep)){
			iso_urb_update(ep,urb,length);
		}
		

		//printk("Trans len:%d %d %d\n",length,urb->actual_length,urb->transfer_buffer_length);
		
		
		if ( !is_iso_endpoint(ep) ) {
			if(urb->actual_length == urb->transfer_buffer_length) {
				is_last = 1;
			}else if( length < ep->maxpacket) {
				is_last = is_short = 1;
				if (urb->transfer_flags & URB_SHORT_NOT_OK){
					urb->status = -EREMOTEIO;
			        	shorterr = 1;
				}
			}
		} else {
			if ( is_iso_urb_over(ep,urb) ) {
				is_last = 1;
			}	
		}

//#ifdef DEBUG_TXRX
#if 0
		if(ep->type == PIPE_BULK){
			INFO( "receive %dB %s %s %s %s actual %dB claim %dB\n",
				length,is_last ? "L":"", is_short ? "S":"", shorterr ? "E":"", overflag? "O":"",
				urb->actual_length,urb->transfer_buffer_length);
		}
#endif
		/*when short,urb is ok*/
		if(is_short ||is_last) {
			//printk("read_hcep_fifo:len:%x tran_len:%x total_len:%x %x\n",length,urb->actual_length,urb->transfer_buffer_length,aotg_readb(ep->reg_hcepcs));
			finish_request(acthcd,ep, urb);
			
			return 1;
		}	
    }else{
		printk("read_hcep_fifo error ,the buffer is busy!\n");
	}

	return 0;	
}


static int write_hcep_fifo(struct aotg_hcd *acthcd,struct aotg_hcep *ep,struct urb *urb)
{
	while (!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)) {
		int	count;
		int		is_last;
		struct usb_device *udev;

		count = ep->length;

		//printk("<jjf_debug>write_hcep_fifo:send %d byte data!\n",count);

		//count = write_packet(ep,urb,ep->maxpacket);
		udev = urb->dev;
		usb_dotoggle(udev, ep->epnum, 1);
		ep->length = count;
		urb->actual_length += ep->length;
		/* last packet is usually short (or a zlp) */
		if (count != ep->maxpacket)
			is_last =  1;
		else {
			if ((urb->transfer_buffer_length != urb->actual_length)
					|| (urb->transfer_flags & URB_ZERO_PACKET)){
				is_last = 0;
			}else
				is_last = 1;
		}
		
#ifdef DEBUG_TXRX		
		if(ep->type ==PIPE_BULK ){
			INFO("<am7x_hcd>@@@---<BULK> OUT: send %dB L actual %dB claim %db\n",
					ep->length,urb->actual_length,
					urb->transfer_buffer_length);
		}
#endif

		if (is_last){
			//printk("<jjf_debug>write_hcep_fifo:write finish!\n",is_last);
			finish_request(acthcd,ep,urb);
			return 1;
		}else{
			//printk("<jjf_debug>write_hcep_fifo:write not finish! %d %d\n",urb->actual_length,urb->transfer_buffer_length);
		}
	};
	 return 0;	
}


static void handle_hcep0_err(struct aotg_hcd *acthcd)
{
	struct aotg_hcep  *ep;
	struct urb		*urb;
	struct usb_device	*udev;

	ep = acthcd->ep_list[0];


	if(acthcd->cur_ep != NULL){

		if(acthcd->cur_ep->index != 0){
			printk("Cur ep is skip! %d %d\n",0,acthcd->cur_ep->index);	
		}

	}else{
		printk("handle_hcep0_err:acthcd->cur_ep is NULL!\n");
	}

	if(ep == NULL){
		/*ep is disable,the urb is clean up by disable*/
		printk("handle_hcep0_err ep is null,maybe is disable!\n");
		return;
	}

	if(list_empty(&ep->hep->urb_list)){
		printk("handle_hcep0_err urb list empty ,maybe dequeue !\n");
		return;
	}

	
	

#if 0
	//1.test point by jjf
	if(ep == NULL){
		printk("<jjf_debug> handle_hcep0_out ep is error ,ep addr:%x!\n",ep);
		return;
	}

	if(list_empty(&ep->hep->urb_list)){
		printk("<jjf_debug> handle_hcep0_out urb list error is NULL !\n");
		return;
	}
#endif

	urb = container_of(ep->hep->urb_list.next,struct urb,urb_list);
	udev = urb->dev;

	if(urb == NULL){
		printk("handle_hcep0_err urb is NULL!\n");
		return;
	}else{
		urb->status = -EPIPE;
		finish_request(acthcd,ep,urb);
	}




	
#if 0
	switch(ep->nextpid){
	case USB_PID_SETUP:
		DMSG_CTRL("<am7x_hcd>@@@---<CTRL> SETUP stage end\n");
		if(urb->transfer_buffer_length == urb->actual_length) {
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL>NO-DATA transfer\n");
			ep->nextpid = USB_PID_ACK;
			handle_status(1);/*no-data transfer*/
			return;  /*no need to schedule the async list*/
		}else if(usb_pipeout(urb->pipe)) {
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL> DATA stage start,control write\n");
			usb_settoggle(udev, 0, 1, 1);
			ep->nextpid = USB_PID_OUT;		
		}else{
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL> DATA stage start,control read\n");
			usb_settoggle(udev, 0, 0, 1);
			ep->nextpid = USB_PID_IN;
			//aotg_writeb(0,HCIN0BC);
		}
		break;
	case USB_PID_OUT:
		urb->actual_length += ep->length;
		usb_dotoggle(udev, ep->epnum, 1);
		if (urb->actual_length == urb->transfer_buffer_length) {
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL> OUT: send %dB L actual %dB claim %db\n",
				ep->length,urb->actual_length,urb->transfer_buffer_length);
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL>DATA stage end, control read\n");
			ep->nextpid = USB_PID_ACK;
			handle_status(1);/*control write transfer*/
		//	DMSG_CTRL("%s:%d\n",__func__,__LINE__);
			return;  /*no need to schedule the async list*/
		}
		DMSG_CTRL("<am7x_hcd>@@@---<CTRL> OUT: send %dB   actual %dB claim %db\n",
				ep->length,urb->actual_length,urb->transfer_buffer_length);
		break;
	case USB_PID_ACK:
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL> STATUS stage end\n");
			finish_request(acthcd,ep,urb);
			return ;
			break;
	default:
		DMSG_CTRL("<am7x_hcd>@@@---<CTRL>ep0 out ,odd pid %d\n",ep->nextpid);	
	}
	start_async_transfer(acthcd,ep);
#endif
}




static void handle_hcep0_out(struct aotg_hcd *acthcd)
{
	struct aotg_hcep  *ep;
	struct urb		*urb;
	struct usb_device	*udev;

	ep = acthcd->ep_list[0];


	if(acthcd->cur_ep != NULL){


		if(acthcd->cur_ep->index != 0){
			printk("Cur ep is skip! %d %d\n",0,acthcd->cur_ep->index);	
		}

	}else{
		printk("handle_hcep0_out:acthcd->cur_ep is NULL!\n");
	}

	if(ep == NULL){
		/*ep is disable,the urb is clean up by disable*/
		printk("handle_hcep0_out ep is null,maybe is disable!\n");
		return;
	}

	if(list_empty(&ep->hep->urb_list)){
		printk("handle_hcep0_out urb list empty ,maybe dequeue !\n");
		return;
	}

	
	

#if 0
	//1.test point by jjf
	if(ep == NULL){
		printk("<jjf_debug> handle_hcep0_out ep is error ,ep addr:%x!\n",ep);
		return;
	}

	if(list_empty(&ep->hep->urb_list)){
		printk("<jjf_debug> handle_hcep0_out urb list error is NULL !\n");
		return;
	}
#endif

	urb = container_of(ep->hep->urb_list.next,struct urb,urb_list);
	udev = urb->dev;

	if(urb == NULL){
		printk("handle_hcep0_out urb is NULL!\n");
		return;
	}else{
		if(urb->status != -EINPROGRESS){
			printk("handle_hcep0_out :error process!\n");
			finish_request(acthcd,ep,urb);
		}
	}


	switch(ep->nextpid){
		case USB_PID_SETUP:
			if(urb->transfer_buffer_length == urb->actual_length) {	
				ep->nextpid = USB_PID_ACK;
			}else if(usb_pipeout(urb->pipe)){
				ep->nextpid = USB_PID_OUT;	
				usb_settoggle(udev, 0, 1, 1);
			}else{
				ep->nextpid = USB_PID_IN;
				usb_settoggle(udev, 0, 0, 1);
			}
			break;
		case USB_PID_OUT:
			usb_dotoggle(udev, ep->epnum, 0);
			urb->actual_length += ep->length;
			if (urb->actual_length == urb->transfer_buffer_length){
				ep->nextpid = USB_PID_ACK;
			}
			break;
		case USB_PID_IN:
			read_hcep0_fifo(ep,urb);
			break;
		case USB_PID_ACK:
			finish_request(acthcd,ep,urb);
			ep->nextpid = USB_PID_SETUP;
			break;
		default:
			printk("<jjf_debug> handle_hcep0_out urb nextpid error :%x !\n",ep->nextpid);
			break;
	}

	
#if 0
	switch(ep->nextpid){
	case USB_PID_SETUP:
		DMSG_CTRL("<am7x_hcd>@@@---<CTRL> SETUP stage end\n");
		if(urb->transfer_buffer_length == urb->actual_length) {
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL>NO-DATA transfer\n");
			ep->nextpid = USB_PID_ACK;
			handle_status(1);/*no-data transfer*/
			return;  /*no need to schedule the async list*/
		}else if(usb_pipeout(urb->pipe)) {
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL> DATA stage start,control write\n");
			usb_settoggle(udev, 0, 1, 1);
			ep->nextpid = USB_PID_OUT;		
		}else{
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL> DATA stage start,control read\n");
			usb_settoggle(udev, 0, 0, 1);
			ep->nextpid = USB_PID_IN;
			//aotg_writeb(0,HCIN0BC);
		}
		break;
	case USB_PID_OUT:
		urb->actual_length += ep->length;
		usb_dotoggle(udev, ep->epnum, 1);
		if (urb->actual_length == urb->transfer_buffer_length) {
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL> OUT: send %dB L actual %dB claim %db\n",
				ep->length,urb->actual_length,urb->transfer_buffer_length);
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL>DATA stage end, control read\n");
			ep->nextpid = USB_PID_ACK;
			handle_status(1);/*control write transfer*/
		//	DMSG_CTRL("%s:%d\n",__func__,__LINE__);
			return;  /*no need to schedule the async list*/
		}
		DMSG_CTRL("<am7x_hcd>@@@---<CTRL> OUT: send %dB   actual %dB claim %db\n",
				ep->length,urb->actual_length,urb->transfer_buffer_length);
		break;
	case USB_PID_ACK:
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL> STATUS stage end\n");
			finish_request(acthcd,ep,urb);
			return ;
			break;
	default:
		DMSG_CTRL("<am7x_hcd>@@@---<CTRL>ep0 out ,odd pid %d\n",ep->nextpid);	
	}
	start_async_transfer(acthcd,ep);
#endif
}






static void handle_hcep(struct aotg_hcd *acthcd, int epmask,int index)
{	
	struct aotg_hcep  *ep;
	struct urb		*urb;
	struct usb_device	*udev;


	//if(epmask&0xf0){
		//printk("handle_hcep:%x\n",epmask);
	//}
	
	if(acthcd->cur_ep != NULL){
		if(acthcd->cur_ep->index != index){
				printk("Cur ep is skip! %d %d\n",index,acthcd->cur_ep->index);
		}
	}

	ep = acthcd->ep_list[index];
	if(ep == NULL){
		printk("handle_hcep ep%d is disable!\n",index);
		goto handle_hcep_end;
	}

	if(list_empty(&ep->hep->urb_list)){
		printk("handle_hcep%d urb list empty ,maybe dequeue !\n",index);
		goto handle_hcep_end;
	}

	
	

	//printk("Cur ep is ! %d %d %x\n",index,acthcd->cur_ep->index,ep);
	
	
#if 0
	//1.test point by jjf
	if(ep == NULL){
		printk("<jjf_debug> handle_hcep ep is error ,ep addr:%x!\n",ep);
		return;
	}


	if(list_empty(&ep->hep->urb_list)){
		printk("<jjf_debug> handle_hcep urb list error is NULL !\n");
		return;
	}

	if(ep->mask != epmask){
		printk("<jjf_debug> handle_hcep ep is error:%d %d \n!\n",epmask,ep->mask);
		return;
	}
#endif

	

	urb = container_of(ep->hep->urb_list.next,struct urb,urb_list);
	if(urb == NULL){
		printk("handle_hcep0_out urb is NULL!\n");
		
		goto handle_hcep_end;
	}else{
		if(urb->status != -EINPROGRESS){
			finish_request(acthcd,ep,urb);
		}
	}

	udev = urb->dev;
	switch(ep->nextpid) {
		case USB_PID_IN:
			read_hcep_fifo(acthcd,ep,urb);
			break;
		case USB_PID_OUT:
			usb_dotoggle(udev, ep->epnum, 1);
			urb->actual_length += ep->length;
			//printk("Trans len:%d %d %d %x\n",ep->length,urb->actual_length,urb->transfer_buffer_length,aotg_readb(ep->reg_hcepcs));
			if (urb->actual_length == urb->transfer_buffer_length){
				finish_request(acthcd,ep,urb);
			}
			break;
		case USB_PID_ACK:
			
			goto handle_hcep_end;
		default:
			break;
	}
handle_hcep_end:
	return;
}


#define sof_get_frm(x)	((x>>3)&0x7ff)
#define sof_get_ufrm(x)	(x&0x7)

static int sof_irq_lost(unsigned int curr, unsigned int last)
{
	unsigned int curr_frm,curr_ufrm,last_frm,last_ufrm;
	unsigned int result;
	
	if (last==0xffffffff)  
		return 1;
	
	curr_frm = sof_get_frm(curr);
	curr_ufrm = sof_get_ufrm(curr);

	last_frm = sof_get_frm(last);
	last_ufrm = sof_get_ufrm(last);
#if 0
	result = (curr_frm - last_frm + 2048)%2048;
	result = result*8 + ( curr_ufrm - last_ufrm );
#else
	result = (curr_frm - last_frm + 2048)&2047;
	result = (result<<3) + ( curr_ufrm - last_ufrm );
#endif

	return result;
}


static unsigned int curr=0xffffffff,last=0xffffffff;
static unsigned int lost_count = 0, max_count=0;
static void do_sof_irq(struct aotg_hcd *acthcd)
{
	unsigned index;
	int i;
	int ls = 0; /* merge code from James Chiang, 20170224, Fix https://teamwork.iezvu.com/T3715 , unsigned ls -> int */   

#if 1
	curr = aotg_readw(AOTG_REG(0x1ac));
	ls=sof_irq_lost(curr,last);
	last = curr;
#else
	curr = aotg_readw(AOTG_REG(0x1ac));
	if ( (ls=sof_irq_lost(curr,last)) > 1 ){
		if (ls > max_count)
			max_count = ls;
		
		lost_count += ls;
		DMSG_HCD("<am7x_hcd>@@@---sl:%d.%d %d.%d %d.%d.%d\n",
			sof_get_frm(curr),
			sof_get_ufrm(curr), 
			sof_get_frm(last),
			sof_get_ufrm(last),
			ls,
			max_count,
			lost_count);
	}
	last = curr;
#endif	

	for(i=0;i<ls;i++)
	{
	//index = acthcd->frame % PERIODIC_SIZE;
	index = acthcd->frame & (PERIODIC_SIZE-1);
    acthcd->frame++;
	acthcd->stat_sof++;
	
	/* be graceful about almost-inevitable periodic schedule
	 * overruns:  continue the previous frame's transfers iff
	 * this one has nothing scheduled.
	 */
	if (acthcd->next_periodic) {
		acthcd->stat_overrun++;
	}
	
	if (acthcd->periodic[index]){
		acthcd->next_periodic = acthcd->periodic[index];
        start_periodic_transfer(acthcd,NULL);
	}
	}
}

static void hcep_error_handler(struct usb_hcd *hcd,int ep_mask,int index)
{
	struct aotg_hcep * ep;
	struct urb * urb = NULL;
	struct aotg_hcd *acthcd = hcd_to_aotg(hcd);
	int type;
	unsigned int reg_base;

	if(acthcd->cur_ep != NULL){
		if(acthcd->cur_ep->index != index){
				printk("hcep_error_handler:Cur ep is skip! %d %d\n",index,acthcd->cur_ep->index);
		}
	}
	
	ep = acthcd->ep_list[index];
	if(ep == NULL){
		printk("hcep_error_handler ep%d is disable!\n",index);
		return;
	}
	
	type = get_ep_errtype(ep->reg_hceperr);
	if ( type ==  EPERR_NONE){
		DMSG_HCD("<am7x_hcd>@@@---reserved error!\n");
		aotg_writeb(1<<(ep_mask&0x0f),HCOUT04ERRIRQ);	
		return;
	}
	
#ifndef CONFIG_USB_AM7X_HOST_ONLY_0	
	if(list_empty(&ep->hep->urb_list)){
		urb=NULL;
		ERR("<am7x_hcd>@@@---urb NULL\n");
		if (type == EPERR_STALL){
			INFO("<am7x_hcd>@@@---ep stalled!\n");
			acthcd->peer_flags |= PEER_STALLED;
			ep->stalled = 1;
		}
	}else{
		urb = container_of(ep->hep->urb_list.next,struct urb,urb_list);
		if (type == EPERR_STALL){
			acthcd->peer_flags |= PEER_STALLED;
			urb->status = -EPIPE;
			ep->stalled = 1;
		}else
			urb->status = -ETIMEDOUT;
			
		finish_request(acthcd,ep,urb);
	}
	printk("CONFIG_USB_AM7X_HOST_ONLY_0:%x\n",CONFIG_USB_AM7X_HOST_ONLY_0);
#endif	
	reg_base = (ep_mask&USB_HCD_OUT_MASK) ? HCOUT0ERR: HCIN0ERR;
	if (urb || (type != EPERR_NONE) ){
		aotg_hcep_reset(ep->mask,ENDPRST_FIFOREST |ENDPRST_TOGRST);
	}
	INFO("<am7x_hcd>@@@---disable resend\n");
	//else
		//aotg_setbits(1<<5,reg_base+4*(ep_mask&0x0f));	//calculate reg addr base on HCOUT0ERR	
}

static irqreturn_t aotg_hcd_irq(struct usb_hcd *hcd)
{	
	u8 irqvector,type;
	struct aotg_hcep * ep;
	struct list_head *iter;
	struct urb * urb = NULL;
	struct aotg_hcd *acthcd = hcd_to_aotg(hcd);
	irqvector = aotg_readb(IVECT);
	if(irqvector == UIV_OTGIRQ){
		DMSG_HCD("<am7x_hcd>@@@---this irq belongs to otg\n");
		return IRQ_HANDLED; 
	}

	/*USB  interrupt*/
	//printk("<irq_%x>\n",irqvector);
	switch(irqvector) {
		case UIV_SOF:
			//printk("<am7x_hcd>@@@---sof\n");
			aotg_clear_pend(USBIRQ_SOF,USBIRQ); /*clear sof irq*/
			list_for_each(iter, &acthcd->async){
				if(iter !=  &acthcd->async)
					ep = container_of(iter,struct aotg_hcep,schedule);
					ep->ep_schedule=0;
			}
			if(is_ep_busy()){
				irq_cnt++;
				if(irq_cnt>3){
					dump_urb_info();
					set_ep_idle();
					irq_cnt = 0;
				}
				/*do nothing*/
			}else{
				kick_ep_thread();
			}
			
			//sofEnable=1;
			//sof_disable();
			//do_sof_irq(acthcd);
			break;
			
		case UIV_USBRESET:
			printk("UIV_USBRESET\n");
			acthcd->speed = FULLSPEED; /*FS is the default*/
			acthcd->portstate = acthcd->inserted ? AOTG_PORT_ATTACHED : AOTG_PORT_ENABLE;
			acthcd->transceiver->port_status &=  ~USB_PORT_STAT_RESET;
			acthcd->transceiver->port_status |= USB_PORT_STAT_ENABLE;
			
			if(acthcd->inserted)
				acthcd->transceiver->port_status |= USB_PORT_STAT_CONNECTION;
			
			/*set EP0 IN maxpacket */
			aotg_writeb(0x40,HCIN0MAXPCK);

			/*disable all EP's PIO irq*/
			aotg_writeb(0,HCIN04IEN);
			aotg_writeb(0,HCOUT04IEN);

			/*reset all ep*/
			aotg_hcep_reset(USB_HCD_IN_MASK,ENDPRST_FIFOREST |ENDPRST_TOGRST);
			aotg_hcep_reset(USB_HCD_OUT_MASK,ENDPRST_FIFOREST |ENDPRST_TOGRST);
			
			if(aotg_readb(USBIRQ) & USBIRQ_HS){
				//aotg_clear_pend(USBIRQ_HS,USBIRQ);
				acthcd->speed  = HIGHSPEED;
				acthcd->transceiver->port_status |= USB_PORT_STAT_HIGH_SPEED;
				DMSG_OTG("<am7x_hcd>@@@---res:h\n");
			}else if(aotg_readb(USBCS) & USBCS_LSMODE){
				acthcd->speed = LOWSPEED;
				acthcd->transceiver->port_status |= USB_PORT_STAT_LOW_SPEED;
				DMSG_OTG("<am7x_hcd>@@@---res:l\n");
			}else{ 
				acthcd->speed = FULLSPEED;
				DMSG_OTG("<am7x_hcd>@@@---res:f\n");
			}
			
			aotg_clear_pend(USBIRQ_URES,USBIRQ); /*clear usb reset irq*/
			aotg_clearbits(USBIEN_URES,USBIEN); /*disable reset irq */
			/*clear&enable EPERRIRQ */
			aotg_writeb(aotg_readb(HCIN04ERRIRQ),HCIN04ERRIRQ);
			aotg_writeb(aotg_readb(HCOUT04ERRIRQ),HCOUT04ERRIRQ);

			aotg_setbits((1<<0)|(1<<1)|(1<<2),HCIN04ERRIEN);
			aotg_setbits((1<<0)|(1<<0),HCOUT04ERRIEN);	
			aotg_writeb(0x1f,HCIN04ERRIEN);
			aotg_writeb(0x1f,HCOUT04ERRIEN);

			break;
		case UIV_HSPEED:
			INFO("<am7x_hcd>@@@---get hs irq\n");
			break;

		case UIV_EP0IN:
			//printk("UIV_EP0IN_Start\n");
			aotg_writeb(EP0_OUT_IRQ,HCOUT04IRQ); /*clear hcep0out irq*/
			stop_timer(irqvector);

			handle_hcep0_out(acthcd);
			set_ep_idle();
		
			break;
		case UIV_EP0OUT:
			//printk("UIV_EP0OUT_Start\n");
			aotg_writeb(EP0_IN_IRQ,HCIN04IRQ); /*clear hcep0in irq*/
			stop_timer(irqvector);

			handle_hcep0_out(acthcd);
			set_ep_idle();

			break;
			
		case UIV_EP1IN:  
			//printk("UIV_EP1IN_START\n");
			//INFO("get hcep1out irq\n");
			aotg_writeb(EP1_OUT_IRQ,HCOUT04IRQ); /*clear hcep1out irq*/
			stop_timer(irqvector);

			handle_hcep(acthcd,1 |USB_HCD_OUT_MASK,2);
			set_ep_idle();
			
			//printk("UIV_EP1IN_OUT\n");
			break;
			
		case UIV_EP2IN:
			//INFO("get hcep2out irq\n");
			//printk("UIV_EP2IN\n");
			aotg_writeb(EP2_OUT_IRQ,HCOUT04IRQ); /*clear hcep2out irq*/
			stop_timer(irqvector);
			handle_hcep(acthcd,2 |USB_HCD_OUT_MASK,4);
			set_ep_idle();
			
			break;

		case UIV_EP3IN:
			//INFO("get hcep3out irq\n");
			//printk("UIV_EP3IN\n");
			aotg_writeb(EP3_OUT_IRQ,HCOUT04IRQ); /*clear hcep3out irq*/
			stop_timer(irqvector);
			handle_hcep(acthcd,3 |USB_HCD_OUT_MASK,6);
			set_ep_idle();
			break;
			
		case UIV_EP4IN:
			//INFO("get hcep4out irq\n");
			//printk("UIV_EP4IN\n");
			aotg_writeb(EP4_OUT_IRQ,HCOUT04IRQ); /*clear hcep4out irq*/
			stop_timer(irqvector);
			handle_hcep(acthcd,4 |USB_HCD_OUT_MASK,8);
			set_ep_idle();
			
			break;

		case UIV_EP5IN:
			//INFO("get hcep4out irq\n");
			//printk("UIV_EP5IN\n");
			aotg_writeb(EP5_OUT_IRQ,HCOUT04IRQ); /*clear hcep4out irq*/
			stop_timer(irqvector);
			handle_hcep(acthcd,5 |USB_HCD_OUT_MASK,10);
			set_ep_idle();
			break;

		case UIV_EP6IN:
			//INFO("get hcep4out irq\n");
			//printk("UIV_EP6IN\n");
			aotg_writeb(EP6_OUT_IRQ,HCOUT04IRQ); /*clear hcep4out irq*/
			stop_timer(irqvector);
			handle_hcep(acthcd,6 |USB_HCD_OUT_MASK,12);
			set_ep_idle();
			break;
			
		case UIV_EP1OUT: 	 
			//INFO("get hcep1in irq\n");
			//printk("UIV_EP1OUT\n");
			//printk("UIV_EP1OUT_START\n");
			
			
			//aotg_clearbits((1<<AUTO_CTRL_INTIRQ_MASK),HCIN1AUTOCTLUP);
			//invalidate_ep_reg(HCIN1CON);
			

			aotg_writeb(EP1_IN_IRQ,HCIN04IRQ); /*clear hcep1in irq*/
			stop_timer(irqvector);
			handle_hcep(acthcd,1,1);
			set_ep_idle();
#if 0
			if(aotg_readb(HCIN04ERRIRQ)&(1<<1)){
				aotg_clear_pend(1<<1,HCIN04ERRIRQ);
				aotg_hcep_reset(0x1,ENDPRST_FIFOREST);
			}
#else
#if 0
			if(aotg_readb(HCIN04ERRIRQ)&(1<<1)){
				aotg_clear_pend(1<<1,HCIN04ERRIRQ);
				if(get_ep_errtype(HCIN1ERR)){
					aotg_hcep_reset(0x01,ENDPRST_FIFOREST);		
				}
			}
#endif
#endif
			
			//aotg_writeb(1<<5,HCIN1ERR);
			//printk("UIV_EP1OUT_END\n");
			break;
			
		case UIV_EP2OUT:
			//INFO("get hcep2in irq\n");
			//printk("UIV_EP2OUT\n");
			//aotg_clearbits((1<<AUTO_CTRL_INTIRQ_MASK),HCIN2AUTOCTLUP);
			//invalidate_ep_reg(HCIN2CON);
			

			aotg_writeb(EP2_IN_IRQ,HCIN04IRQ); /*clear hcep2in irq*/
			stop_timer(irqvector);
			handle_hcep(acthcd,2,3);
			set_ep_idle();
#if 0
			if(aotg_readb(HCIN04ERRIRQ)&(1<<2)){
				aotg_clear_pend(1<<2,HCIN04ERRIRQ);
				aotg_hcep_reset(0x2,ENDPRST_FIFOREST);
			}
#else
#if 0
			if(aotg_readb(HCIN04ERRIRQ)&(1<<2)){
				aotg_clear_pend(1<<2,HCIN04ERRIRQ);
				if(get_ep_errtype(HCIN2ERR)){
					aotg_hcep_reset(0x02,ENDPRST_FIFOREST);		
				}
			}
#endif
#endif
			
			//aotg_writeb(1<<5,HCIN2ERR);
			break;	

		case UIV_EP3OUT: 	 
			//INFO("get hcep3in irq\n");
			//printk("UIV_EP3OUT\n");
			//aotg_clearbits((1<<AUTO_CTRL_INTIRQ_MASK),HCIN3AUTOCTLUP);
			//invalidate_ep_reg(HCIN3CON);
			

			aotg_writeb(EP3_IN_IRQ,HCIN04IRQ); /*clear hcep3in irq*/
			stop_timer(irqvector);
			//printk("3out:%x\n",aotg_readb(HCIN04ERRIRQ));	
			handle_hcep(acthcd,3,5);
			set_ep_idle();
#if 0
			if(aotg_readb(HCIN04ERRIRQ)&(1<<3)){
				aotg_clear_pend(1<<3,HCIN04ERRIRQ);
				aotg_hcep_reset(0x3,ENDPRST_FIFOREST);
			}
#else
#if 0
			if(aotg_readb(HCIN04ERRIRQ)&(1<<3)){
				aotg_clear_pend(1<<3,HCIN04ERRIRQ);
				if(get_ep_errtype(HCIN3ERR)){
					aotg_hcep_reset(0x03,ENDPRST_FIFOREST);		
				}
			}
#endif
#endif
			//aotg_writeb(1<<5,HCIN3ERR);
			break;
			
		case UIV_EP4OUT:
			//INFO("get hcep4in irq\n");
			//printk("UIV_EP4OUT\n");
			//aotg_clearbits((1<<AUTO_CTRL_INTIRQ_MASK),HCIN4AUTOCTLUP);
			//invalidate_ep_reg(HCIN4CON);
			

			aotg_writeb(EP4_IN_IRQ,HCIN04IRQ); /*clear hcep4in irq*/
			stop_timer(irqvector);
			handle_hcep(acthcd,4,7);
			set_ep_idle();
#if 0
			if(aotg_readb(HCIN04ERRIRQ)&(1<<4)){
				aotg_clear_pend(1<<4,HCIN04ERRIRQ);
				aotg_hcep_reset(0x4,ENDPRST_FIFOREST);
			}
#else
#if 0
			if(aotg_readb(HCIN04ERRIRQ)&(1<<4)){
				aotg_clear_pend(1<<4,HCIN04ERRIRQ);
				if(get_ep_errtype(HCIN4ERR)){
					aotg_hcep_reset(0x04,ENDPRST_FIFOREST);		
				}
			}
#endif
#endif
			
			//aotg_writeb(1<<5,HCIN4ERR);
			break;	

		case UIV_EP5OUT:  
			//INFO("get hcep4in irq\n");
			aotg_writeb(EP5_IN_IRQ,HCIN04IRQ); /*clear hcep4in irq*/
			stop_timer(irqvector);
			handle_hcep(acthcd,5,9);
			set_ep_idle();
			break;

		case UIV_EP6OUT:  
			//INFO("get hcep4in irq\n");
			aotg_writeb(EP6_IN_IRQ,HCIN04IRQ); /*clear hcep4in irq*/
			stop_timer(irqvector);
			handle_hcep(acthcd,6,11);
			set_ep_idle();
			break;
			
		case UIV_HCOUT0ERR:
#if 0
			ep = acthcd->ep_list[0];
			ERR("<am7x_hcd>@@@---###$$$HCOUT0ERR =%x\n", aotg_readb(HCOUT0ERR));	
			if(list_empty(&ep->hep->urb_list)){
				urb=NULL;
				ERR("<am7x_hcd>@@@---urb null\n");				
			}else{
				urb = container_of(ep->hep->urb_list.next,struct urb,urb_list);
				urb->status = -EPIPE;
				aotg_setbits(1<<5,HCOUT0ERR);	
				finish_request(acthcd,ep,urb);
			}
			
			aotg_setbits(1<<0,HCOUT04ERRIRQ);
			mb();
#else
			ERR("<am7x_hcd>@@@---###$$$HCOUT0ERR =%x\n", aotg_readb(HCOUT0ERR));
			//aotg_setbits(1<<0,HCOUT04ERRIRQ);
			aotg_clear_pend(1<<0,HCOUT04ERRIRQ);	
			stop_timer(irqvector);
			set_ep_idle();
			handle_hcep0_err(acthcd);
			aotg_hcep_reset(0x10,ENDPRST_FIFOREST);
			
#endif
			break;
			
		case UIV_HCIN0ERR: 	
#if 0
			ep = acthcd->ep_list[0];
			ERR("<am7x_hcd>@@@---HCIN0ERR =%x\n", aotg_readb(HCIN0ERR));
			type = (aotg_readb(HCIN0ERR)>>2) & 7;
			if (!ep->hep)
				goto hcin0err_exit;

			if(list_empty(&ep->hep->urb_list)){
				urb=NULL;
				ERR("<am7x_hcd>@@@---urb null\n");					
			}else{
				urb = container_of(ep->hep->urb_list.next,struct urb,urb_list);
				urb->status = -EPIPE;
				finish_request(acthcd,ep,urb);
			}
hcin0err_exit:
			aotg_setbits(1<<0,HCIN04ERRIRQ);
			if (urb || type==3){
				aotg_hcep_reset(ep->mask,ENDPRST_FIFOREST |ENDPRST_TOGRST);
			}
#else
			ERR("<am7x_hcd>@@@---HCIN0ERR =%x\n", aotg_readb(HCIN0ERR));
			aotg_clear_pend(1<<0,HCIN04ERRIRQ);
			stop_timer(irqvector);
			set_ep_idle();
			
			handle_hcep0_err(acthcd);
			aotg_hcep_reset(0x0,ENDPRST_FIFOREST);
			
#endif
			break;
			
		case UIV_HCOUT1ERR:
			ERR("<am7x_hcd>@@@---hcout1err:%x\n",aotg_readb(HCOUT1ERR));
			//hcep_error_handler(hcd, 1 |USB_HCD_OUT_MASK,2);
			aotg_clear_pend(1<<1,HCOUT04ERRIRQ);	
			set_ep_idle();
			if(get_ep_errtype(HCOUT1ERR)){
				printk("HCOUT1ERR:%x\n",aotg_readb(HCOUT1ERR));
				aotg_hcep_reset(0x11,ENDPRST_FIFOREST);
			}
			//handle_hcep0_err(acthcd);
			//aotg_hcep_reset(0x11,ENDPRST_FIFOREST);
			stop_timer(irqvector);
			break;	

		case UIV_HCOUT2ERR:
			ERR("<am7x_hcd>@@@---hcout2err:%x\n",aotg_readb(HCOUT2ERR));
			//hcep_error_handler(hcd, 2 |USB_HCD_OUT_MASK,4);
			aotg_clear_pend(1<<2,HCOUT04ERRIRQ);	
			set_ep_idle();
			if(get_ep_errtype(HCOUT2ERR)){
				printk("HCOUT2ERR:%x\n",aotg_readb(HCOUT2ERR));
				aotg_hcep_reset(0x12,ENDPRST_FIFOREST);
			}
			//aotg_hcep_reset(0x12,ENDPRST_FIFOREST);
			stop_timer(irqvector);
			break;	
			
		case UIV_HCOUT3ERR:
			ERR("<am7x_hcd>@@@---hcout3err:%x\n",aotg_readb(HCOUT3ERR));
			//hcep_error_handler(hcd, 3 |USB_HCD_OUT_MASK,6);
			aotg_clear_pend(1<<3,HCOUT04ERRIRQ);
			set_ep_idle();
			if(get_ep_errtype(HCOUT3ERR)){
				printk("HCOUT3ERR:%x\n",aotg_readb(HCOUT3ERR));
				aotg_hcep_reset(0x13,ENDPRST_FIFOREST);
			}
			//aotg_hcep_reset(0x13,ENDPRST_FIFOREST);
			stop_timer(irqvector);
			break;
			
		case UIV_HCOUT4ERR:
			ERR("<am7x_hcd>@@@---hcout4err:%x\n",aotg_readb(HCOUT4ERR));
			//hcep_error_handler(hcd, 4 |USB_HCD_OUT_MASK,8);
			aotg_clear_pend(1<<4,HCOUT04ERRIRQ);	
			set_ep_idle();
			if(get_ep_errtype(HCOUT4ERR)){
				printk("HCOUT4ERR:%x\n",aotg_readb(HCOUT4ERR));
				aotg_hcep_reset(0x14,ENDPRST_FIFOREST);
			}
			//aotg_hcep_reset(0x14,ENDPRST_FIFOREST);
			stop_timer(irqvector);
			break;

		case UIV_HCOUT5ERR:
			ERR("<am7x_hcd>@@@---hcout5err:%x\n",aotg_readb(HCOUT5ERR));
			//hcep_error_handler(hcd, 5 |USB_HCD_OUT_MASK,10);
			aotg_clear_pend(1<<5,HCOUT04ERRIRQ);	
			set_ep_idle();
			if(get_ep_errtype(HCOUT5ERR)){
				printk("HCOUT5ERR:%x\n",aotg_readb(HCOUT5ERR));
				aotg_hcep_reset(0x15,ENDPRST_FIFOREST);
			}
			//aotg_hcep_reset(0x15,ENDPRST_FIFOREST);
			stop_timer(irqvector);
			break;

		case UIV_HCOUT6ERR:
			ERR("<am7x_hcd>@@@---hcout6err:%x\n",aotg_readb(HCOUT6ERR));
			//hcep_error_handler(hcd, 6 |USB_HCD_OUT_MASK,12);
			aotg_clear_pend(1<<6,HCOUT04ERRIRQ);	
			set_ep_idle();
			if(get_ep_errtype(HCOUT6ERR)){
				printk("HCOUT6ERR:%x\n",aotg_readb(HCOUT6ERR));
				aotg_hcep_reset(0x16,ENDPRST_FIFOREST);
			}
			//aotg_hcep_reset(0x16,ENDPRST_FIFOREST);
			stop_timer(irqvector);
			break;
			
		case  UIV_HCIN1ERR:		
			//ERR("<am7x_hcd>@@@---hcin1terr:%x\n",aotg_readb(HCIN1ERR));
			//hcep_error_handler(hcd, 1,1);
			//printk(KERN_INFO "%s %d====HCIN04ERRIRQ=%x\n",__FUNCTION__,__LINE__,aotg_readl(HCIN04ERRIRQ));
			aotg_clearbits((1<<AUTO_CTRL_INTIRQ_MASK),HCIN1AUTOCTLUP);
			invalidate_ep_reg(HCIN1CON);
			
			aotg_clear_pend(1<<1,HCIN04ERRIRQ);
			set_ep_idle();

			if(get_ep_errtype(HCIN1ERR)){
				printk("HCIN1ERR:%x\n",aotg_readb(HCIN1ERR));
				aotg_hcep_reset(0x01,ENDPRST_FIFOREST);
			}
			//aotg_writeb(1<<5,HCIN1ERR);
			//aotg_hcep_reset(0x01,ENDPRST_FIFOREST);
			stop_timer(irqvector);
			//printk(KERN_INFO "%s %d====HCIN04ERRIRQ=%x\n",__FUNCTION__,__LINE__,aotg_readl(HCIN04ERRIRQ));
			break;
			
		case  UIV_HCIN2ERR:
			//ERR("<am7x_hcd>@@@---hcin2terr:%x\n",aotg_readb(HCIN2ERR));
			//hcep_error_handler(hcd, 2,3);
			aotg_clearbits((1<<AUTO_CTRL_INTIRQ_MASK),HCIN2AUTOCTLUP);
			invalidate_ep_reg(HCIN2CON);

			aotg_clear_pend(1<<2,HCIN04ERRIRQ);
			set_ep_idle();
			if(get_ep_errtype(HCIN2ERR)){
				printk("HCIN2ERR:%x\n",aotg_readb(HCIN2ERR));
				aotg_hcep_reset(0x02,ENDPRST_FIFOREST);
			}
			//aotg_writeb(1<<5,HCIN2ERR);
			//aotg_hcep_reset(0x02,ENDPRST_FIFOREST);
			stop_timer(irqvector);
			break;
			
		case  UIV_HCIN3ERR:
			//ERR("<am7x_hcd>@@@---hcin3terr:%x\n",aotg_readb(HCIN3ERR));
			//hcep_error_handler(hcd, 3,5);
			aotg_clearbits((1<<AUTO_CTRL_INTIRQ_MASK),HCIN3AUTOCTLUP);
			invalidate_ep_reg(HCIN3CON);

			aotg_clear_pend(1<<3,HCIN04ERRIRQ);
			set_ep_idle();
			if(get_ep_errtype(HCIN3ERR)){
				printk("HCIN3ERR:%x\n",aotg_readb(HCIN3ERR));
				aotg_hcep_reset(0x03,ENDPRST_FIFOREST);
			}
			//aotg_writeb(1<<5,HCIN3ERR);
			//aotg_hcep_reset(0x03,ENDPRST_FIFOREST);
			stop_timer(irqvector);
			break;
			
		case  UIV_HCIN4ERR:
			//ERR("<am7x_hcd>@@@---hcin4terr:%x\n",aotg_readb(HCIN4ERR));
			//hcep_error_handler(hcd, 4,7);
			aotg_clearbits((1<<AUTO_CTRL_INTIRQ_MASK),HCIN4AUTOCTLUP);
			invalidate_ep_reg(HCIN4CON);

			aotg_clear_pend(1<<4,HCIN04ERRIRQ);
			set_ep_idle();
			if(get_ep_errtype(HCIN4ERR)){
				printk("HCIN4ERR:%x\n",aotg_readb(HCIN4ERR));
				aotg_hcep_reset(0x04,ENDPRST_FIFOREST);
			}
			//aotg_writeb(1<<5,HCIN4ERR);
			//aotg_hcep_reset(0x04,ENDPRST_FIFOREST);
			stop_timer(irqvector);
			break;

		case  UIV_HCIN5ERR:
			//ERR("<am7x_hcd>@@@---hcin5terr:%x\n",aotg_readb(HCIN5ERR));
			//hcep_error_handler(hcd, 5,9);
			aotg_clearbits((1<<AUTO_CTRL_INTIRQ_MASK),HCIN5AUTOCTLUP);
			invalidate_ep_reg(HCIN5CON);
			
			aotg_clear_pend(1<<5,HCIN04ERRIRQ);
			set_ep_idle();
			if(get_ep_errtype(HCIN5ERR)){
				printk("HCIN5ERR:%x\n",aotg_readb(HCIN5ERR));
				aotg_hcep_reset(0x05,ENDPRST_FIFOREST);
			}
			
			stop_timer(irqvector);
			break;

		case  UIV_HCIN6ERR:
			//ERR("<am7x_hcd>@@@---hcin4terr:%x\n",aotg_readb(HCIN6ERR));
			//hcep_error_handler(hcd, 6,11);
			aotg_clearbits((1<<AUTO_CTRL_INTIRQ_MASK),HCIN6AUTOCTLUP);
			invalidate_ep_reg(HCIN6CON);

			aotg_clear_pend(1<<6,HCIN04ERRIRQ);
			set_ep_idle();
			if(get_ep_errtype(HCIN6ERR)){
				printk("HCIN6ERR:%x\n",aotg_readb(HCIN6ERR));
				aotg_hcep_reset(0x06,ENDPRST_FIFOREST);
			}
			stop_timer(irqvector);
			break;
			
		default:
			printk("<am7x_hcd>@@@---aotg_hcd doesn't care this IRQ 0x%x\n",irqvector);
			break;
	}
	
	return 	IRQ_HANDLED;
}


static void hcd_plugin_event(struct aotg_hcd* _acthcd)
{
	struct aotg_hcd *acthcd = _acthcd;
	unsigned long flags;
	INFO("<am7x_hcd>@@@---<PLUG IN EVENT >\n");
	spin_lock_irqsave(&acthcd->lock, flags);
	acthcd->transceiver->port_change |= USB_PORT_STAT_C_CONNECTION;
	acthcd->inserted = 1;
	acthcd->transceiver->port_status  |= USB_PORT_STAT_CONNECTION;
	spin_unlock_irqrestore(&acthcd->lock, flags);
}


static void hcd_plugout_event(struct aotg_hcd*_acthcd)
{
	struct aotg_hcd *acthcd = _acthcd;
	struct aotg_hcep *ep;
	unsigned long flags;
	int i;
	
	INFO("<am7x_hcd>@@@---<PLUG OUT EVENT>\n");
	
	spin_lock_irqsave(&acthcd->lock, flags);
	acthcd->inserted = 0;	
	acthcd->transceiver->port_status &= ~(USB_PORT_STAT_ENABLE
				| USB_PORT_STAT_LOW_SPEED
				| USB_PORT_STAT_HIGH_SPEED
				| USB_PORT_STAT_CONNECTION);
	acthcd->transceiver->port_change = 0;
	acthcd->transceiver->port_change |=  USB_PORT_STAT_C_CONNECTION;
	acthcd->portstate = AOTG_PORT_NOATTACHED;
	spin_unlock_irqrestore(&acthcd->lock, flags);
	
	/*reset all ep-in */
	for(i=1;i <AOTG_HCD_NUM_ENDPOINTS; i++){
		ep = acthcd->ep_list[i];
		if(!ep)	continue;
		
		if(use_dma && ep->dma_working)
			aotg_hcd_cancel_dma(ep,acthcd);		
		//aotg_hcep_reset(acthcd->ep_list[i]->mask,ENDPRST_FIFOREST /*|ENDPRST_TOGRST*/);
	}
	
	INFO("<am7x_hcd>@@@---<PLUG OUT EVENT>exit\n");
}



static void hcd_abusdrop_event(struct aotg_hcd*_acthcd)
{
	struct aotg_hcd *acthcd = _acthcd;	
	unsigned long flags;
       printk(KERN_ALERT"<am7x_hcd>@@@---<VBUS DROP EVENT>\n");
	   
	spin_lock_irqsave(&acthcd->lock, flags);
	acthcd->transceiver->port_status &= ~(USB_PORT_STAT_CONNECTION |
					USB_PORT_STAT_ENABLE |
					USB_PORT_STAT_LOW_SPEED |
					USB_PORT_STAT_HIGH_SPEED |
					USB_PORT_STAT_SUSPEND);
	acthcd->transceiver->port_change |=  USB_PORT_STAT_C_CONNECTION ;
	acthcd->portstate= AOTG_PORT_POWEROFF;
	spin_unlock_irqrestore(&acthcd->lock, flags);
}


static int  aotg_portevent_notify(void *hcd_target,int event)
{
	struct aotg_hcd *acthcd = hcd_target;
#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
	int usb_cur_inout_status = get_cur_usb_inout_status();
#endif
	DMSG_OTG("<am7x_hcd>@@@---<HCD_NOTIFY> event %08x\n",event);
	switch(event) {
		case AOTG_PLUGIN_EVENT:
		case AOTG_PERIPHERAL_TO_HOST_EVENT:
#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
			printk("<am7x_hcd>@@@---<HCD_NOTIFY> Current status %d\n",usb_cur_inout_status);
			// T166,William 20150814: send "plug in" event only current status is "plug out"
			if (usb_cur_inout_status == USB_CUR_STATUS_PLUG_OUT)
			{
				printk("<am7x_hcd>@@@---<HCD_NOTIFY> William: Ignore HCD Plug in event\n");
//				set_cur_usb_inout_status(USB_CUR_STATUS_PLUG_IN);
//				hcd_plugin_event(acthcd);
			}
			else
			{
				printk("<am7x_hcd>@@@---<HCD_NOTIFY> Ignore plug in event because current status is plug in\n");
			}
#else
			hcd_plugin_event(acthcd);
#endif
			break;
		case AOTG_PLUGOUT_EVENT:
		case AOTG_HOST_TO_PERIPHERAL_EVENT:
#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
			// T166,William 20150814: send "plug out" event only current status is "plug in"
			printk("<am7x_hcd>@@@---<HCD_NOTIFY> Current status %d\n",usb_cur_inout_status);
			if (usb_cur_inout_status == USB_CUR_STATUS_PLUG_IN)
			{
				set_cur_usb_inout_status(USB_CUR_STATUS_PLUG_OUT);
				hcd_plugout_event(acthcd);
			}
			else
			{
				printk("<am7x_hcd>@@@---<HCD_NOTIFY> Ignore plug out event because current status is plug out\n");
			}
#else
			hcd_plugout_event(acthcd);
#endif
			break;
		case AOTG_VBUS_DROP_EVENT:
		case AOTG_ID_CHANGE_EVENT:
			hcd_abusdrop_event(acthcd);
			break;		
		default:
			ERR("<am7x_hcd>@@@---unknown portevent");
			break;
	}
	return 0;
}

static int aotg_hcd_start(struct usb_hcd *hcd)
{
	struct  aotg_hcd  * acthcd = hcd_to_aotg(hcd);
	int retval = 0;
    int otg_set_host_count = 0;
	struct port_handler *phandler;
	unsigned long flags;

	phandler = kmalloc(sizeof(struct port_handler),GFP_KERNEL);
	if(!phandler){
		ERR("<am7x_hcd>@@@---%s:malloc failed\n",__FUNCTION__);
		return  -ENOMEM;
	}

	acthcd->private = phandler;
	phandler->data = (unsigned long)acthcd;
	phandler->function = aotg_portevent_notify;
	retval = otg_set_porthandler(acthcd->transceiver,phandler);
	if(retval){
		ERR("<am7x_hcd>@@@---%s porthandler register failed,erro:%d\n", hcd->product_desc,retval);
		goto end;
	}

    /* Avoid some usb device can not work. 
     * Wait usb status to HOTPLUG_STAT_IN_CHECKING
     * James Chiang 2016.06.02
     */
	retval = 1;
    while( retval != 0 && otg_set_host_count < 100 ) {
        otg_set_host_count++;
	    retval = otg_set_host(acthcd->transceiver,&hcd->self);
    }
    
	if(retval){
		otg_set_porthandler(acthcd->transceiver,NULL);
		ERR("<am7x_hcd>@@@---%s bind to otg driver %s  error %d\n", hcd->product_desc,
		       acthcd->transceiver->label, retval);
		goto end;
	}
	
	spin_lock_irqsave(&acthcd->lock,flags);
	hcd->state = HC_STATE_RUNNING;
	hcd->uses_new_polling = 1;
	hcd->poll_rh = 1;
	
	/*  
	  * liucan:if set-host success ,
	  *	inform hub that we have a device plugin
	  */
#ifndef CONFIG_USB_AM7X_HOST_ONLY_0
    
	acthcd->inserted = 1;
	acthcd->transceiver->port_change |= USB_PORT_STAT_C_CONNECTION;
	acthcd->transceiver->port_status  |= USB_PORT_STAT_CONNECTION;
#else
    /*
     * For host only mode, there may be no device at all.
     * Modify by Simon Lee.
     */
    printk("<am7x_hcd>@@@-------> already connected,%d\n",acthcd->transceiver->state);
    if(acthcd->transceiver && acthcd->transceiver->state == OTG_STATE_A_HOST){
        acthcd->inserted = 1;
	    acthcd->transceiver->port_change |= USB_PORT_STAT_C_CONNECTION;
	    acthcd->transceiver->port_status  |= USB_PORT_STAT_CONNECTION;
    }
    
#endif

	spin_unlock_irqrestore(&acthcd->lock, flags);
	DMSG_HCD("<am7x_hcd>@@@---%s: ok!!\n",__FUNCTION__);
	return 0;
	
end:
	kfree(phandler);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE)&& MODULE_CONFIG_EZCASTPRO_MODE==8074
	// T166,William 20150817: atog hcd start failed so change plug in status to plug out
	printk("<am7x_hcd>@@@---<ATOG_HCD_START> William: HCD start error. Change plug in to plug out\n");
	if (get_cur_usb_inout_status() == 1)
	{
		set_cur_usb_inout_status(USB_CUR_STATUS_PLUG_OUT);
	}
	usb_hotplug_msg(MSG_HOST_RAW,0,1);// T932
#endif
	return retval;	
}


static void aotg_hcd_stop(struct usb_hcd *hcd)
{
	struct aotg_hcd  *acthcd = hcd_to_aotg(hcd);
	
	acthcd->portstate= AOTG_PORT_POWEROFF;
	DMSG_HCD("<am7x_hcd>@@@---stop AOTG USB Host Controller\n");
	otg_set_power(acthcd->transceiver,0);
	DMSG_HCD("<am7x_hcd>@@@---power off ok\n");
	otg_set_host(acthcd->transceiver,NULL);
	otg_set_porthandler(acthcd->transceiver,NULL);
	if(acthcd->private){
		kfree(acthcd->private);
		acthcd->private = NULL;
	}
	
	DMSG_HCD("<am7x_hcd>@@@---stop AOTG USB Host Controller\n");
}


static void handle_setup_packet(struct aotg_hcep *ep,struct urb *urb) 
{
	u32  *buf ;
	struct usb_ctrlrequest  *ctrlreq;
	int addr = HCEP0OUTDAT0;
	u16	w_value ,w_index,w_length;

	buf = (u32*)urb->setup_packet;
	ctrlreq = (struct usb_ctrlrequest *)urb->setup_packet;

	w_value = ctrlreq->wValue; 
	w_index  = ctrlreq->wIndex;
	w_length = ctrlreq->wLength;
	
	DMSG_CTRL("<am7x_hcd>@@@---<CTRL>SETUP stage  %02x.%02x V%04x I%04x L%04x\n ",
		ctrlreq->bRequestType, ctrlreq->bRequest,w_value,w_index,w_length);

	/*initialize the setup stage*/
	
	aotg_setbits(EP0CS_HCSET,EP0CS);
	udelay(1);
	/*initialize the setup stage*/
	//printk(KERN_ALERT "SETUP :send setup token instead\n" );
	/*fill the setup data in fifo*/
	aotg_writel(*buf++,addr);
	addr += 4;
	aotg_writel(*buf,addr);
	aotg_writeb(8,HCOUT0BC);
}



static int int_out_packet(struct aotg_hcd *acthcd,
	struct aotg_hcep	*ep,
	struct urb* urb)
{
	if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)) {
		DMSG_HCD("<am7x_hcd>@@@---%s\n","hcin2ep cs not busy");		
		{
			unsigned int frame ;
			frame = aotg_readb(0xb00e01ac) | (aotg_readb(0xb00e01ad)<<8);
			DMSG_HCD("<am7x_hcd>@@@---hcepcs=%02x\n",aotg_readb(ep->reg_hcepcs));
			DMSG_HCD("<am7x_hcd>@@@---frame=%d\n",frame);
		}
		pio_irq_clear(ep->mask);
		write_hcep_fifo(acthcd,ep,urb);
	}	

	//validate_ep(ep);
	DMSG_HCD("<am7x_hcd>@@@---<ASYNC XFERS>bulk ep%din busy,%d\n"
			,ep->epnum,
			aotg_readb(ep->reg_hcepcs));
	return 0;
}

static int int_in_packet(struct aotg_hcd *acthcd,
	struct aotg_hcep	*ep,
	struct urb* urb)
{
	if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)) {
		//invalidate_ep(ep );
		DMSG_HCD("<am7x_hcd>@@@---%s\n","hcin2ep cs not busy");		
		{
			unsigned int frame ;
			frame = aotg_readb(0xb00e01ac) | (aotg_readb(0xb00e01ad)<<8);
			DMSG_HCD("hcepcs=%02x\n",aotg_readb(ep->reg_hcepcs));
			DMSG_HCD("frame=%d\n",frame);
		}
		pio_irq_clear(ep->mask);
		read_hcep_fifo(acthcd,ep,urb);
	}
	
	//validate_ep(ep);
	DMSG_HCD("<am7x_hcd>@@@---<ASYNC XFERS>bulk ep%din busy,%d\n"
			,ep->epnum,
			aotg_readb(ep->reg_hcepcs));
	return 0;
}

static int iso_in_packet(struct aotg_hcd *acthcd,
	struct aotg_hcep	*ep,
	struct urb* urb)
{
	if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)) {
		read_hcep_fifo(acthcd,ep,urb);
		//INFO("<am7x_hcd>@@@---%s: pre valid ep\n",__FUNCTION__);
		//validate_ep(ep );
	}
	return 0;
}

static inline int compute_frame_remainder(struct aotg_hcep *ep,struct urb* urb)
{
	int fclock;
	
	/* if this frame doesn't have enough time left to transfer this
	 * packet, wait till the next frame.  too-simple algorithm...
	 */
	fclock = aotg_readb(HCFRMREMAINL);
	fclock |= (aotg_readb(HCFRMREMAINH) & 0xf )<<8;
	
	fclock -= 100;		/* setup takes not much time */
	if (urb->dev->speed == USB_SPEED_LOW) {
		fclock -= ep->maxpacket << 8;
	} else {
		fclock -= 12000 / 19;	/* 19 64byte packets/msec */
	}
	return 0;
}


static int start_periodic_work(struct aotg_hcd *acthcd,struct aotg_hcep *ep)
{
	 struct urb* urb;
	 
	if (list_empty(&ep->hep->urb_list)) {
		WARN("<am7x_hcd>@@@---empty %p queue?epcs:%02x\n", ep,aotg_readb(ep->reg_hcepcs));
		return -1;
	}

	urb = container_of(ep->hep->urb_list.next, struct urb, urb_list);
	if ( ep->type == PIPE_ISOCHRONOUS){
		if ( ep->nextpid == USB_PID_IN)
			iso_in_packet(acthcd, ep, urb);
		else 
			DMSG_HCD("<am7x_hcd>@@@---%s:%d not support\n",__FUNCTION__, __LINE__);
	}else if ( ep->type == PIPE_INTERRUPT){
		switch (ep->nextpid) {	
		case USB_PID_IN:
			int_in_packet(acthcd, ep, urb);
			break;
		case USB_PID_OUT:
			DMSG_HCD("<am7x_hcd>@@@---interrupt pid out\n");
			int_out_packet(acthcd, ep, urb);
			break;
		default:
			DMSG_HCD("<am7x_hcd>@@@--- bad ep%p pid %02x\n", ep, ep->nextpid);
			ep = NULL;
			break;
		}
	}
	
	return 0;
}




static int start_periodic_transfer(struct aotg_hcd *acthcd,struct aotg_hcep	*mep)
{
	struct aotg_hcep	*ep;
	int result;

	BUG_ON(mep &&!mep->period);
	if ( mep && (mep != acthcd->next_periodic) ){
		ep = mep;
		return start_periodic_work(acthcd,ep);
	}	

	if(!acthcd->next_periodic)
		return 0;

	ep = acthcd->next_periodic;
	while(ep != NULL){
		result = start_periodic_work(acthcd, acthcd->next_periodic); 
		ep = acthcd->next_periodic = ep->next;
	}
	return result;
}


static void hcd_set_fifoauto(struct aotg_hcep * ep,int is_on)
{
	u8 val = is_on  ? ( FIFOAUTO |ep->mask ) : ep->mask;
	DMSG_DMA("<am7x_hcd>@@@---FIFOAUTO VAL:%x\n",val);
	aotg_writeb(val,FIFOCTRL);			
}

static void  hcd_dma_select_ep(struct aotg_hcep *ep)
{
	u8 val    = ((ep->mask & 0x0f) -1) * 2;
	int base = (ep->mask & USB_HCD_OUT_MASK) ?  0x02 : 0x03;

	val += base;
	DMSG_DMA("<am7x_hcd>@@@---DMAEPSEL VAL:%x\n",val);
	aotg_writeb(val,DMAEPSEL);
}


static int aotg_hcd_start_dma(struct aotg_hcep *ep, struct urb *urb,u32 len,struct aotg_hcd *acthcd)
{
	u32 	dcmd = 0;
	u32 	fifo_addr,buf_addr;
	u32	pipe = urb->pipe;
	u32 	buffer_nr = 0;
	u32  tmp = 0;
	
	DMSG_DMA("<am7x_hcd>@@@---<start_dma>:ep->reg_hcmaxpack(%x)\n",(u32)ep->reg_hcmaxpck);
	DMSG_DMA("<am7x_hcd>@@@---<start_dma>:ep HC IN epnum (%x) fifo(%x)\n",(u32)ep->epnum,(unsigned int)ep->reg_hcfifo);

	/*  liucan:we cannot use transfer_dma ,for we not indicate hcd 
		support dma ,another reason is our dma api not standard with linux*/

	buf_addr		=(u32)urb->transfer_buffer + urb->actual_length;
	fifo_addr 	= ep->reg_hcfifo;
	dcmd 		= len;
	
	if( dcmd > (urb->transfer_buffer_length - urb->actual_length) ){
		DMSG_DMA("<am7x_hcd>@@@---<start_dma>: ERROR:dcmd  > buf left\n");
		return -1;
	}
	
	dma_cache_wback_inv(buf_addr,dcmd);
	/*open fifo auto*/
	hcd_set_fifoauto(ep,1);
	hcd_dma_select_ep(ep);	
	
	DMSG_DMA("<am7x_hcd>@@@---<start_dma>:HC  IN:urb->transfer_dma =%x ,transfer_buffer = %x actual_length = %x\n",
			(unsigned int )urb->transfer_dma,(unsigned int )urb->transfer_buffer,(unsigned int )urb->actual_length);
	DMSG_DMA("<am7x_hcd>@@@---<start_dma>:HC  IN:buf = %x , fifo = %x ,dcmd =%x \n",buf_addr,fifo_addr,dcmd ); 

	/*host read*/
	if(usb_pipein(pipe)){		
		ep->dma_bytes = dcmd;
		/*config dma ep and set fifo auto*/
		am_dma_cmd(acthcd->dma_nr,DMA_CMD_RESET);
		am_set_dma_addr(acthcd->dma_nr,fifo_addr, buf_addr);		
		//dma_mode = hostread_dmamode(BURST_SINGLE);
		//printk(KERN_ALERT "dma_mode:%x\n",dma_mode);
		am_dma_config(acthcd->dma_nr,HOST_READ_MODE,dcmd);
	}else{
		/*maybe has bug ,can not use long dma 
		    when single buffer.because dma don`t 
		    waitting whether the buffer is busy or not
		 */	 
		tmp  = aotg_readb(ep->reg_hcepcs);
		if(tmp  & EPCS_BUSY){	
			DMSG_DMA("\n<am7x_hcd>@@@---<start_dma>  reg_hcepcs = %x is busy = %x...\n",
				aotg_readb(ep->reg_hcepcs),
				aotg_readb(ep->reg_hcepcs) & EPCS_BUSY);
			
			while( (aotg_readb(ep->reg_hcepcs) & EPCS_BUSY) != 0);
			buffer_nr = 1;	
		}else{
			buffer_nr = tmp & 0x0c;
			buffer_nr = buffer_nr >> 2;
			buffer_nr += 1;
			
			if(buffer_nr < 4)
				DMSG_DMA("<am7x_hcd>@@@---<start_dma> buffer_nr =%x \n",buffer_nr);					
		}

		dcmd = min(dcmd ,(u32)(buffer_nr * ep->maxpacket));
		ep->dma_bytes = dcmd;
		
		am_dma_cmd(acthcd->dma_nr,DMA_CMD_RESET);
		am_set_dma_addr(acthcd->dma_nr,buf_addr, fifo_addr);	
		//dma_mode = hostwrite_dmamode(BURST_SINGLE);
		am_dma_config(acthcd->dma_nr,HOST_WRITE_MODE,dcmd);
	}
	
	DMSG_DMA("<am7x_hcd>@@@---<start_dma>:HCIN04IEN = %x HCIN04IRQ = %x , HCOUT04IRQ = %x,IRQVECT = %x,ep->mask = %x\n",
			aotg_readb(HCIN04IEN),
			aotg_readb(HCIN04IRQ),
			aotg_readb(HCOUT04IRQ),
			aotg_readb(IVECT),
			ep->mask);

	am_set_dma_irq(acthcd->dma_nr,1,0);
	am_dma_start(acthcd->dma_nr,0);
	
	DMSG_DMA("<am7x_hcd>@@@---<start_dma>:arm end........\n");
	DMSG_DMA("<am7x_hcd>@@@---<start_dma>:HCIN04IEN = %x HCIN04IRQ = %x , HCOUT04IRQ = %x,ep->mask = %x\n",
			aotg_readb(HCIN04IEN),
			aotg_readb(HCIN04IRQ),
			aotg_readb(HCOUT04IRQ),
			ep->mask);
	
	DMSG_DMA( "<am7x_hcd>@@@---DMAIRQPD:%x\n",aotg_readl(DMA_IRQPD));	

	return 0;
}

static int aotg_hcd_dma_write(struct aotg_hcd *acthcd,struct aotg_hcep *ep, struct urb *urb)
{
	struct usb_device *udev;
	u32 temp = 0;
	u32 buffer_nr = 0;
	u32 dcmd = urb->transfer_buffer_length - urb->actual_length;

	pio_irq_disable(ep->mask);
	
	//dma is already working,do nothing 
	if(ep->dma_working == 1){
		DMSG_DMA("<am7x_hcd>@@@---<dma_read>:dma write is runed ...waitting\n");
		return -EBUSY;
	}
	
	ep->dma_working = 1;
	
	//short dma 1 packet
	temp = aotg_readb(ep->reg_hcepcs);
	if(temp & EPCS_BUSY){
		DMSG_DMA("<am7x_hcd>@@@---<dma_write>: ep OUT busy\n");
		while((aotg_readb(ep->reg_hcepcs) & EPCS_BUSY) != 0);
		buffer_nr = 1;
	}else{
		buffer_nr = temp & 0x0c;
		buffer_nr = buffer_nr >> 2;
		buffer_nr += 1;
		if(buffer_nr < 4)
			DMSG_DMA("<am7x_hcd>@@@---<dma_write>:buffer_nr =%x \n",buffer_nr);
	}
	dcmd = min(dcmd , ((u32)(buffer_nr * ep->maxpacket) ));


	DMSG_DMA("<am7x_hcd>@@@---<dma_write>: dcmd = %x,addr=%08x\n",
			dcmd,urb->transfer_dma);
	
	aotg_hcd_start_dma(ep , urb ,dcmd,acthcd);
	
	//use dotoggle??
	udev = urb->dev;
	usb_dotoggle(udev, ep->epnum, 0);	
	
	return 0;
}


static int aotg_hcd_dma_read(struct aotg_hcd *acthcd,struct aotg_hcep *ep, struct urb *urb)
{
	struct usb_device *udev;
	
	pio_irq_disable(ep->mask);	
	
	//dma is already working
	if(ep->dma_working == 1){
		DMSG_DMA("<am7x_hcd>@@@---<dma_read>:ERROR:dma read is runed .....return ...\n");
		return -EBUSY;
	}	
	
	ep->dma_working = 1;
	aotg_hcd_start_dma(ep , urb ,(urb->transfer_buffer_length - urb->actual_length),acthcd );
	udev = urb->dev;
	usb_dotoggle(udev, ep->epnum, 1);

	return 0;
}



static irqreturn_t aotg_hcd_dma_handler(int irq, void *_dev)
{
	struct urb *urb;
	u32 remains;
	u32 is_in ;
	int  i,completed;
	struct aotg_hcd *acthcd = (struct aotg_hcd *)_dev;
	struct aotg_hcep *ep = NULL;
	
//	DMSG_DMA("<dma_handler>..........\n");
//	DMSG_DMA( "DMAIRQPD:%x\n",aotg_readl(DMA_IRQPD));	
//	DMSG_DMA("<dma_handler>: dma_nr:%d\n",acthcd->dma_nr);

	if(!am_get_dma_irq(acthcd->dma_nr,1,0) ){	
//		DMSG_DMA( "CH:%d,IRQ not  us\n",acthcd->dma_nr);
//		DMSG_DMA( "DMAIRQPD:%x\n",aotg_readl(DMA_IRQPD));			
		return IRQ_NONE;
	}

	for(i=0;i<AOTG_HCD_NUM_ENDPOINTS;i++){
		if(acthcd->ep_list[i] != NULL){
			if(acthcd->ep_list[i]->dma_working == 1){
				ep = acthcd->ep_list[i];
				break;
			}
		}
	}

	if(!ep ||ep->dma_working  == 0){
		ERR("<am7x_hcd>@@@---<dma_handler> ERR,DMAIRQPD:%x\n",aotg_readl(DMA_IRQPD));
		return IRQ_NONE;
	}
	
	am_clear_dma_irq(acthcd->dma_nr,1,0);
	/*close fifo auto*/
	hcd_set_fifoauto(ep, 0);
	
	urb = container_of(ep->hep->urb_list.next,struct urb,urb_list);
	is_in = (usb_pipein(urb->pipe) & USB_DIR_IN) ? 1 : 0;

	/*update transfer status*/
	urb->actual_length += ep->dma_bytes;
	remains = urb->transfer_buffer_length - urb->actual_length;
	completed = 0;
	if( is_in){
		if (remains > 0 && remains < DMA_LEAST_BYTE ){
		//	DMSG_DMA("<dma_handler>:--1-- length - actual <maxpacket (short packet)\n");	
			if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)) {
				pio_irq_clear(ep->mask);
				completed = read_hcep_fifo(acthcd,ep,urb);
			}else{
		//		DMSG_DMA("dma_handler:have bug\n");
				pio_irq_enable(ep->mask);
			}			
			
			//this req is complete	//????
			ep->dma_working = 0; 
			completed = 1;	
		}else if(remains == 0){ /*transfer complete*/
		//	DMSG_DMA("<dma_handler>:--3-- call done()\n");
			finish_request(acthcd,ep,urb);
			ep->dma_working = 0;
			completed = 1;
		}else{			
		//	DMSG_DMA("<dma_handler>:--2-- length - actual >= ep->maxpacket\n");
		//	DMSG_DMA("<dma_handler>:--2-- urb->transfer_buffer_length = %x ,urb->actual_length = %x\n",urb->transfer_buffer_length,urb->actual_length);
			/*finish this dma work,start a new dma handler*/
			ep->dma_working = 0;	
			aotg_hcd_dma_read(acthcd,  ep,  urb);
			completed = 0;
		}		
	}else{
		if ( remains > 0&& remains < ep->maxpacket){
		//	DMSG_DMA("<dma_hander>:OUT : short packet\n");
			write_hcep_fifo(acthcd,  ep,  urb);
			completed = 1;
			ep->dma_working = 0;
			//maybe not compeleted
		}else if(remains == 0){
		//	DMSG_DMA("<dma_hander>:OUT: packet compeleted\n");
			completed = 1;
			ep->dma_working = 0;
			finish_request(acthcd,ep,urb/*,NULL*/);		
		}else{
			completed = 0;
			DMSG_DMA("<am7x_hcd>@@@---<dma_hander>:***continue !!***\n");
			ep->dma_working = 0; //complete the dma then start a new dma
			aotg_hcd_dma_write(acthcd,  ep,  urb);
		}		
	}
	
	if(completed == 1){
	//	DMSG_DMA("<dma_handler>: completed == 1----\n");
		start_async_transfer(acthcd, ep);
	}else{
		DMSG_DMA("<am7x_hcd>@@@---<dma_handler>: completed == 0\n");
	}
	
	return IRQ_HANDLED;
}


static void aotg_hcd_cancel_dma(struct aotg_hcep * ep , struct aotg_hcd *acthcd)
{
	ep->dma_working = 0;
	am_dma_cmd(acthcd->dma_nr,DMA_CMD_PAUSE);
	am_dma_cmd(acthcd->dma_nr,DMA_CMD_RESET);
	
	/*close fifo auto*/
	hcd_set_fifoauto(ep,0);
	
	/*flush fifo & set toggle*/
	aotg_hcep_reset(ep->mask,ENDPRST_FIFOREST);

	/*if irq ,clear &disable irq*/
	am_clear_dma_irq(acthcd->dma_nr,1,0);
	ep->dma_working = 0;
	pio_irq_enable(ep->epnum);
	
	DMSG_DMA("<am7x_hcd>@@@---<cancel dma >:epmask:%d \n",ep->mask);		
 }


invalidate_all_ep(){
	unsigned char reg_val;
	unsigned char i;
	unsigned int reg;
	for(i=0;i<4;i++){

		reg = OUT1CON+i*8;
		do {
			reg_val = aotg_readb(reg);
			aotg_writeb(reg_val &~EPCON_VAL,reg);
		}while(reg_val & EPCON_VAL);

		//reg = IN1CON+i*8;
		//do {
			//reg_val = aotg_readb(reg);
			//aotg_writeb(reg_val &~EPCON_VAL,reg);
		//}while(reg_val & EPCON_VAL);
		
	}

	//printk("in_ep:%x %x %x %x \n",aotg_readb(OUT1CON+0*8),aotg_readb(OUT1CON+1*8),aotg_readb(OUT1CON+2*8),aotg_readb(OUT1CON+3*8));
	//printk("out_ep:%x %x %x %x \n",aotg_readb(IN1CON+0*8),aotg_readb(IN1CON+1*8),aotg_readb(IN1CON+2*8),aotg_readb(IN1CON+3*8));
}


void not_support(void){
	while(1){
		printk("not support\n");
	}
}


static int wait_bulk_out_transfer_end;


static int start_transfer(struct aotg_hcd *acthcd,struct aotg_hcep *aim)
{
	struct aotg_hcep *ep=aim;
	struct urb *urb;
    int  completed = 0;
 	int 	rw_bytes;
	unsigned long flags;
	unsigned char frame;
	
	//printk("start_schedule:type:%x pid:%x \n",ep->type,ep->nextpid);

	if (unlikely(list_empty(&ep->hep->urb_list))) {
		printk("<jjf_debug> start_transfer error,urb list empty ,ep is %x %d\n",ep,ep->mask);
		set_ep_idle();
		return 0;
	}

	//printk("<start_transfer_start>%x\n",ep);
	
	
	urb = container_of(ep->hep->urb_list.next,struct urb,urb_list);
	rw_bytes = urb->transfer_buffer_length - urb->actual_length;
	BUG_ON(rw_bytes < 0);


	//printk("urb :%x %d %d\n",urb,urb->num,urb->status);

	if (urb->status != -EINPROGRESS) {
		printk("start_transfer, urb status is error %d!\n",urb->status);
		finish_request(acthcd,ep, urb);
		goto start_transfer_END;
	}


	

	//invalidate_all_ep();

	/*Wait the next sof*/
	//sofEnable=0;
	//aotg_clear_pendbits(USBIRQ_SOF,USBIRQ);
	//sof_enable();

	//frame = aotg_readb(AOTG_REG(0x1ac));
	//while((sofEnable == 0) && (aotg_readb(AOTG_REG(0x1ac)) == frame));
	

	//if(ep->index)
		//validate_ep(ep);



	aotg_writeb(usb_pipedevice(urb->pipe),FNADDR);
	if(ep->index == 0){
		aotg_writeb(ep->epnum,HCEP0CTRL);
	}else{
		aotg_writeb(ep->epnum,ep->reg_hcepctrl);
	}


	//printk("<enqueue_start_%d_%d_%d_%d_%d>\n",urb->num,usb_pipedevice(urb->pipe),usb_pipeendpoint(urb->pipe),usb_pipetype(urb->pipe),usb_pipeout(urb->pipe));
	
#if 0

	/*Wait the next sof*/
	sofEnable=0;
	sof_enable();
	frame = aotg_readb(AOTG_REG(0x1ac));
	while((sofEnable == 0) && (aotg_readb(AOTG_REG(0x1ac)) == frame));
#endif
	
	//printk("trans:%d\n",ep->index);


	switch(ep->type){
		case PIPE_CONTROL:
			switch(ep->nextpid){
				case USB_PID_SETUP:
					//printk("<start_transfer_%d> CONTROL SETUP Start\n",urb->num);
					//printk("<CTL_SETUP_%d_%d_%d_%d_%d>\n",urb->num,usb_pipedevice(urb->pipe),usb_pipeendpoint(urb->pipe),usb_pipetype(urb->pipe),usb_pipeout(urb->pipe));
					
					handle_setup_packet(ep,urb);

					//if(get_ep_errtype(HCOUT0ERR) != EPERR_NONE){
						//printk("<start_transfer_%d> CONTROL SETUP busy Start\n",urb->num);	
						//aotg_writeb(1<<5,HCOUT0ERR);
					//}
					
					
					//printk("<start_transfer_%d> CONTROL SETUP End\n",urb->num);
					break;
				case USB_PID_OUT:
					//printk("<start_transfer_%d> CONTROL OUT Start\n",urb->num);
					//printk("<CTL_OUT_%d_%d_%d_%d_%d>\n",urb->num,usb_pipedevice(urb->pipe),usb_pipeendpoint(urb->pipe),usb_pipetype(urb->pipe),usb_pipeout(urb->pipe));
					
					
					write_hcep0_fifo(ep,urb);
					//if(get_ep_errtype(HCOUT0ERR) != EPERR_NONE){
						//printk("<start_transfer_%d> CONTROL SETUP busy Start\n",urb->num);	
						//aotg_writeb(1<<5,HCOUT0ERR);
					//}
					
					//printk("<start_transfer_%d> CONTROL OUT End\n",urb->num);
					break;
				case USB_PID_IN:
					//printk("<start_transfer_%d> CONTROL IN Start\n",urb->num);
					//aotg_writeb(usb_pipedevice(urb->pipe),FNADDR);
					//aotg_writeb(ep->epnum, HCEP0CTRL);
					//printk("<CTL_IN_%d_%d_%d_%d_%d>\n",urb->num,usb_pipedevice(urb->pipe),usb_pipeendpoint(urb->pipe),usb_pipetype(urb->pipe),usb_pipeout(urb->pipe));

					ep->length = min((int)ep->maxpacket,rw_bytes);
					aotg_writeb(0,HCIN0BC);/*arm HC IN EP0 for the IN transfer*/
					//if(get_ep_errtype(HCIN0ERR) != EPERR_NONE){
						//printk("<start_transfer_%d> CONTROL SETUP busy Start\n",urb->num);	
						//aotg_writeb(1<<5,HCIN0ERR);
					//}
					//printk("<start_transfer_%d> CONTROL IN End\n",urb->num);
					
					break;
				case USB_PID_ACK:
					//printk("<start_transfer_%d> CONTROL ACK Start\n",urb->num);
					//aotg_writeb(usb_pipedevice(urb->pipe),FNADDR);
					//aotg_writeb(ep->epnum, HCEP0CTRL);
					//printk("<CTL_ACK_%d_%d_%d_%d_%d>\n",urb->num,usb_pipedevice(urb->pipe),usb_pipeendpoint(urb->pipe),usb_pipetype(urb->pipe),usb_pipeout(urb->pipe));

					handle_status(usb_pipeout(urb->pipe));
					//if(usb_pipeout(urb->pipe)){
						//if(get_ep_errtype(HCIN0ERR) != EPERR_NONE){
							//printk("<start_transfer_%d> CONTROL SETUP busy Start\n",urb->num);	
							//aotg_writeb(1<<5,HCIN0ERR);
						//}
					//}else{
						//if(get_ep_errtype(HCOUT0ERR) != EPERR_NONE){
							//printk("<start_transfer_%d> CONTROL SETUP busy Start\n",urb->num);	
							//aotg_writeb(1<<5,HCOUT0ERR);
						//}
					//}
					
					//printk("<start_transfer_%d> CONTROL ACK End\n",urb->num);
					break;
				default:
					not_support();
					printk("<jjf_debug> start_transfer PIPE_CONTROL error stage:%d",ep->nextpid);
					break;
			}
			break;
#if 1
		case PIPE_BULK:
			switch(ep->nextpid){
				case USB_PID_IN:
					//printk("<start_trans_in_%d_%x_%x_%x>\n",urb->num,aotg_readb(HCIN04IEN),aotg_readb(HCIN04IRQ),aotg_readb(ep->reg_hcepcs));
					aotg_writeb(1<<5,ep->reg_hceperr);	
					aotg_setbits((1<<AUTO_CTRL_INTIRQ_MASK),ep->reg_hcin_auto_ctrl_up);
					mb();
					aotg_writeb(0,ep->reg_hcepcs); /*arm this ep(sub buffer) for the IN transaction*/
					validate_ep(ep);
					//aotg_writeb(1<<5,ep->reg_hceperr);
					//if(get_ep_errtype(ep->reg_hceperr)){
						//aotg_writeb(1<<5,ep->reg_hceperr);	
					//}
					mb();
					//ep->ep_schedule=1;
					break;
				case USB_PID_OUT:
					//printk("<start_trans_out_%d_%x_%x_%x>\n",urb->num,aotg_readb(HCOUT04IEN),aotg_readb(HCOUT04IRQ),aotg_readb(ep->reg_hcepcs));

					//aotg_clearbits(EP0CS_HCSET,EP0CS);
					//pio_irq_clear(ep->mask);					
					
					if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)){
						aotg_hcep_reset(ep->mask,ENDPRST_FIFOREST);
						write_packet(ep,urb,ep->maxpacket);
						//aotg_writeb(0,ep->reg_hcepcs); 
						//mb();
					}else{
						write_packet_busy(ep,urb,ep->maxpacket);
						aotg_writeb(1<<5,ep->reg_hceperr);	
					}
					//validate_ep(ep);
					aotg_writeb(0,ep->reg_hcepcs); 
					mb();
					//wait transfer end!
					//wait_bulk_out_transfer_end=1;
					//set_ep_idle();
					//return;

					
					
					
					
										
					
					break;
				default:
					printk("<jjf_debug> start_transfer PIPE_BULK error stage:%d\n",ep->nextpid);
					break;
			}
			break;
#endif
		case PIPE_INTERRUPT:
			switch(ep->nextpid){
				case USB_PID_IN:
#if 0
						if(get_ep_errtype(ep->reg_hceperr)==EPERR_NONE){
							if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)){
								printk("<ep:%d_%d_%x_%x_%x_%x_%x>  PIPE_INTERRUPT USB_PID_IN idle Start\n",ep->index,urb->num,ep->reg_hcepcs,aotg_readb(HCIN04IEN),aotg_readb(HCOUT04IEN),aotg_readb(HCIN04IRQ),aotg_readb(HCOUT04IRQ));
							}else{
								printk("<ep:%d_%d_%x_%x_%x_%x_%x>  PIPE_INTERRUPT USB_PID_IN busy Start\n",ep->index,urb->num,ep->reg_hcepcs,aotg_readb(HCIN04IEN),aotg_readb(HCOUT04IEN),aotg_readb(HCIN04IRQ),aotg_readb(HCOUT04IRQ));
							}
						}else{
							printk("<ep:%d_%d_%x_%x_%x_%x_%x>  PIPE_INTERRUPT USB_PID_IN error Start\n",ep->index,urb->num,ep->reg_hcepcs,aotg_readb(HCIN04IEN),aotg_readb(HCOUT04IEN),aotg_readb(HCIN04IRQ),aotg_readb(HCOUT04IRQ));
							aotg_writeb(aotg_readl(HCIN04ERRIRQ),HCIN04ERRIRQ);// clear error pending
							aotg_writeb(1<<5,ep->reg_hceperr); // do resend 
						}
						aotg_writeb(usb_pipedevice(urb->pipe),FNADDR);
						aotg_writeb(ep->epnum, ep->reg_hcepctrl);
#endif

						//printk("trans_in:%d\n",ep->index);
						//aotg_hcep_reset(ep->mask,ENDPRST_FIFOREST);
						aotg_writeb(1<<5,ep->reg_hceperr);	
						aotg_setbits((1<<AUTO_CTRL_INTIRQ_MASK),ep->reg_hcin_auto_ctrl_up);
						mb();
						aotg_writeb(0,ep->reg_hcepcs); /*arm this ep(sub buffer) for the IN transaction*/
						validate_ep(ep);
						//aotg_writeb(1<<5,ep->reg_hceperr);
						//if(get_ep_errtype(ep->reg_hceperr)){
							//aotg_writeb(1<<5,ep->reg_hceperr);	
						//}
						mb();
						ep->ep_schedule=1;
						
					break;
					case USB_PID_OUT:

						if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)){
							aotg_hcep_reset(ep->mask,ENDPRST_FIFOREST);
							write_packet(ep,urb,ep->maxpacket);
							//aotg_writeb(0,ep->reg_hcepcs); 
							//mb();
						}else{
							write_packet_busy(ep,urb,ep->maxpacket);
							aotg_writeb(1<<5,ep->reg_hceperr);	
						}
						//validate_ep(ep);
						aotg_writeb(0,ep->reg_hcepcs); 
						mb();
					break;
#if 0
				case USB_PID_OUT:
					not_support();
					if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)){
						printk("<start_transfer_%d_%x_%x> PIPE_INTERRUPT USB_PID_OUT Start\n",urb->num,ep,urb);
						aotg_writeb(usb_pipedevice(urb->pipe),FNADDR);
						aotg_writeb(ep->epnum, ep->reg_hcepctrl);

						write_packet(ep,urb,ep->maxpacket);
						
						printk("<start_transfer_%d_%x_%x> PIPE_INTERRUPT USB_PID_OUT End\n",urb->num,ep,urb);
					}else{
						printk("<jjf_debug> start_transfer PIPE_INTERRUPT_OUT error reg_hcepcs:%d\n",aotg_readb(ep->reg_hcepcs));
					}
					
					break;
#endif
				default:
					printk("<jjf_debug> start_transfer PIPE_INTERRUPT error stage:%d\n",ep->nextpid);
					break;
			}
			break;
		default:
			not_support();
			printk("<jjf_debug> start_transfer error pipe type :%d\n",ep->type);
			break;
	}

start_transfer_END:

//	mod_timer(&Send_token_timeout_timer,jiffies+msecs_to_jiffies(2));
	//mod_timer(&Send_token_timeout_timer,jiffies+msecs_to_jiffies(100));// test for jjf
	start_timer();
	return completed;


	



#if 0
	printk("<trans_urb_%d> ",urb->num);
	switch(ep->type){
		case PIPE_CONTROL:
			printk("type:PIPE_CONTROL ");
			break;
		case PIPE_BULK:
			printk("type:PIPE_BULK ");
			break;
		case PIPE_INTERRUPT:
			printk("type:PIPE_INTERRUPT ");
			break;
		case PIPE_ISOCHRONOUS:
			printk("type:PIPE_ISOCHRONOUS ");
			break;
		default:
			printk("type:PIPE_UNKONWN ");
			break;
	}

	
	switch(ep->nextpid){
		case USB_PID_IN:
			printk("stage:USB_PID_IN ");
			break;
		case USB_PID_OUT:
			printk("stage:USB_PID_OUT ");
			break;
		case USB_PID_ACK:
			printk("stage:USB_PID_ACK ");
			break;
		case USB_PID_SETUP:
			printk("stage:USB_PID_SETUP ");
			break;
		default:
			printk("stage:USB_PID_UNKONWN ");
			break;
	}
	printk("\n");

#endif
	

	//printk("%s %d ep->type:%x pid:%x\n",__func__,__LINE__,ep->type,ep->nextpid);
#if 0
	switch(ep->type){
	case PIPE_BULK:
		DMSG_BULK("<am7x_hcd>@@@---<ASYNC XFERS>BULK XFERS**>\n");		
		switch(ep->nextpid) {
		case USB_PID_IN:
			aotg_writeb(0,ep->reg_hcepcs); /*arm this ep(sub buffer) for the IN transaction*/
			mb();

			//printk("need_packet:len:%d tans_len:%d ep_len:%d\n",urb->transfer_buffer_length,\

			//urb->actual_length,ep->maxpacket);
			break;
		case USB_PID_OUT:
			if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)){
				write_packet(ep,urb,ep->maxpacket);
			}else{
				printk("<jjf_debug> start_async_transfer ,Some error happen in bulk out!\n");
			}
			break;
	    case USB_PID_ACK:
			if(ep->mask & USB_HCD_OUT_MASK){
				if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)) {
					//INFO( "ep-num%d,ep_mask:%x not busy\n",ep->epnum,ep->mask);

					//printk("<jjf_debug> start_async_transfer:%d reg_hcepcs:%x\n",__LINE__,aotg_readb(ep->reg_hcepcs));

					pio_irq_clear(ep->mask);
					completed = write_hcep_fifo(acthcd,ep,urb);
					ep->nextpid=USB_PID_OUT;
					if (completed && !list_empty(&ep->hep->urb_list) ){
						//printk("<jjf_debug> start_async_transfer:complete:%d %d\n",completed);
						write_packet(ep,urb,ep->maxpacket);
					}else if(completed == 0){
						//printk("<jjf_debug> start_async_transfer:go to transfer complete:%d %d\n",completed);
						write_packet(ep,urb,ep->maxpacket);
					}
				}else{
					printk("<jjf_debug> start_async_transfer error, out:%d reg_hcepcs:%x\n",__LINE__,aotg_readb(ep->reg_hcepcs));
				}
			}else{
				if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)) {
					//INFO( "ep-num%d,ep_mask:%x not busy\n",ep->epnum,ep->mask);
					//printk("<jjf_debug> start_async_transfer:%d reg_hcepcs:%x\n",__LINE__,aotg_readb(ep->reg_hcepcs));
					pio_irq_clear(ep->mask);
					completed = read_hcep_fifo(acthcd,ep,urb);
					ep->nextpid=USB_PID_IN;
					if(completed==0){
						/*need go on transfer*/
						aotg_writeb(0,ep->reg_hcepcs); /*arm this ep(sub buffer) for the IN transaction*/
						mb();
					}else{
						if (completed && !list_empty(&ep->hep->urb_list) ){
							DMSG_BULK("<am7x_hcd>@@@---<epout> process next urb\n");
							/*need go on transfer*/
							aotg_writeb(0,ep->reg_hcepcs); /*arm this ep(sub buffer) for the IN transaction*/
							mb();
						}
					}
				}else{
					printk("<jjf_debug> start_async_transfer, error in:%d reg_hcepcs:%x\n",__LINE__,aotg_readb(ep->reg_hcepcs));
				}
			}
			break;

		default:
			DMSG_BULK("<am7x_hcd>@@@---<ASYNC XFERS>odd bulk xfers stage\n");
			break;
		}
		break;
	case PIPE_CONTROL:
		DMSG_XFERS("<am7x_hcd>@@@---<ASYNC XFERS>CTRL XFERS-->\n");
		switch(ep->nextpid) {
		case USB_PID_SETUP:
			DMSG_CTRL("<am7x_hcd>@@@---USB_PID_SETUP\n");
			handle_setup_packet(ep,urb);
			break;
		case USB_PID_IN:
			DMSG_CTRL("<am7x_hcd>@@@---USB_PID_IN\n");
			ep->length = min((int)ep->maxpacket,rw_bytes);
			aotg_writeb(0,HCIN0BC);/*arm HC IN EP0 for the next IN transfer*/
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL> IN waiting to receive %dB data\n",ep->length);
			break;
		case USB_PID_OUT:
			DMSG_CTRL("<am7x_hcd>@@@---USB_PID_OUT\n");
			write_hcep0_fifo(ep,urb);
			break;
		case USB_PID_ACK:
			DMSG_CTRL("<am7x_hcd>@@@---USB_PIN_ACK\n");
			break;
		default:
			DMSG_CTRL("<am7x_hcd>@@@---<ASYNC XFERS>odd control xfers stage\n");
		}
		break;
	}

#endif
	return completed;
}



#if 1

static int start_async_transfer(struct aotg_hcd *acthcd,struct aotg_hcep *aim)
{
	struct aotg_hcep *ep;
	struct urb *urb;
        int  completed = 0;
	 int 	rw_bytes;

	if ((aim != NULL)  && (aim != acthcd->next_async)) {
		ep = aim;

		//printk("<jjf_debug> start_async_transfer ,ep exist:%x\n",ep);
		
		goto start_work;
	}
	
	if(acthcd->next_async){
		//printk("<jjf_debug> start_async_transfer,exist async waiting for transfer\n");
		ep = acthcd->next_async;
//printk("debug >>>>>>>>>>>>>><ASYNC XFERS> exist async ep%d waiting for transfer\n",__func__,__LINE__,ep->epnum);
		DMSG_XFERS("<am7x_hcd>@@@---<ASYNC XFERS> exist async ep%d waiting for transfer\n",ep->epnum);
	}else{
		if(!list_empty(&acthcd->async)) {
//printk("debug %s %d>>>>>>>>>>>>>>>>>>>>>>\n",__func__,__LINE__);
			//printk("<jjf_debug> start_async_transfer, find an ep for transfer \n");
			ep = container_of(acthcd->async.next,struct aotg_hcep,schedule);
			DMSG_XFERS("<am7x_hcd>@@@---<ASYNC XFERS> get a async ep index %d from async list\n",ep->index);
		}else {
			ERR("<am7x_hcd>@@@---<ASYNC XFERS> no async ep wait\n");
			return 0;
		}
	}

	//printk("%s %d %x\n",__func__,__LINE__,ep->index);
	//printk("%s %d %x\n",__func__,__LINE__,ep->reg_hcepctrl);
	//printk("%s %d %x\n",__func__,__LINE__,ep->epnum);
	//printk("%s %d %x\n",__func__,__LINE__,ep->mask);
	//printk("%s %d %x\n",__func__,__LINE__,ep);
	
	acthcd->ep_list[ep->index]=ep;

	if(ep->index !=0){
		aotg_writeb(ep->epnum, ep->reg_hcepctrl);
	}
	
	
	
	if (ep->schedule.next == &acthcd->async)
		acthcd->next_async = NULL;
	else
		acthcd->next_async = container_of(ep->schedule.next,struct aotg_hcep, schedule);
	//printk("<jjf_debug> start_async_transfer, next ep addr:%x \n",acthcd->next_async);
	
	

	
start_work:	
	if (unlikely(list_empty(&ep->hep->urb_list))) {
		//printk("<jjf_debug> start_async_transfer %x,urb list empty\n",ep->mask);
		return 0;
	}else{
		//printk("<jjf_debug> start_async_transfer:%x,urb list not empty\n",&ep->hep->urb_list);
	}
	
	/*start transfer,and at this point, next_async has a valid ep*/
	urb = container_of(ep->hep->urb_list.next,struct urb,urb_list);
	rw_bytes = urb->transfer_buffer_length - urb->actual_length;
	BUG_ON(rw_bytes < 0);

	//printk("<jjf_debug> start_async_transfer, ep func addr:%x \n",usb_pipedevice(urb->pipe));
	aotg_writeb(usb_pipedevice(urb->pipe),FNADDR);// FNADDR is shared by all endpoint
#if 0
	printk("<trans_urb_%d> ",urb->num);
	switch(ep->type){
		case PIPE_CONTROL:
			printk("type:PIPE_CONTROL ");
			break;
		case PIPE_BULK:
			printk("type:PIPE_BULK ");
			break;
		case PIPE_INTERRUPT:
			printk("type:PIPE_INTERRUPT ");
			break;
		case PIPE_ISOCHRONOUS:
			printk("type:PIPE_ISOCHRONOUS ");
			break;
		default:
			printk("type:PIPE_UNKONWN ");
			break;
	}

	
	switch(ep->nextpid){
		case USB_PID_IN:
			printk("stage:USB_PID_IN ");
			break;
		case USB_PID_OUT:
			printk("stage:USB_PID_OUT ");
			break;
		case USB_PID_ACK:
			printk("stage:USB_PID_ACK ");
			break;
		case USB_PID_SETUP:
			printk("stage:USB_PID_SETUP ");
			break;
		default:
			printk("stage:USB_PID_UNKONWN ");
			break;
	}
	printk("\n");

#endif
	

	//printk("%s %d ep->type:%x pid:%x\n",__func__,__LINE__,ep->type,ep->nextpid);

	switch(ep->type){
	case PIPE_BULK:
		DMSG_BULK("<am7x_hcd>@@@---<ASYNC XFERS>BULK XFERS**>\n");		
		switch(ep->nextpid) {
		case USB_PID_IN:
			aotg_writeb(0,ep->reg_hcepcs); /*arm this ep(sub buffer) for the IN transaction*/
			mb();

			//printk("need_packet:len:%d tans_len:%d ep_len:%d\n",urb->transfer_buffer_length,\

			//urb->actual_length,ep->maxpacket);
			break;
		case USB_PID_OUT:
			if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)){
				write_packet(ep,urb,ep->maxpacket);
			}else{
				printk("<jjf_debug> start_async_transfer ,Some error happen in bulk out!\n");
			}
			break;
	    case USB_PID_ACK:
			if(ep->mask & USB_HCD_OUT_MASK){
				if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)) {
					//INFO( "ep-num%d,ep_mask:%x not busy\n",ep->epnum,ep->mask);

					//printk("<jjf_debug> start_async_transfer:%d reg_hcepcs:%x\n",__LINE__,aotg_readb(ep->reg_hcepcs));

					pio_irq_clear(ep->mask);
					completed = write_hcep_fifo(acthcd,ep,urb);
					ep->nextpid=USB_PID_OUT;
					if (completed && !list_empty(&ep->hep->urb_list) ){
						//printk("<jjf_debug> start_async_transfer:complete:%d %d\n",completed);
						write_packet(ep,urb,ep->maxpacket);
					}else if(completed == 0){
						//printk("<jjf_debug> start_async_transfer:go to transfer complete:%d %d\n",completed);
						write_packet(ep,urb,ep->maxpacket);
					}
				}else{
					printk("<jjf_debug> start_async_transfer error, out:%d reg_hcepcs:%x\n",__LINE__,aotg_readb(ep->reg_hcepcs));
				}
			}else{
				if(!(aotg_readb(ep->reg_hcepcs) & EPCS_BUSY)) {
					//INFO( "ep-num%d,ep_mask:%x not busy\n",ep->epnum,ep->mask);
					//printk("<jjf_debug> start_async_transfer:%d reg_hcepcs:%x\n",__LINE__,aotg_readb(ep->reg_hcepcs));
					pio_irq_clear(ep->mask);
					completed = read_hcep_fifo(acthcd,ep,urb);
					ep->nextpid=USB_PID_IN;
					if(completed==0){
						/*need go on transfer*/
						aotg_writeb(0,ep->reg_hcepcs); /*arm this ep(sub buffer) for the IN transaction*/
						mb();
					}else{
						if (completed && !list_empty(&ep->hep->urb_list) ){
							DMSG_BULK("<am7x_hcd>@@@---<epout> process next urb\n");
							/*need go on transfer*/
							aotg_writeb(0,ep->reg_hcepcs); /*arm this ep(sub buffer) for the IN transaction*/
							mb();
						}
					}
				}else{
					printk("<jjf_debug> start_async_transfer, error in:%d reg_hcepcs:%x\n",__LINE__,aotg_readb(ep->reg_hcepcs));
				}
			}
			break;

		default:
			DMSG_BULK("<am7x_hcd>@@@---<ASYNC XFERS>odd bulk xfers stage\n");
			break;
		}
		break;
	case PIPE_CONTROL:
		DMSG_XFERS("<am7x_hcd>@@@---<ASYNC XFERS>CTRL XFERS-->\n");
		switch(ep->nextpid) {
		case USB_PID_SETUP:
			DMSG_CTRL("<am7x_hcd>@@@---USB_PID_SETUP\n");
			handle_setup_packet(ep,urb);
			break;
		case USB_PID_IN:
			DMSG_CTRL("<am7x_hcd>@@@---USB_PID_IN\n");
			ep->length = min((int)ep->maxpacket,rw_bytes);
			aotg_writeb(0,HCIN0BC);/*arm HC IN EP0 for the next IN transfer*/
			DMSG_CTRL("<am7x_hcd>@@@---<CTRL> IN waiting to receive %dB data\n",ep->length);
			break;
		case USB_PID_OUT:
			DMSG_CTRL("<am7x_hcd>@@@---USB_PID_OUT\n");
			write_hcep0_fifo(ep,urb);
			break;
		case USB_PID_ACK:
			DMSG_CTRL("<am7x_hcd>@@@---USB_PIN_ACK\n");
			break;
		default:
			DMSG_CTRL("<am7x_hcd>@@@---<ASYNC XFERS>odd control xfers stage\n");
		}
		break;
	}
	return completed;
}

#endif


static void start_schedule(struct aotg_hcd *acthcd,struct aotg_hcep* ep )
{
	
	unsigned long flags;
	struct list_head *iter;
	struct list_head *head;
	struct urb *urb;
	int i;
	if (!HC_IS_RUNNING(aotg_to_hcd(acthcd)->state) 
		/*||(acthcd->transceiver->port_status & USB_PORT_STAT_SUSPEND)*/) {
		DMSG_XFERS("<am7x_hcd>@@@---stop schedule\n");
		return;
	}



	//printk("%s %d \n",__func__,__LINE__);
	spin_lock_irqsave(&acthcd->lock,flags);		

	if(list_empty(&am7x_hcd_event_list)){
		goto start_schedule_END;
	}
	else{
		list_del(am7x_hcd_event_list.next);
	}


	

	
	
	

	if(is_ep_busy()){
		printk("<jjf_debug> start_schedule error,The Controller is busy!\n");
	}


	
#if 0	
	//find the async endpoint to transfer
	if(wait_bulk_out_transfer_end){
		if(acthcd->cur_ep != NULL){
			for(i=0;i<1000;i++){
				if((aotg_readb(acthcd->cur_ep->reg_hcepcs)&2) == 0){
					/*out transfer end*/
					wait_bulk_out_transfer_end=0;
					handle_hcep(acthcd,acthcd->cur_ep->mask,acthcd->cur_ep->index);
					break;
				}else{
					printk("out trans is busy:%08x %08x %08x %08x",acthcd->cur_ep->reg_hcepbc,acthcd->cur_ep->reg_hcepcs,aotg_readw(acthcd->cur_ep->reg_hcepbc),aotg_readb(acthcd->cur_ep->reg_hcepcs));
					goto start_schedule_END;
				}
			}
		}else{
			printk("Error State:wait_bulk_out_transfer_end:%x ep->index:%x\n",wait_bulk_out_transfer_end,acthcd->cur_ep->index);
		}
	}
	
#endif
	if(ep_has_exist_async_list(acthcd->cur_ep,&acthcd->async)){
		ep = acthcd->cur_ep;
		if(ep->inuse == 0 ){
			printk("<jjf_debug> start_schedule error,ep is disable %x %d\n",ep,ep->mask);
			head = &acthcd->async;
		}else if(acthcd->cur_ep->type == PIPE_CONTROL){
			if(ep->nextpid != USB_PID_SETUP){
				goto start_schedule_GO;
			}else{
				head = &acthcd->cur_ep->schedule;		
			}
		}else{
			head = &acthcd->cur_ep->schedule;	
		}
	}else{
		head = &acthcd->async;
	}
	if(list_empty(&acthcd->async)){
		sof_disable();
		goto start_schedule_END;
	}else{
		list_for_each(iter, head){
			if(iter != (&acthcd->async)){
				acthcd->cur_ep = container_of(iter,struct aotg_hcep,schedule);
				if(acthcd->cur_ep->inuse == 0){
					continue;
				}

				if(acthcd->cur_ep->ep_schedule){
					continue;
				}else{
					break;
				}
			}
		}

		if(acthcd->cur_ep->ep_schedule || (acthcd->cur_ep->inuse==0)){
			goto start_schedule_END;
		}
	}







	
start_schedule_GO:


	acthcd->ep_list[acthcd->cur_ep->index]=acthcd->cur_ep;

	urb= container_of(acthcd->cur_ep->hep->urb_list.next,struct urb,urb_list);
	


	set_ep_busy();
	start_transfer(acthcd,acthcd->cur_ep);

start_schedule_END:
	//printk("%s %d \n",__func__,__LINE__);
	spin_unlock_irqrestore(&acthcd->lock,flags);
	
}


#if 0
/*-------------------------------------------------------------------------*/
/* usb 1.1 says max 90% of a frame is available for periodic transfers.
 * this driver doesn't promise that much since it's got to handle an
 * IRQ per packet; irq handling latencies also use up that time.
 *
 * NOTE:  the periodic schedule is a sparse tree, with the load for
 * each branch minimized.  see fig 3.5 in the OHCI spec for example.
 */
 
static int balance(struct aotg_hcd *acthcd, u16 period, u16 load)
{
	int	i, branch = -ENOSPC;

	/* search for the least loaded schedule branch of that period
	 * which has enough bandwidth left unreserved.
	 */
	for (i = 0; i < period ; i++) {
		if (branch < 0 || acthcd->load[branch] > acthcd->load[i]) {
			int j;
			for (j = i; j < PERIODIC_SIZE; j += period) {
				if ((acthcd->load[j] + load) > MAX_PERIODIC_LOAD)
					break;
			}
			
			if (j < PERIODIC_SIZE)
				continue;
			
			branch = i;
		}
	}
	
	return branch;
}
#endif
#define cond_printf(condition)   do{ if (condition) DMSG_HCD("%s:%d\n",__func__,__LINE__);}while(0) 

static void dump_periodic_sched(struct aotg_hcd	*acthcd)
{
	int i;
	struct aotg_hcep	*ep = NULL;

	DMSG_HCD("<am7x_hcd>@@@---periodic sz= %d,count=%d,curr=%d\n", PERIODIC_SIZE,acthcd->periodic_count,acthcd->frame);
	for (i = 0; i < PERIODIC_SIZE; i++) {
		ep = acthcd->periodic[i];
		if (!ep)
			continue;
		
		DMSG_HCD("<am7x_hcd>@@@---%2d [%3d]:\n", i, acthcd->load[i]);
		do {
			DMSG_HCD("<am7x_hcd>@@@---qh%d/%p (%sdev%d ep%d%s max %d) "
					"err %d\n",
				ep->period, ep,
				(ep->udev->speed == USB_SPEED_FULL)
					? "" : "ls ",
				ep->udev->devnum, ep->epnum,
				(ep->epnum == 0) ? ""
					: ((ep->nextpid == USB_PID_IN)
						? "in"
						: "out"),
				ep->maxpacket, ep->error_count);
			ep = ep->next;
		} while (ep);
	}
}


static int inline urb_in_hc_ep_queue(struct usb_host_endpoint *hep,
	struct urb		*urb)
{
	struct urb *curr;
	list_for_each_entry (curr, &hep->urb_list, urb_list) {
		if (curr == urb)
			return 1;
	}
	return 0;
}


static int inline ep_has_exist_async_list(struct aotg_hcep	*hep,struct list_head *head)
{	
	struct aotg_hcep *curr;
	struct list_head *iter;
	list_for_each(iter, head) {
		curr = container_of(iter,struct aotg_hcep,schedule);
		if (curr == hep)
			return 1;
	}
	return 0;
}

static int inline ep_in_hcep_map_list(struct aotg_hcep	*hep){
	unsigned char i;
	
	for(i = 0;i < ARRAY_SIZE(hcep_map);i++){
		if(hep == &hcep_map[i])
			return 1;
	}
	return 0;
}


static int aotg_hcd_urb_enqueue (struct usb_hcd *hcd,
				struct urb *urb,
				gfp_t mem_flags)
{
	struct aotg_hcd	*acthcd = hcd_to_aotg(hcd);
	struct usb_device	*udev = urb->dev;
	unsigned int		pipe = urb->pipe;
	int			is_out = !usb_pipein(pipe);
	int			type = usb_pipetype(pipe);
	int			epnum = usb_pipeendpoint(pipe);	
	int			retval = 0;
	unsigned long flags;	
	struct aotg_hcep	*ep = NULL;
	struct aotg_hcep	*ep_new = NULL;
	struct usb_host_endpoint *hep=urb->ep;


	
#ifdef DEBUG_TXRX
	if((type == PIPE_BULK)&&( urb->transfer_buffer_length < 2500UL)/*&& is_out*/){
		printk("<am7x_hcd>@@@---enq:t=%d num=%d is_out=%d urb=%x,l=%d,al=%d\n", 
			type,
			epnum,
			is_out,
			(unsigned long)urb,
			urb->transfer_buffer_length,
			urb->actual_length);
	}
#endif	

	spin_lock_irqsave(&acthcd->lock,flags);
	retval = usb_hcd_link_urb_to_ep(hcd, urb);
	if (retval){
		ERR("<am7x_hcd>@@@---hcd_link_urb_to_ep failed\n");
		goto enqueue_fail;
	}
	urb->num = urb_index;
	 
	urb_index++;


	//printk("<enqueue_start_%d_%d_%d_%d_%d>\n",urb->num,usb_pipedevice(urb->pipe),usb_pipeendpoint(pipe),type,is_out);

	
/*
	if ( !urb_in_hc_ep_queue(hep,urb) ){
		ERR("finished before enqueue,urb=%p use_count=%d\n", \
			urb,atomic_read(&(urb->use_count)));
		retval = -EFAULT;
		goto error;
	}
*/	
	/* don't submit to a dead or disabled port */
	if(!HC_IS_RUNNING(hcd->state)) {
		ERR("<am7x_hcd>@@@---<QUEUE> port is dead or disable\n");
		retval = -ENODEV;
		goto error;
	}



	/*only support int transfer and control transfer*/
	if(type == PIPE_ISOCHRONOUS){
		printk("%s %d \n",__func__,__LINE__);
		finish_unspport_urb_request(acthcd,urb,-EPIPE);
		retval = ENODEV;
		goto enqueue_fail;
	}
	

	
	
	if (hep->hcpriv) {/*ep has been initialized*/
		DMSG_XFERS("<am7x_hcd>@@@---<QUEUE> ep has been initialized\n");
		ep = hep->hcpriv;	
		//printk("<jjf_debug> aotg_hcd_urb_enqueue ,ep has been initialized %x\n");
	}  else {/*not init*/
	    DMSG_XFERS("<am7x_hcd>@@@---<QUEUE> init ep\n");
		
		ep = find_an_idle_hcep(type,is_out);
		if(!ep){
			ERR( "<am7x_hcd>@@@---cannot find idle ep,type:%x,is_out:%d,epnum:%d\n",
				type,is_out,epnum);
			goto error;
		}

		printk("<find_an_idle_hcep:ep:%x type:%d mask:%x>\n",ep,ep->type,ep->mask);
		
		
		if(ep->type == PIPE_CONTROL){
			if(ep->inuse){
				ep_new = find_an_idle_hcep_reserve();	
				memcpy(ep_new,ep,sizeof(struct aotg_hcep));
				ep = ep_new;
			}
			ep->inuse = 1;
			
		}else if(ep == NULL){
			printk("\n******ERROR can not find an ep!****\n\n");
			ep_new = find_an_idle_hcep_reserve();	
			ep = find_an_idle_hcep_next(type,is_out);
			if(ep == NULL){
				printk("find_an_idle_hcep_next this func have some error!\n");
			}else{
				memcpy(ep_new,ep,sizeof(struct aotg_hcep));
				ep = ep_new;
			}
			ep->inuse = 1;
			
		}else{
			
		}
		printk("<find_an_idle_hcep1:ep:%x type:%d mask:%x>\n",ep,ep->type,ep->mask);


		//printk("<jjf_debug> aotg_hcd_urb_enqueue , ep index:%d\n",ep->index);
		INIT_LIST_HEAD(&ep->schedule);
		ep->next = NULL;
		ep->udev = udev;
		ep->epnum = epnum;
		ep->maxpacket = usb_maxpacket(udev, urb->pipe, is_out);
		ep->ep_schedule=0;
		usb_settoggle(udev, epnum, is_out, 0);

		if (type == PIPE_CONTROL)
			ep->nextpid = USB_PID_SETUP;
		else if (is_out)
			ep->nextpid = USB_PID_OUT;
		else
			ep->nextpid = USB_PID_IN;

		switch(type) {
			case PIPE_CONTROL:
				if(acthcd->ep_ref_cnt[0] == 0) {
					ep->index = 0;
					ep->mask = 0;
					acthcd->ep_list[0]=ep;

					//acthcd->ep_list[0] = ep;/*hook ep0 to ep_list*/	
					DMSG_CTRL("<am7x_hcd>@@@---<QUEUE> initialize ep0 successfully,the index is %d,address %02x software ep %d hardware ep %02x\n"
							,ep->index,aotg_readb(FNADDR),ep->epnum,aotg_readb(HCEP0CTRL));

					/*enable ep0 irq*/
					//aotg_setbits(EP0_IN_IEN,HCIN04IEN);
					//aotg_setbits(EP0_OUT_IEN,HCOUT04IEN);
				}
				break;
			case PIPE_BULK:
			case PIPE_INTERRUPT:
				if(acthcd->ep_ref_cnt[ep->index] == 0){
					retval = aotg_hcep_config(acthcd,ep,ep->buftype,is_out);
					if(retval < 0) {
						ERR( "<am7x_hcd>@@@---<QUEUE>cannot config async ep\n");
						goto error;
					}
				}else{
					printk("<jjf_debug> error aotg_hcd_urb_enqueue , ep index %d exist\n",ep->index);
				}
				break;
			default:
				printk("not support urb type:%x\n",type);
				break;
			
		}
		acthcd->ep_ref_cnt[ep->index] += 1;	
		/*indicate this ep has been allocated already*/
		ep->hep = hep;
		hep->hcpriv = ep;	
	}


	//printk("<enqueue_urb_%d> ep num:%x mask:%x\n",urb->num,ep->epnum,ep->mask);

	//int i;
	switch(type) {
		case PIPE_CONTROL:	
			/*enable ep0 irq*/
			aotg_setbits(EP0_IN_IEN,HCIN04IEN);
			aotg_setbits(EP0_OUT_IEN,HCOUT04IEN);
			break;
		case PIPE_BULK:
		case PIPE_INTERRUPT:
			//printk("enqueue int urb! %d %x\n",urb->num,ep);
			pio_irq_enable(ep->mask);
			break;
		default:
			printk("%s %d not support ep type:%x\n",__func__,__LINE__,type);
			break;
	}

	
	retval = 0;
	urb->hcpriv = hep;



	/* in case of unlink-during-submit */
	if (urb->status != -EINPROGRESS) {
	printk("%s %d \n",__func__,__LINE__);
		ERR("<am7x_hcd>@@@---<ENQUEUE> urb unlink\n");
		finish_request(acthcd, ep, urb);
		retval = 0;
		goto error;
	}

	sof_enable();
	switch(type){
		case PIPE_CONTROL:
			submit_ep(ep);
			break;
		case PIPE_BULK:
		case PIPE_INTERRUPT:
			submit_ep(ep);
			break;
		default :
			printk("Not support ep type!\n");
			goto error;
			break;
	}

error:
	if(retval){
		ERR("<am7x_hcd>@@@---<ENQUEUE>unlink urb from ep\n");
		usb_hcd_unlink_urb_from_ep(hcd, urb);
	}
enqueue_fail:
	spin_unlock_irqrestore(&acthcd->lock,flags);
	return retval;
	
}

static int aotg_hcd_urb_dequeue(
	struct usb_hcd *hcd, struct urb *urb,int status
) {
	

	struct aotg_hcd *acthcd = hcd_to_aotg(hcd);
	struct usb_host_endpoint *hep;
	struct aotg_hcep	*ep;
	unsigned long flags;
	int	retval = 0;
	struct urb * iter;
	struct list_head * pos;
	
	spin_lock_irqsave(&acthcd->lock, flags);
	hep = urb->hcpriv;

	printk("%s %d %x \n",__func__,__LINE__,hep);

	if (!hep)
		goto dequeue_fail;

	ep = hep->hcpriv;
	
	printk("%s %d %x \n",__func__,__LINE__,hep->hcpriv);


	DMSG_XFERS("<am7x_hcd>@@@---%s:%d ep->index=%d\n",__func__,__LINE__,ep->index);
	if (ep) {
			//while(!list_empty(&(hep->urb_list)) ){//&& (!list_empty(&(iter->urb_list)))
			while(!list_empty(&(hep->urb_list)) ){//&& (!list_empty(&(iter->urb_list)))
					iter = container_of(hep->urb_list.next,struct urb,urb_list);
					iter->status=status;
					//printk("<am7x_hcd> aotg_hcd_urb_dequeue the urb is remove form the ep%d urb addr:%x num:%d %x %x %x!\n",ep->index,iter,iter->num,&(iter->urb_list),&(hep->urb_list),iter->urb_list.next);

					printk("<aotg_hcd_urb_dequeue_%d_%d_%d_%d_%d>\n",iter->num,usb_pipedevice(iter->pipe),usb_pipeendpoint(iter->pipe),usb_pipetype(iter->pipe),usb_pipeout(iter->pipe));
					finish_request(acthcd, ep, iter);// unlink all urb from ep,avoid the urb is useing by hardware ,the next urb is use error!
			}

			
		
		



#if 0
		if(urb){
			urb->status=status;
			if (ep->hep->urb_list.next != &urb->urb_list){
				finish_request(acthcd,ep,urb);
				printk("<am7x_hcd> aotg_hcd_urb_dequeue the urb is remove form the ep%d!\n",ep->index);
			}
			else{
				/*do nothing ,because ,the ep maybe use the urb*/
				urb->status = -EPIPE;
				finish_request(acthcd, ep, urb);
				printk("<am7x_hcd> aotg_hcd_urb_dequeue the urb is using by ep! %x %d %d\n",urb,urb->num,urb->status);
			}
		}
#endif
				
		
#if 0
		if (ep->hep->urb_list.next != &urb->urb_list) {
			printk("<am7x_hcd>@@@---<DEQUEUE> this urb belongs to ep%d is not active",ep->epnum);
		}else if(ep != acthcd->next_async){
			printk("<am7x_hcd>@@@---<DEQUEUE> this ep%d is not active",ep->epnum);
		}
		else {
            urb->status = -EPIPE;//for prevent hcd fatal error when irq lost
			printk("<am7x_hcd>@@@---<DEQUEUE> urb is active, wait for irq\n");
		}

		if (urb){
			INFO("<am7x_hcd>@@@---cancel this urb,ep->num=%d\n",ep->epnum);
			//finish_request(acthcd, ep, urb);
		}
#endif
	}else
		retval = -EINVAL;

dequeue_fail:
	spin_unlock_irqrestore(&acthcd->lock, flags);	
	return retval;
}


void unlink_urb_from_ep(struct usb_hcd *hcd,struct usb_host_endpoint *hep){
	struct list_head *iter;
	struct urb *urb;
	struct aotg_hcd *acthcd = hcd_to_aotg(hcd);
	struct aotg_hcep *ep = hep->hcpriv;

	if(list_empty(&(hep->urb_list))){
		printk("unlink_urb_from_ep ,parametor urb_list is empty!\n");
		return;
	}

	if(ep == NULL){
		printk("unlink_urb_from_ep ,parametor ep is NULL!\n");
		return;
	}


	while(!list_empty(&(hep->urb_list)) ){//&& (!list_empty(&(iter->urb_list)))
		urb = container_of(hep->urb_list.next,struct urb,urb_list);
		urb->status=-EPIPE;
		printk("<am7x_hcd> aotg_hcd_urb_dequeue the urb is remove form the ep%d %x %x %x!\n",ep->index,&(urb->urb_list),&(hep->urb_list),urb->urb_list.next);
		finish_request(acthcd, ep, urb);// unlink all urb from ep,avoid the urb is useing by hardware ,the next urb is use error!
	}

	
}


static void aotg_hcd_endpoint_disable(
	struct usb_hcd *hcd,
	struct usb_host_endpoint *hep
){
	printk("%s %d \n",__func__,__LINE__);

	int i,index;
	unsigned long flags;
	struct aotg_hcd *acthcd = hcd_to_aotg(hcd);
	struct aotg_hcep *ep = hep->hcpriv;

	

	printk("<am7x_hcd>@@@---disable ep:%d\n",hep->desc.bEndpointAddress);
	if (!ep)
		return;
	printk("disable:ep infor:%x %x %x %x %x %x\n",ep,ep->mask,ep->index,ep->buftype,ep->reg_hcepaddr,ep->reg_hcepcon);

	/* assume we'd just wait for the irq */
	for (i = 0; i < 100 && !list_empty(&hep->urb_list); i++)
		mdelay(5);
	
	if (!list_empty(&hep->urb_list)){
		/*fix me: will close all urb */
		WARN("<am7x_hcd>@@@---ep %p not empty?\n", ep);
	}


	spin_lock_irqsave(&acthcd->lock, flags);

	unlink_urb_from_ep(hcd,hep);
	

	
	

	index=ep->index;

	if(acthcd->ep_ref_cnt[index]==0){
		printk("error:The endpoint%d has been disabled ! \n",index);
	}else{
		acthcd->ep_ref_cnt[index] -= 1;
	}
	
	printk("%s %d \n",__func__,__LINE__);
	
	if(ep != acthcd->ep_list[index]){
		if(acthcd->ep_list[index] != NULL){
			printk("<am7x_hcd>@@@---<EP DISABLE> phy ep list ep%d index %d %x \n",acthcd->ep_list[index]->epnum,index,acthcd->ep_list[index]);
		}else{
			printk("<am7x_hcd>@@@---<EP DISABLE> phy ep list ep%d index %d %x \n",ep->epnum,index,ep);
		}
		
		//if(acthcd->ep_list[index]){
			//acthcd->ep_list[index]->inuse=0;
			
			//acthcd->ep_list[index] = NULL; /*remove this ep from global ep list*/
		//}
	}else{
		/*目前暂时这么做 ，暂时ep num的数量是够的 ，如果ep num的数量不够此处会造成错误*/
		//printk("%s %d \n",__func__,__LINE__);
		//acthcd->ep_list[index]=NULL; // 可能会有reserve ep 使用它
	}
	
	printk("<am7x_hcd>@@@---<EP DISABLE> remove ep%d index %d %x %x from global ep list\n",ep->epnum,index,ep,ep->type);
	ep->inuse = 0;
	hep->hcpriv = NULL;

#ifdef CONFIG_USB_AM7X_HOST_ONLY_0
    /*
    * reset the endpoint.
    */
    if(ep->epnum !=0){
        pio_irq_disable(ep->mask);
        invalidate_ep(ep);
        printk("<am7x_hcd>@@@===== reset endpoint,epnum=%d,mask=0x%x\n",ep->epnum,ep->mask);
        aotg_hcep_reset(ep->mask,ENDPRST_FIFOREST |ENDPRST_TOGRST);
    }
#endif
	spin_unlock_irqrestore(&acthcd->lock, flags);	

}

static int aotg_hcd_get_frame(struct usb_hcd *hcd)
{
	struct aotg_hcd *acthcd = hcd_to_aotg(hcd);
	u16 frmnum;

	frmnum = aotg_readl(FRMNRL);
	if(acthcd->speed == HIGHSPEED) {
		DMSG_HCD("<am7x_hcd>@@@---<GET FRAME NUM> frame num(HS) is %d\n",frmnum);
		return frmnum;
	}else {
		DMSG_HCD("#<am7x_hcd># <GET FRAME NUM>frame num(FS/LS) is %d\n",frmnum>>3);
		return (frmnum >> 3);
	}
}

static void port_suspend(struct aotg_hcd *acthcd)
{
	if(acthcd->transceiver->state == OTG_STATE_A_HOST) {
		/*only a host */
		DMSG_HCD("<am7x_hcd>@@@---<PORT_SUSPEND> a host suspend port\n");
		acthcd->transceiver->state = OTG_STATE_A_SUSPEND;
		aotg_clearbits(OTGCTRL_BUSREQ,OTGCTRL);
	}
}

static void port_resume(struct aotg_hcd *acthcd)
{
	if(acthcd->transceiver->state == OTG_STATE_A_SUSPEND) {
		/*only a host */
		DMSG_HCD("<am7x_hcd>@@@---<PORT_RESUME> a host resume port\n");
		acthcd->transceiver->state = OTG_STATE_A_HOST;
		aotg_setbits(OTGCTRL_BUSREQ,OTGCTRL);
	}else{
		DMSG_HCD("<am7x_hcd>@@@---<PORT_RESUME> not at a suspend state??\n");
	}
}


/*for we have not a hub truelly,we emulate a fake root hub for core*/
static void am_hub_descriptor(struct aotg_hcd  *acthcd,
				    struct usb_hub_descriptor *desc)
{
	desc->bDescriptorType = 0x29;
	desc->bHubContrCurrent = 0;
	desc->bNbrPorts = AM_MAX_ROOT_HUB;
	desc->bDescLength = 9;
	desc->bPwrOn2PwrGood = 0;
	desc->wHubCharacteristics = cpu_to_le16(0x0011);
	desc->bitmap[0] = ((1 << AM_MAX_ROOT_HUB) - 1) << 1;
	desc->bitmap[1] = ~0;

}
	
static void force_port_speed(int speed)
{
	//if (speed)
		//aotg_setbits(0x80,VDCTRL);
	//else
		//aotg_clearbits(0x80,VDCTRL);
}

static int aotg_hub_control (
	struct usb_hcd	*hcd,
	u16		typereq,
	u16		feature,
	u16 		wIndex, 
	char		*buf,
	u16		wLength) 
{
	struct aotg_hcd  *acthcd = hcd_to_aotg(hcd);
	int    retval = 0;
	unsigned long flags;
	
	if (hcd->state != HC_STATE_RUNNING) {
		DMSG_CTRL("<am7x_hcd>@@@---<PORT_CONTROL> hc state is not HC_STATE_RUNNING\n");
		return -ETIMEDOUT;
	}
	
	//printk(KERN_DEBUG "<am7x_hcd>@@@---%s:feature:%x,wIndex:%x\n",show_hub_req(typereq),feature,wIndex);
	
	switch(typereq) {
		case ClearHubFeature:
		case SetHubFeature:	
			switch (feature) {
			case C_HUB_OVER_CURRENT:
			case C_HUB_LOCAL_POWER:
				break;
			default:
				goto port_error;
			}
			break;		
		case GetHubDescriptor:
			am_hub_descriptor(acthcd,
					(struct usb_hub_descriptor *)buf);
			break;	
		case GetHubStatus:
			*(char*)buf = 0x00;
			break;
		case ClearPortFeature:
			DMSG_CTRL("<am7x_hcd>@@@---<PORT_CONTROL> clear port feature:%d\n",feature);
			if(!acthcd->running){
				DMSG_CTRL("<am7x_hcd>@@@---<PORT_CONTROL>before acthcd running\n");
				break;
			}
			
			switch (feature) {
				case USB_PORT_FEAT_ENABLE:	
					spin_lock_irqsave(&acthcd->lock,flags);
					acthcd->transceiver->port_status &= ~(USB_PORT_STAT_ENABLE
							| USB_PORT_STAT_LOW_SPEED
							| USB_PORT_STAT_HIGH_SPEED);
					acthcd->portstate= AOTG_PORT_DISABLE;
					//if(acthcd->transceiver->port_status & USB_PORT_STAT_POWER) 
					//	otg_set_power(acthcd->transceiver,0);	
					spin_unlock_irqrestore(&acthcd->lock,flags);
					break;
				case USB_PORT_FEAT_SUSPEND:
					port_resume(acthcd);	
					spin_lock_irqsave(&acthcd->lock,flags);
					acthcd->transceiver->port_status &= ~USB_PORT_STAT_SUSPEND;
					spin_unlock_irqrestore(&acthcd->lock,flags);	
					break;
					
				case USB_PORT_FEAT_POWER:
					spin_lock_irqsave(&acthcd->lock,flags);
					acthcd->transceiver->port_status = 0;
					acthcd->transceiver->port_change = 0;
					acthcd->portstate = AOTG_PORT_POWEROFF;
					otg_set_power(acthcd->transceiver,0);
					spin_unlock_irqrestore(&acthcd->lock,flags);
					break;
				case USB_PORT_FEAT_C_ENABLE:
					spin_lock_irqsave(&acthcd->lock,flags);
					acthcd->transceiver->port_change &= ~USB_PORT_STAT_C_ENABLE;
					spin_unlock_irqrestore(&acthcd->lock,flags);
					break;
				case USB_PORT_FEAT_C_SUSPEND:
					spin_lock_irqsave(&acthcd->lock,flags);
					acthcd->transceiver->port_change &= ~USB_PORT_STAT_C_SUSPEND;
					spin_unlock_irqrestore(&acthcd->lock,flags);
					break;
				case USB_PORT_FEAT_C_CONNECTION:
					DMSG_CTRL("<am7x_hcd>@@@---###clear port feature :connections!!!\n");
					spin_lock_irqsave(&acthcd->lock,flags);
					acthcd->transceiver->port_change &= ~USB_PORT_STAT_C_CONNECTION;
					spin_unlock_irqrestore(&acthcd->lock,flags);
					break;
				case USB_PORT_FEAT_C_RESET:
					spin_lock_irqsave(&acthcd->lock,flags);
					acthcd->transceiver->port_change &= ~USB_PORT_STAT_C_RESET;
					spin_unlock_irqrestore(&acthcd->lock,flags);
					break;
				case USB_PORT_FEAT_DISABLE_HS:
					spin_lock_irqsave(&acthcd->lock,flags);
					force_port_speed(1);
					spin_unlock_irqrestore(&acthcd->lock,flags);
					break;
				case USB_PORT_FEAT_C_OVER_CURRENT:
				default:
					goto port_error;
			}
			break;
		case  GetPortStatus:
			DMSG_CTRL("<am7x_hcd>@@@---<PORT_CONTROL> GetPortStatus\n");
			/*special process for plugout before poll rh status*/
			if(!acthcd->running){
				*(__le16 *) buf = (__le16)USB_PORT_STAT_POWER;
				*(__le16 *) (buf + 2) = 0;
				DMSG_CTRL("<am7x_hcd>@@@---<PORT_CONTROL>before  running\n");
				print_hex_dump(KERN_DEBUG, "hub staus: ", DUMP_PREFIX_OFFSET,16, 1, buf, 4, 1);
				break;
			}
			
			spin_lock_irqsave(&acthcd->lock, flags);
			*(__le16 *) buf = cpu_to_le16(acthcd->transceiver->port_status);
			*(__le16 *) (buf + 2) = cpu_to_le16(acthcd->transceiver->port_change);
			spin_unlock_irqrestore(&acthcd->lock, flags);
			break;
		case SetPortFeature:
			DMSG_CTRL("<am7x_hcd>@@@---<PORT_CONTROL> SetPortFeature:%d\n",feature);
			if(!acthcd->running){
				DMSG_CTRL("<am7x_hcd>@@@---<PORT_CONTROL>before acthcd running\n");
				break;
			}
			
			switch(feature) {
				case USB_PORT_FEAT_POWER:
					spin_lock_irqsave(&acthcd->lock, flags);
					if(acthcd->transceiver->port_status & USB_PORT_STAT_POWER){
						INFO("<am7x_hcd>@@@---port already power on\n");
						spin_unlock_irqrestore(&acthcd->lock, flags);
						break;
					}	
			              acthcd->transceiver->port_status |= USB_PORT_STAT_POWER;
					acthcd->portstate = AOTG_PORT_POWERED;
					otg_set_power(acthcd->transceiver,1);	
					spin_unlock_irqrestore(&acthcd->lock, flags);
					break;
					
				case USB_PORT_FEAT_RESET:
					//spin_lock_irqsave(&acthcd->lock, flags);
					acthcd->transceiver->port_status &= ~(USB_PORT_STAT_ENABLE
								| USB_PORT_STAT_LOW_SPEED
								| USB_PORT_STAT_HIGH_SPEED);
					acthcd->transceiver->port_status |= USB_PORT_STAT_RESET;
					acthcd->transceiver->port_change |= USB_PORT_STAT_C_RESET;
					acthcd->portstate = AOTG_PORT_RESET;

               #ifdef CONFIG_USB_AM7X_HOST_ONLY_0
                    /** reset fifo_addr for the next use */
                    acthcd->fifo_addr = HC_FIFO_START_ADDR;
               #endif

					aotg_clear_pend(aotg_readb(USBIRQ),USBIRQ);
					port_reset();	
					aotg_setbits(USBIEN_URES,USBIEN); /*enable reset irq*/
				
					/* if it's already enabled, disable */
				//	spin_unlock_irqrestore(&acthcd->lock, flags);
					break;
			
				case USB_PORT_FEAT_SUSPEND:
					spin_lock_irqsave(&acthcd->lock, flags);
					acthcd->transceiver->port_status |= USB_PORT_STAT_SUSPEND;
					acthcd->portstate= AOTG_PORT_SUSPEND;
					DMSG_CTRL("<am7x_hcd>@@@---<PORT_CONTROL>set port feature:feature suspend acthcd->port = %x\n",acthcd->transceiver->port_status);
					port_suspend(acthcd);
					spin_unlock_irqrestore(&acthcd->lock, flags);
					break;		
				case USB_PORT_FEAT_DISABLE_HS:
					spin_lock_irqsave(&acthcd->lock,flags);
					force_port_speed(1);
					spin_unlock_irqrestore(&acthcd->lock,flags);
					break;
				default:
					ERR("<am7x_hcd>@@@---<PORT_CONTROL>feature:%d",feature);
				//	if(acthcd->transceiver->port_status  &  USB_PORT_STAT_POWER)
				//		acthcd->transceiver->port_status |= (1<<feature);
					break;
			}
			break;
                default:
port_error:
			retval = -EPIPE;
			ERR("<am7x_hcd>@@@---<PORT_CONTROL> hub control error\n");
			break;		
	}
	
	return retval;
}


static int aotg_hub_status_data(struct usb_hcd *hcd, char *buf)
{
	unsigned long flags;
	u16 portchange;
	struct aotg_hcd  *acthcd = hcd_to_aotg(hcd);
	if (!HC_IS_RUNNING(hcd->state))
		return -ESHUTDOWN;

	buf[0] = 0;
	
	if(!acthcd->running){
		DMSG_CTRL("<am7x_hcd>@@@---poll port status before acthcd running\n");
		return 0;
	}
	
	spin_lock_irqsave(&acthcd->lock,flags);
	portchange = acthcd->transceiver->port_change;
	
	#ifndef CONFIG_USB_AM7X_HOST_ONLY_0
	//#if 1  //bluse add for test otg status jump to a_wait_bconn
	//printk("--------portchange:0x%02x statue:%02x\n",portchange,get_otg_state());
	if(acthcd->inserted==0){
		udelay(50000);
		if(get_otg_state()==A_HOST){
			acthcd->inserted=1;
			acthcd->transceiver->port_change = 0x0;
			portchange=0x0;
		}
	}
	#endif
	spin_unlock_irqrestore(&acthcd->lock,flags);
	
	/*report port status .bit0:roothub bit1:port*/
	/*special process for the first plugout*/
	if(portchange & USB_PORT_STAT_C_CONNECTION){
		buf[0] |= 1<<1;	
		DMSG_CTRL("<am7x_hcd>@@@---hub port status may change,%d\n",buf[0]);
		return 1;
	}
	
	return 0;
}

static int aotg_hcd_ioctl(struct  usb_hcd *hcd,int cmd,int arg)
{
	int result = 0;
	int epnums;
	int dma_weight;
	struct aotg_hcd * acthcd;
	
	acthcd = hcd_to_aotg(hcd);
	switch(cmd){
	case HCD_CMD_REINIT_BUF:
		DMSG_HCD("<am7x_hcd>@@@---reinit ep buffer\n");
		epnums = arg;
		result = 0;
		modify_hcep_buftype(epnums);
		break;
	case HCD_CMD_SET_DMA:
		
		INFO("<am7x_hcd>@@@---<AOTG_IOCTL>set dma:%d\n",arg);
		
		if(arg != use_dma)		
			use_dma =  arg;
		else 
			break;
		
		DMSG_HCD("<am7x_hcd>@@@---request dma here\n");
		if(use_dma == 1){ /*need request dma*/
			result = am_request_dma(DMA_CHAN_TYPE_SPECIAL,
				"acthcd",aotg_hcd_dma_handler,
				0,(void *)acthcd);
			
			if(result < 0 ){
				ERR("<am7x_hcd>@@@---<HCD IOCTL>request DMA failed\n");
				result = -EBUSY;
				break;
			}
			
			acthcd->dma_nr = result;
			am_dma_cmd(acthcd->dma_nr,DMA_CMD_RESET);
			am_set_dma_irq(acthcd->dma_nr,1,0);
			
			/*set dma never timeout */
			dma_weight = am_get_dma_weight(acthcd->dma_nr);
			am_set_dma_weight(acthcd->dma_nr,dma_weight |(1UL<<28));
			DMSG_HCD("<am7x_hcd>@@@---<UDC_PROBE>DMA request success ,chnl:%d\n",result);
		}
		else{ /*release the dma we requested*/
			if(acthcd->dma_nr > 0)
				am_free_dma(acthcd->dma_nr);
			acthcd->dma_nr = -1;
		} 
		
		result = 0;
		break;
	default:
		result = -1;
		break;
	}

	return result;
}

static struct hc_driver act_hc_driver = {
	.description =		"am7x_hcd",
	.hcd_priv_size =	sizeof(struct aotg_hcd),
	
	/* generic hardware linkage*/
	.flags =		HCD_USB2  | HCD_MEMORY,
	.start =		aotg_hcd_start,
	.stop =		aotg_hcd_stop,
	.irq	 = 		aotg_hcd_irq,
	.hcd_ioctl = 	aotg_hcd_ioctl,
	
	.urb_enqueue       =	aotg_hcd_urb_enqueue,
	.urb_dequeue       =	aotg_hcd_urb_dequeue,
	.endpoint_disable =	aotg_hcd_endpoint_disable,

	/* periodic schedule support*/
	.get_frame_number =	aotg_hcd_get_frame,
	
	/* root hub support*/
	.hub_status_data =		aotg_hub_status_data,	 
	.hub_control =		aotg_hub_control,
	
#ifdef CONFIG_PM	
	.bus_suspend = aotg_hcd_suspend,
	.bus_resume = aotg_hcd_resume,
#endif
};


#if 0
static int in_pkt_available(struct urb *urb,struct aotg_hcep *ep)
{
	unsigned length;
	if ((aotg_readb(ep->reg_hcepcs) & EPCS_BUSY))
		return 0;
	length = aotg_readw(ep->reg_hcepbc);
	mb();
	if ( (length < ep->maxpacket )  && 
		(length == urb->transfer_buffer_length-urb->actual_length))
		return 1;
	return 0;
}

static int is_expect_last_pkt(struct urb *urb,struct aotg_hcep *ep)
{
	int remainder = urb->transfer_buffer_length - urb->actual_length;
	return (remainder <= (int)ep->maxpacket);
}
#endif


#ifdef  USE_EPREG_EX
static void sumpck_ctrl_enable(struct aotg_hcep *ep,struct urb* urb)
{
	u8 reg_val1,reg_val2;
	int pkt ;
	int cnt = 10;
	int condition = 0 ;
	
	if(ep->mask & USB_HCD_OUT_MASK){
		DMSG_HCD("<am7x_hcd>@@@---%s:ep out\n",__FUNCTION__);
		return;
	}	
	
	/*make sure the ep_valid and sumpkt_en is inactive,try 10 times*/	
	if(!ep->enable_pktctrl){	
		ep->enable_pktctrl = 1;
		do{		
			reg_val1 = (aotg_readb(ep->reg_hcin_auto_ctrl_usb)&0x0c)>>3;			
			reg_val2 = ( aotg_readb(ep->reg_hcin_auto_ctrl_up)& 0x0c)>>3;	
			if((reg_val1 == 0 ) && ( reg_val2 == 0)){
				condition = 1;
				break;
			}
		}while(cnt-- >0); 

		if(!condition){		
			ERR("<am7x_hcd>@@@---not match.reg1=0x%x,reg_val2=0x%x\n",reg_val1,reg_val2);
			return;
		}	
	}
	
	aotg_setbits(1<<AUTO_CTRL_SUMPCK_MASK,ep->reg_hcin_auto_ctrl_up);
	mb();
	pkt = (urb->transfer_buffer_length - urb->actual_length + ep->maxpacket -1)/ep->maxpacket;
	aotg_writeb(pkt,ep->reg_hcin_sumpck_nb);	
}


static void sumpck_ctrl_disable(struct aotg_hcep *ep)
{
	u8 reg_val=aotg_readb(ep->reg_hcin_auto_ctrl_up);
	if(reg_val&(1<<AUTO_CTRL_SUMPCK_MASK){
		invalidate_ep(ep);
		aotg_clearbits(1<<AUTO_CTRL_SUMPCK_MASK,ep->reg_hcin_auto_ctrl_up);
	}
}

static void frmpck_ctrl_enable(struct aotg_hcep *ep,int pck)
{
	u8 reg_val1,reg_val2;
	reg_val1=aotg_readb(ep->reg_hcin_auto_ctrl_up);
	reg_val2=aotg_readb(ep->reg_hcin_auto_ctrl_usb);
	if(reg_val1&0x10)
		invalidate_ep(ep);

	//Make sure that reg_hcin_auto_ctrl_usb equasl reg_hcin_auto_ctrl_up.
	do{
		reg_val1=aotg_readb(ep->reg_hcin_auto_ctrl_up);
		reg_val2=aotg_readb(ep->reg_hcin_auto_ctrl_usb);		
	}while((reg_val1!=reg_val2)||(reg_val1&0x10)); 

	aotg_writew(pck&((1<<14)-1),ep->reg_hcin_sumpck_nb);	
	aotg_setbits(1<<AUTO_CTRL_FRMPCK_MASK,ep->reg_hcin_auto_ctrl_up);
	//validate_ep(ep);
}


static void frmpck_ctrl_disable(struct aotg_hcep *ep)
{
	u8 reg_val=aotg_readb(ep->reg_hcin_auto_ctrl_up);
	if(reg_val&(1<<AUTO_CTRL_FRMPCK_MASK)){
		invalidate_ep(ep);		
		aotg_clearbits(1<<AUTO_CTRL_FRMPCK_MASK,ep->reg_hcin_auto_ctrl_up);
	}
}

static void frm_ctrl_enable(struct aotg_hcep * ep,int start_frame_number,int interval,int int_irq_en)
{
	u8 reg_val1,reg_val2;
	reg_val1=aotg_readb(ep->reg_hcin_auto_ctrl_up);
	reg_val2=aotg_readb(ep->reg_hcin_auto_ctrl_usb);
	if(reg_val1&0x10)
		invalidate_ep(ep);

	//Make sure that reg_hcin_auto_ctrl_usb equasl reg_hcin_auto_ctrl_up.
	do{
		reg_val1=aotg_readb(ep->reg_hcin_auto_ctrl_up);
		reg_val2=aotg_readb(ep->reg_hcin_auto_ctrl_usb);		
	}while((reg_val1!=reg_val2)||(reg_val1&0x10)); 

	aotg_writew(start_frame_number&((1<<14)-1),ep->reg_hcin_start_frm);
	aotg_writew(interval&((1<<14)-1),ep->reg_hcin_frm_interval);
	aotg_setbits(1<<AUTO_CTRL_FRM_MASK,ep->reg_hcin_auto_ctrl_up);

	//Generate an interrupt irq when it has received a NAK
	if(int_irq_en) {
		aotg_setbits((1<<AUTO_CTRL_INTIRQ_MASK),ep->reg_hcin_auto_ctrl_up);
	}
	else	{
		//Don't generate an interrtupt irq when it has received a NAK
		aotg_clearbits((1<<AUTO_CTRL_INTIRQ_MASK),ep->reg_hcin_auto_ctrl_up);
	}
	//validate_ep(ep);	
}


static void frm_ctrl_disable(struct aotg_hcep * ep)
{
	u8 reg_val=aotg_readb(ep->reg_hcin_auto_ctrl_up);
	if(reg_val&(1<<AUTO_CTRL_FRM_MASK))
	{
		invalidate_ep(ep);		
		aotg_clearbits((1<<AUTO_CTRL_FRM_MASK)|(1<<AUTO_CTRL_INTIRQ_MASK),ep->reg_hcin_auto_ctrl_up);
		mb();
	}	
}
#endif


 static int aotg_hcd_init(struct aotg_hcd *acthcd)
 {
	int i,retval = 0;
	int dma_weight;
	
	/*init software state*/
	spin_lock_init(&acthcd->lock);
	INIT_LIST_HEAD(&acthcd->async);
	acthcd->cur_ep = NULL;
	
	acthcd->next_async = acthcd->next_periodic = NULL;
	acthcd->fifo_addr = HC_FIFO_START_ADDR;
	acthcd->running =0;

	memset(acthcd->ep_ref_cnt,0,sizeof(acthcd->ep_ref_cnt));
	
	for(i = 0;i<AOTG_HCD_NUM_ENDPOINTS;i++)
	 	acthcd->ep_list[i] = NULL;
	
	acthcd->transceiver = otg_get_transceiver();
	if(!acthcd->transceiver) {
		ERR("<am7x_hcd>@@@---<HCD_INIT> no transceiver\n");
		retval = -ENODEV;
	 	return retval;
	} 

	aotg_init_hcep();
	
	/*request dma*/
	if(use_dma){
		retval = am_request_dma(DMA_CHAN_TYPE_SPECIAL,
				"acthcd",aotg_hcd_dma_handler,
				0,(void *)acthcd);
		
		if(retval < 0 ){
			ERR("<am7x_hcd>@@@--- <UDC_PROBE>request DMA failed\n");
			return -EBUSY;
		}	
		
		acthcd->dma_nr = retval;
		am_dma_cmd(acthcd->dma_nr,DMA_CMD_RESET);
		am_set_dma_irq(acthcd->dma_nr,1,0);
		
		/*set dma never timeout */
		dma_weight = am_get_dma_weight(acthcd->dma_nr);
		am_set_dma_weight(acthcd->dma_nr,dma_weight |(1UL<<28));
		INFO("<am7x_hcd>@@@---<UDC_PROBE>DMA request success ,chnl:%d\n",acthcd->dma_nr);
	}

	return 0;
 }


 static int aotg_hcd_probe(struct platform_device *pdev)
{
	struct usb_hcd	*hcd;
	struct aotg_hcd 	*acthcd;
	int				retval;
	hcd = usb_create_hcd (&act_hc_driver,&pdev->dev, (char *)hcd_name);
	if (!hcd) {
		ERR("<am7x_hcd>@@@---<HCD_PROBE>usb create hcd failed\n");
		retval =  -ENOMEM;
		goto err0;
	}
	/*init hcd */
	acthcd = hcd_to_aotg(hcd);
	retval = aotg_hcd_init(acthcd);
	if(retval) {
		ERR("<am7x_hcd>@@@---HCD_PROBE:hcd init failed\n");
		goto err1;
	}		
	
	retval = usb_add_hcd(hcd, IRQ_USB, IRQF_DISABLED |IRQF_SHARED);
	if (retval ){		
		ERR("<am7x_hcd>@@@---HCD_PROBE:usb add hcd failed\n");
		goto err1;
	}
	create_debug_file(hcd_to_aotg(hcd));
	msleep(5);
	
	spin_lock_irq(&acthcd->lock);
	acthcd->running = 1;
	spin_unlock_irq(&acthcd->lock);
	INFO("<am7x_hcd>@@@-- <HCD_PROBE> usb probe successfully!\n");


	am7x_hcd = hcd;
	am7x_hcd_task = kthread_run(ep_thread, NULL, "ep_thread");
	if (IS_ERR(am7x_hcd_task)){
		ERR("<am7x_hcd>@@@---<HCD_PROBE>kthread_run am7x_hcd_task failed\n");
		retval =  -ECHILD;
		goto err1;
	}
	wait_bulk_out_transfer_end=0;
	

	//Send_token_timeout_timer.expires = jiffies + 10*HZ;
	//add_timer(&Send_token_timeout_timer);
	return 0;
err1:
	usb_put_hcd(hcd);
err0:		
	return retval;	
}



 static int  aotg_hcd_remove( struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct aotg_hcd *acthcd = hcd_to_aotg(hcd);
	
	if(use_dma &&acthcd->dma_nr ){
		am_free_dma(acthcd->dma_nr);
		acthcd->dma_nr = -1;
	}

	
	//del_timer(&Send_token_timeout_timer);
	kthread_stop(am7x_hcd_task);
	remove_debug_file(acthcd);
	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);
	INFO("<am7x_hcd>@@@---aotg hcd removed sucessfully!\n");
	am7x_hcd=NULL;
	return 0;
}

#ifdef	CONFIG_PM

static int aotg_hcd_suspend(struct usb_hcd *hcd)/*struct platform_device *pdev, pm_message_t state*/
{
	//struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct aotg_hcd *acthcd = hcd_to_aotg(hcd);
	
	INFO("<am7x_hcd>@@@---%s:%d enter\n",__FUNCTION__,__LINE__);
	
	/* 
	  * fixme:first  suspend  device,clear
	  * busreq to give out suspend signal 
	  */
	aotg_clearbits((1<<0),OTGCTRL);
	mdelay(10);
	
	aotg_writeb(0x0,CLKGATE);
	
	/*suspend ourself*/
	otg_set_suspend(acthcd->transceiver,1);
	
	INFO("<am7x_hcd>@@@---%s:%d enter\n",__FUNCTION__,__LINE__);
	return 0;
}


static int aotg_hcd_resume(struct usb_hcd *hcd)/*struct platform_device *pdev*/
{
	//struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct aotg_hcd *acthcd = hcd_to_aotg(hcd);

	INFO("<am7x_hcd>@@@---%s:%d enter\n",__FUNCTION__,__LINE__);
	
	/*resume ourself*/
	otg_set_suspend(acthcd->transceiver,0);

	/*resume device*/
	aotg_setbits((1<<0),OTGCTRL);
	
	INFO("<am7x_hcd>@@@---%s:%d enter\n",__FUNCTION__,__LINE__);
	return 0;
}
#endif

static struct platform_driver hcd_am_driver = {
	.remove		= __exit_p(aotg_hcd_remove),
	/*default use usb subsystem to suspend & resume*/
	
#ifdef	CONFIG_PM
	//.suspend	= aotg_hcd_suspend,
	//.resume	= aotg_hcd_resume,
#endif
	.driver		= {
		.name	= "aotg-usb-am7x-",
		.owner	= THIS_MODULE,
	},
};



#if 0
static int ep_thread(void *__unused)
{
	/* khubd needs to be freezable to avoid intefering with USB-PERSIST
	 * port handover.  Otherwise it might see that a full-speed device
	 * was gone before the EHCI controller had handed its port over to
	 * the companion full-speed controller.
	 */
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	set_freezable();

	do{
		start_schedule(acthcd,NULL);
		wait_event_freezable(am7x_hcd_wait,
				!list_empty(am7x_hcd_event_list) ||
				kthread_should_stop());
	} while (!kthread_should_stop() || !list_empty(acthcd->async));
	return 0;
}

static void submit_ep(struct aotg_hcep	*ep,)
{
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	list_add_tail(ep->schedule, acthcd->async);
	kick_ep_thread();
}

static void kick_ep_thread(){
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);

	if(!list_empty(acthcd->async)){
		list_add_tail(&am7x_hcd_event, &am7x_hcd_event_list);
		wake_up(&am7x_hcd_wait);
	}
}


static u8 is_ep_busy(){
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	return acthcd->busy;
}

static void set_ep_busy(){
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	acthcd->busy=1;
}


static void set_ep_idle(){
	struct aotg_hcd *acthcd = hcd_to_aotg(am7x_hcd);
	acthcd->busy=0;
	kick_ep_thread();
}

#endif



static int __init am_hcd_init(void)
{
	INFO("driver %s, %s\n", hcd_name, DRIVER_VERSION);	
	return platform_driver_probe(&hcd_am_driver,aotg_hcd_probe);
}

static void __exit am_hcd_cleanup(void)
{
	platform_driver_unregister(&hcd_am_driver);
}





module_init(am_hcd_init);
module_exit(am_hcd_cleanup);


MODULE_DESCRIPTION("aotg udc driver");
MODULE_AUTHOR("actions, liucan");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:aotg_udc");
