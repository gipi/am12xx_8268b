#ifndef  __USDK130_USB_HOST_AOTG_H 
#define  __USDK130_USB_HOST_AOTG_H 

#define  CONFIG_USB_HCD_DEBUG_FILES


#define	LOG2_PERIODIC_SIZE		6	/* arbitrary; this matches OHCI */ 
#define	PERIODIC_SIZE			(1 << LOG2_PERIODIC_SIZE) 
#define MAX_PERIODIC_LOAD		500 //50% residue for iso/int


//#define HC_ASYNC_FIFO_ADDR			0x00000080
//#define HC_SYNC_FIFO_ADDR			0x00000880

#define HC_FIFO_START_ADDR			0x00000080

#if CONFIG_AM_CHIP_ID ==1211 ||CONFIG_AM_CHIP_ID ==1220
#define HC_FIFO_END_ADDR			0x000010c0 //1211(4352byte)
#endif 

#if CONFIG_AM_CHIP_ID ==1213
#define HC_FIFO_END_ADDR			0x00002080 //1213(8448byte)bq
#endif

#define AUTO_CTRL_SUMPCK_MASK 	3
#define AUTO_CTRL_FRMPCK_MASK 	2
#define AUTO_CTRL_FRM_MASK    		1
#define AUTO_CTRL_INTIRQ_MASK 	0

 
#define  	USB_HCD_IN_MASK			0x00 
#define   USB_HCD_OUT_MASK  			0x10 
#define   	FIFOAUTO                                 	0x20


#define    FULLSPEED     		0 
#define    HIGHSPEED     		1 
#define    LOWSPEED     	 	2 


#define AOTG_PORT_C_MASK  ((USB_PORT_STAT_C_CONNECTION \
	| USB_PORT_STAT_C_ENABLE \
	| USB_PORT_STAT_C_SUSPEND \
	| USB_PORT_STAT_C_OVERCURRENT \
	| USB_PORT_STAT_C_RESET) << 16) 
 
#if CONFIG_AM_CHIP_ID ==1211||CONFIG_AM_CHIP_ID ==1220
#define AOTG_HCD_NUM_ENDPOINTS    	5   /*ep0, bulk-out, bulk-in, int-in,int-out*/ 
#endif

#if CONFIG_AM_CHIP_ID ==1213
#define AOTG_HCD_NUM_ENDPOINTS    	13   /*ep0, bulk-out, bulk-in, int-in,int-out*/ 
#endif
//#define AOTG_HCD_OUT_EP_BASE  	1 
//#define AOTG_HCD_IN_EP_BASE     	2  

#define AOTG_HCD_ASYNC_EPBASE		1	/*ep0,bulk-in,bulk-out,int-in,int-out*/	
#define AOTG_HCD_SYNC_EPBASE			3


/*bit of dma mode register*/
#define BIT_SRC_FIXSIZE				0
#define BIT_SRC_DRQ					3
#define BIT_SRC_FIXADDR				8
#define BIT_SRC_DIRECTION			9
#define BIT_SRC_BURSTTYPE			13

#define BIT_DST_FIXSIZE				16
#define BIT_DST_DRQ					19
#define BIT_DST_FIXADDR				24
#define BIT_DST_DIRECTION			25
#define BIT_DMA_RELOAD				28
#define BIT_DST_BURSTTYPE			29

/*dma burst type*/
#define BURST_SINGLE					0
#define BURST_INTCR4					3
#define BURST_INTCR8					5

#define HOST_READ_MODE	\
	((23<<BIT_SRC_DRQ) \
	|(1<<BIT_SRC_FIXADDR) \
	|(BURST_SINGLE<<BIT_SRC_BURSTTYPE)\
	|(16<<BIT_DST_DRQ) \
	|(0<<BIT_DST_FIXADDR)\
	|(BURST_INTCR4<<BIT_DST_BURSTTYPE))



#define HOST_WRITE_MODE \
	((16<<BIT_SRC_DRQ) \
	|(0<<BIT_SRC_FIXADDR) \
	|(BURST_INTCR4<<BIT_SRC_BURSTTYPE) \
	|(23<<BIT_DST_DRQ)\
	|(1<<BIT_DST_FIXADDR) \
	|(BURST_SINGLE<<BIT_DST_BURSTTYPE))

/*-----------bq add 1213 usb dma read/write mode--------------*/
#define HOST_READ_MODE_NEXT	\
	((19<<BIT_SRC_DRQ) \
	|(1<<BIT_SRC_FIXADDR) \
	|(BURST_SINGLE<<BIT_SRC_BURSTTYPE)\
	|(16<<BIT_DST_DRQ) \
	|(0<<BIT_DST_FIXADDR)\
	|(BURST_INTCR4<<BIT_DST_BURSTTYPE))

#define HOST_WRITE_MODE_NEXT \
	((16<<BIT_SRC_DRQ) \
	|(0<<BIT_SRC_FIXADDR) \
	|(BURST_INTCR4<<BIT_SRC_BURSTTYPE) \
	|(19<<BIT_DST_DRQ)\
	|(1<<BIT_DST_FIXADDR) \
	|(BURST_SINGLE<<BIT_DST_BURSTTYPE))



