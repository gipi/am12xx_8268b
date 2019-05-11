#ifndef  __USDK130_AOTG_H 
#define  __USDK130_AOTG_H 

/*
 *notfication event
 */
#include <linux/kernel.h>   

#undef ERR
#undef WARN
#undef INFO

//#define	DEBUG_USB
//#define 	DEBUG_OTG
//#define	DEBUG_UOC
//#define	DEBUG_HCD
//#define	DEBUG_UDC
//#define	DEBUG_XFERS
//#define	DEBUG_CTRL_XFERS
//#define	DEBUG_BULK_XFERS
//#define 	DEBUG_USBDMA

#define	AOTG_SOFT_DISCONN_EVENT    				(1<<0)
#define	AOTG_SOFT_CONN_EVENT					(1<<1)
#define	AOTG_SUSPEND_EVENT						(1<<2)
#define	AOTG_RESUME_EVENT						(1<<3)
#define	AOTG_HOST_TO_PERIPHERAL_EVENT		(1<<4)
#define	AOTG_PERIPHERAL_TO_HOST_EVENT		(1<<5)
#define	AOTG_PLUGIN_EVENT							(1<<6)
#define	AOTG_PLUGOUT_EVENT						(1<<7)
#define	AOTG_VBUS_DROP_EVENT					(1<<8)
#define	AOTG_ID_CHANGE_EVENT					(1<<9)
#define   AOTG_PERIPHERAL_DISCONN_EVENT		(1<<10)


#define     EOTGSTATE     50  			/*error code for OTG state*/ 
#define	AOTG_EP_MAX_BUFFER		4
/*
#define  get_epcon_reg(dir,x,y,z)   ((dir?x:y) + (z -1)*8)
#define  get_epcs_reg(dir,x,y,z)   ((dir?x:y) + (z -1)*8)
#define  get_epctrl_reg(dir,x,y,z)   ((dir?x:y) + (z -1)*4)
#define  get_epbc_reg(dir,x,y,z)   ((dir?x:y) + (z -1)*8)
#define  get_epmaxpck_reg(dir,x,y,z)   ((dir?x:y) + (z -1)*2)
#define  get_fifo_reg(x,z)   (x + (z-1)*4)
#define  get_epaddr_reg(dir,x,y,z)  ((dir?x:y) + (z -1)*4)
*/

static inline void   aotg_writeb(u8 val, u32 reg)
{
	*(volatile u8 *)(reg) = val;
	mb();
}

static inline  void aotg_writew(u16 val, u32 reg)
{
	*(volatile u16 *)(reg) = val;
	mb();
}

static inline  void aotg_writel(u32 val, u32 reg)
{
	*(volatile u32 *)(reg) = val;
	mb();
}

static inline void __aotg_writel(u32 val, u32 reg)
{
	*(volatile u32 *)(reg) = val;
	barrier();
}


static inline u8 aotg_readb(u32 port)
{
	u8 val;
	val = (*(volatile u8 *)port);
	mb();
	return val;
}

static inline u16 aotg_readw(u32  port)
{
	u16 val;
	val = (*(volatile u16 *)port);
	mb();
	return val;
}

static inline u32 aotg_readl(u32 port)
{
	u32 val;
	val = (*(volatile u32 *)port);
	mb();
	return val;
}

static inline u32 __aotg_readl(u32 port)
{
	u32 val;
	val = (*(volatile u32 *)port);
	barrier();
	return val;
}

static inline void aotg_setbits(u8 val,u32 reg)
{
	*(volatile u8 *)reg  =  (*(volatile u8 *)reg ) | val;
	mb();
}

static inline void aotg_clearbits(u8 val,u32 reg)
{
	*(volatile u8 *)reg  =  (*(volatile u8 *)reg ) & (~val);
	mb();
}

static inline void aotg_clear_pend(u8 val,u32 reg)
{
#if CONFIG_AM_CHIP_ID == 1211
	int i=0;
	for(i=0;i<8;i++)
#endif
	*(volatile u8 *)(reg) = val;
	mb();
}

static inline void aotg_clear_pendbits(u8 val,u32 reg)
{
#if CONFIG_AM_CHIP_ID == 1211
	int i=0;
	for(i=0;i<8;i++)
#endif
	*(volatile u8 *)reg  =  (*(volatile u8 *)reg ) | val;
	mb();
}


#define 	EPERR_NONE				0x00
#define 	EPERR_CRC				0x01
#define 	EPERR_TOGGLE			0x02
#define 	EPERR_STALL				0x03
#define 	EPERR_TIMEOUT			0x04
#define 	EPERR_PID				0x05
#define 	EPERR_DATAOVER		0x06
#define 	EPERR_DATAUNDER		0x07

#define 		get_ep_errtype(reg) \
			((aotg_readb(reg)>>2) & 0x07)
				
