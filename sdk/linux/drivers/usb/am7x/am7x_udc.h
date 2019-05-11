#ifndef  __USDK130_USB_GADGET_AOTG_H
#define  __USDK130_USB_GADGET_AOTG_H

#define	  EP0_PACKET_SIZE	        	64
#define     BULK_HS_PACKET_SIZE		512	
#define     BULK_FS_PACKET_SIZE		64	

#define 	BULK_HS_PACKET_MASK		9	
#define 	BULK_FS_PACKET_MASK		6

#define     INT_HS_PACKET_SIZE		1024
#define     INT_FS_PACKET_SIZE		64	

#define     ISO_HS_PACKET_SIZE		1024
#define     ISO_FS_PACKET_SIZE		1023	

#define 	IRQ_AOTG_DMA    				0

#define	EP2INSTARTADD   				0x00001080  
#define	EP1INSTARTADD   				0x00000880  
#define 	EP1OUTSTARTADD  			0x00000080

#define  	USB_UDC_IN_MASK			0x10
#define   USB_UDC_OUT_MASK 			0x00
#define   	FIFOAUTO                                 	0x20

#define CONFIG_USB_GADGET_DEBUG_FILES
#define SUPPORT_INTIN			

#ifndef	CONFIG_AOTG_EP_AUTO_CONFIG
#ifdef SUPPORT_INTIN
#define	AOTG_UDC_NUM_ENDPOINTS		4 /*ep0,ep1 bulk-in,ep2 bulk-out,ep2 in-int*/
#else
#define 	AOTG_UDC_NUM_ENDPOINTS		3 /*ep0,ep1 bulk-in,ep2 bulk-out*/
#endif
#else
#define	AOTG_UDC_NUM_ENDPOINTS		9
#endif

#define	AOTG_UDC_EP_NAME_LENTH		15
#define	AOTG_UDC_IN_EP_BASE			1
#define	AOTG_UDC_OUT_EP_BASE		5

struct aotg_udc;
struct aotg_ep {	
	struct usb_ep				        		ep;
	struct aotg_udc			        		*dev;
	const struct usb_endpoint_descriptor		*desc;
	struct list_head			       			queue;
	
	u16				      				 	maxpacket;
	u8									pkt_msk;
	u8					           			bEndpointAddress;
	u8					          			bmAttributes;
	u8                                     				mask;
	/*indicate ep buftype*/
	u8                                     				buftype;

	int									dma_nr;
	
	unsigned				           			stopped : 1,
					           				dma_working : 1;
    	u32                           					dma_bytes;	
	/*ep op register*/	
	unsigned long							reg_udccs;/*read only after init assignment*/
   	unsigned long                  				reg_udccon;/*read only after init assignment*/
	unsigned long							reg_udcbc;/*read only after init assignment*/
	unsigned long							reg_udcfifo;/*read only after init assignment*/

#ifdef	CONFIG_AOTG_EP_AUTO_CONFIG
	unsigned long							reg_epmaxpacket;
	unsigned long							reg_epaddr;
#endif
};


struct aotg_request {
	struct usb_request						req;
	struct list_head						queue;
};


enum ep0_state {
        EP0_WAIT_FOR_SETUP,
        EP0_IN_DATA_PHASE,
        EP0_OUT_DATA_PHASE,
        EP0_END_XFER,
        EP0_STALL,
};
                                                                                                             
enum udc_state {
        UDC_UNKNOWN,
        UDC_ACTIVE,
        UDC_SUSPEND,
        UDC_DISABLE,
};


struct aotg_udc {
	spinlock_t					lock;

	/*for register interface gadget */
	struct usb_gadget				gadget;
	struct usb_gadget_driver		*driver;
	
    	struct aotg_ep			   	ep [AOTG_UDC_NUM_ENDPOINTS];
	enum ep0_state		    		ep0state;	 /*for setup*/
	enum udc_state                   	state;	
	
	unsigned                               	softconnect : 1,
						    		rwk:1,
						    		highspeed: 1,
						    		req_pending : 1,
						   		req_std : 1,
						   	 	req_config : 1,
								enabled:1;
	/*interface to otg layer*/
	struct otg_transceiver			*transceiver; 
	int 							dma_channel;
	int 							dma_channel2;
	unsigned long 					dma_used;
#define DMA_CHANNEL1_USED		0
#define DMA_CHANNEL2_USED		1

	struct tasklet_struct 			handle_err_tasklet;
#ifdef  CONFIG_USB_GADGET_DEBUG_FILES
	struct proc_dir_entry 				*pde;	
#endif	
	void *						private;
};


#ifdef VERBOSE_DEBUG
#    define VDBG		DBG
#else
#    define VDBG(stuff...)	do{}while(0)
#endif

#ifdef PACKET_TRACE
#    define PACKET		VDBG
#else
#    define PACKET(stuff...)	do{}while(0)
#endif

#endif /* __USDK130_USB_GADGET_AOTG_H */