enum aotg_port_state { 
	AOTG_PORT_POWEROFF = 0, 
	AOTG_PORT_POWERED, 
	AOTG_PORT_ATTACHED, 
	AOTG_PORT_NOATTACHED, 
	AOTG_PORT_RESET, 
	AOTG_PORT_ENABLE, 
	AOTG_PORT_DISABLE, 
	AOTG_PORT_SUSPEND, 
	AOTG_PORT_ERR 
}; 

struct aotg_hcd{ 
	spinlock_t			lock;
	struct otg_transceiver	*transceiver;
	u8                    		 speed; 
	unsigned              		 inserted :1; 
	unsigned 				 running:1;
	enum aotg_port_state    portstate; 
	struct aotg_hcep     	*ep_list[AOTG_HCD_NUM_ENDPOINTS]; 

	u16 					load[PERIODIC_SIZE]; 
	struct aotg_hcep 		*periodic[PERIODIC_SIZE]; 
	unsigned 				periodic_count; 
	u16 					frame; 
	
	struct list_head 		async;		/*list for bulk/control ep*/
	struct aotg_hcep 		*next_async; 
	struct aotg_hcep		*next_periodic;
	
	u8					dma_nr;
	int 					stat_overrun;
	int 					stat_sof;
       u16  				peer_flags; 
	u32					fifo_addr;
	

#ifdef  CONFIG_USB_HCD_DEBUG_FILES
	struct proc_dir_entry		*pde;   
#endif	
	void*				private;   
	struct aotg_hcep 		*cur_ep;
	u8	busy;
	u8  ep_ref_cnt[AOTG_HCD_NUM_ENDPOINTS];
}; 


 #define  PEER_STALLED   (1<<0)
struct aotg_hcep { 
	struct usb_host_endpoint *hep; 
	struct usb_device *udev; 
	u32 maxpacket; 
	u32 fifo_addr;
	u8 epnum; 
	u8 nextpid; 
	u16 error_count; 
	u16 length; 
	int   index; 
	u8 mask; 
	u8 type; 
	u8 buftype; 
	
	unsigned long 	reg_hcepcs; 
	unsigned long    	reg_hcepcon; 
	unsigned long    	reg_hcepctrl; 
	unsigned long    	reg_hcepbc; 
	unsigned long    	reg_hcfifo; 
	unsigned long    	reg_hcmaxpck; 
	unsigned long    	reg_hcepaddr; 
	unsigned long		reg_hceperr;
	unsigned long 	reg_hcin_auto_ctrl_up;
#ifdef USE_EPREG_EX	
	unsigned long 	reg_hcin_auto_ctrl_usb;
	unsigned long 	reg_hcin_start_frm;
	unsigned long 	reg_hcin_frm_interval;
	unsigned long 	reg_hcin_frmpck_nb;
	unsigned long 	reg_hcin_sumpck_nb;
	unsigned long 	reg_hcin_sumpck_rcnt;	
#endif	
	unsigned          	stopped: 1, 
				       dma_working : 1,
				       enable_pktctrl :1,
				       inuse:1,
				       stalled:1;
	
	u32 dma_bytes;
	u16 period; 
	u16 branch; 
	u16 load; 
	struct aotg_hcep *next; 
	u8 fmindex; 
	struct list_head schedule; 
	u16		periodic_done_counter;	// high Bandwidth ????
	u8 ep_schedule;
}; 
 
static inline struct aotg_hcd *hcd_to_aotg(struct usb_hcd *hcd) 
{ 
	return (struct aotg_hcd *) (hcd->hcd_priv); 
} 
 
static inline struct usb_hcd *aotg_to_hcd(struct aotg_hcd *acthcd) 
{ 
	return container_of((void *)acthcd, struct usb_hcd, hcd_priv); 
} 

/*for get register address*/
static inline u32 get_hcin_auto_reg(int dir,u32 outbase,u32 inbase,int index)
{
	return (u32)(dir?outbase:(inbase + (index -1)*4));
}

static inline u32 get_hcepcon_reg(int dir,u32 outbase,u32 inbase,int index)
{
	return (u32)((dir ? outbase : inbase) + (index -1)*8);
}

static inline u32 get_hcepcs_reg(int dir,u32 outbase,u32 inbase,int index)
{
	return (u32)((dir ? outbase : inbase) + (index -1)*8);
}

static inline u32 get_hcepctrl_reg(int dir,u32 outbase,u32 inbase,int index)
{
	return (u32)((dir ? outbase : inbase) + (index -1)*4);
}

static inline u32 get_hcepbc_reg(int dir,u32 outbase,u32 inbase,int index)
{
	return (u32)((dir ? outbase : inbase) + (index -1)*8);
}

static inline u32 get_hcepmaxpck_reg(int dir,u32 outbase,u32 inbase,int index)
{
	return (u32)((dir ? outbase : inbase) + (index -1)*2);
}

static inline u32 get_hcfifo_reg(u32 addrbase,int index)
{
	return (u32)(addrbase + (index -1)*4);
}

static inline u32 get_hcepaddr_reg(int dir,u32 outbase,u32 inbase,int index)
{
	return (u32)((dir ? outbase : inbase) + (index -1)*4);
}

static inline u32 get_hceperr_reg(int dir,u32 outbase,u32 inbase,int index)
{
	return (u32)((dir ? outbase : inbase) + (index-1)*4);
}

#endif /* __USDK130_USB_HOST_AOTG_H*/ 
 
 