#define		get_otg_state()    (aotg_readb(OTGSTATE)&0x0f)
#define    	get_otg_id()  \
			((aotg_readb(OTGSTATUS) & OTGSTATUS_ID) >> 6)

#define		enable_srp_detect() \
			(aotg_setbits(OTGCTRL_SRPDATDETEN | OTGCTRL_SRPVBUSDETEN,OTGCTRL))
#define		disable_srp_detect() \
			(aotg_clearbits(OTGCTRL_SRPDATDETEN | OTGCTRL_SRPVBUSDETEN,OTGCTRL))
#define 		is_b_sess_vld  \
			((aotg_readb(OTGSTATUS) & (1<<2)) >> 2) 

#define  		is_b_sess_end  \
			(aotg_readb(OTGSTATUS) & OTGSTATUS_BSESSEND) 

#define		is_a_bus_drop \
	       	 	(aotg_readb(OTGCTRL) & OTGCTRL_ABUSDROP)

#define		is_a_bus_vld \
			(aotg_readb(OTGSTATUS) & OTGSTATUS_AVBUSSVAL)
		
#define   		is_b_conn  \
			(aotg_readb(OTGSTATUS) & OTGSTATUS_CONN)
#define     	is_a_conn 	is_b_conn

#define		is_b_bus_req  \
			(aotg_readb(OTGCTRL) & OTGCTRL_BUSREQ)

#define		is_a_bus_req  is_b_bus_req

#define		is_a_set_b_hnp_en \
			(aotg_readb(OTGCTRL) & OTGCTRL_ASETBHNPEN)

#define		set_b_hnp_en() \
			(aotg_setbits(OTGCTRL_BHNPEN,OTGCTRL))

#define		set_a_set_bhnp() \
			(aotg_setbits(OTGCTRL_ASETBHNPEN,OTGCTRL))
	

#define		dplus_up()  	aotg_clearbits(USBCS_DISCONN,USBCS)
#define		dplus_down()  	aotg_setbits(USBCS_DISCONN,USBCS)

#define 		is_b_disconn	\
			(aotg_readb(USBCS) & USBCS_DISCONN)
			
#define		enable_udc_interrupt()   \
			aotg_setbits(USBIEN_URES |USBIEN_HS | USBIEN_SUDAV |USBIEN_SUSP,USBIEN)
			
#define		disable_udc_interrupt() \
			aotg_clearbits(USBIEN_URES |USBIEN_HS | USBIEN_SUDAV |USBIEN_SUSP,USBIEN)

#ifdef  DEBUG_USB
#define DMSG(fmt,stuff...)   printk(KERN_INFO fmt,##stuff)
#else
#define DMSG(fmt,stuff...)   do {} while (0)
#endif

#define ERR(fmt,stuff...)			printk(KERN_NOTICE fmt,##stuff)
#define WARN(fmt,stuff...)		printk(KERN_WARNING fmt,##stuff)
#define INFO(fmt,stuff...)			printk(KERN_INFO fmt,##stuff)


#ifdef DEBUG_XFERS
#define DMSG_XFERS(stuff...)	DMSG("<xfer>" stuff)
#else
#define DMSG_XFERS(stuff...)	do{}while(0)
#endif

#ifdef DEBUG_USBDMA
#define DMSG_DMA(stuff...)		DMSG("<dma>" stuff)
#else
#define DMSG_DMA(stuff...)		do{}while(0)
#endif

#ifdef DEBUG_CTRL_XFERS
#define DMSG_CTRL(stuff...)		DMSG("<ctrl>" stuff)
#else
#define DMSG_CTRL(stuff...)		do{}while(0)
#endif

#ifdef DEBUG_BULK_XFERS
#define DMSG_BULK(stuff...)		DMSG("<bulk>" stuff)
#else
#define DMSG_BULK(stuff...) 		do{}while(0)
#endif


#ifdef DEBUG_OTG
#define DMSG_OTG(stuff...)		DMSG("<otg>" stuff)
#else
#define DMSG_OTG(stuff...) 		do{}while(0)
#endif

#ifdef DEBUG_HCD
#define DMSG_HCD(stuff...)		DMSG("<hcd>" stuff)
#else
#define DMSG_HCD(stuff...) 		do{}while(0)
#endif

#ifdef DEBUG_UDC
#define DMSG_UDC(stuff...)		DMSG("<udc>" stuff)
#else
#define DMSG_UDC(stuff...) 		do{}while(0)
#endif

#ifdef DEBUG_UOC
#define DMSG_UOC(stuff...)		DMSG("<uoc>" stuff)
#else
#define DMSG_UOC(stuff...) 		do{}while(0)
#endif
//#define   CHIP_1201

#endif/* __USDK130_AOTG_H */
