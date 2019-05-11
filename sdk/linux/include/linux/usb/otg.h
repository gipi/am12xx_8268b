/* USB OTG (On The Go) defines */
/*
 *
 * These APIs may be used between USB controllers.  USB device drivers
 * (for either host or peripheral roles) don't use these calls; they
 * continue to use just usb_device and usb_gadget.
 */

#ifndef __LINUX_USB_OTG_H
#define __LINUX_USB_OTG_H

/* OTG defines lots of enumeration states before device reset */
enum usb_otg_state {
	
	OTG_STATE_UNDEFINED = -1,
	/*A state*/
	OTG_STATE_A_IDLE,
	OTG_STATE_A_WAIT_VRISE,
	OTG_STATE_A_WAIT_BCON,
	OTG_STATE_A_HOST,
	OTG_STATE_A_SUSPEND,
	OTG_STATE_A_PERIPHERAL,
	OTG_STATE_A_VBUS_ERR,
	OTG_STATE_A_WAIT_VFALL,
	
	/*B state*/
	OTG_STATE_B_IDLE,
	OTG_STATE_B_PERIPHERAL,
	OTG_STATE_B_WAIT_ACON,
	OTG_STATE_B_HOST,
	
	/*!!exception state*/
	OTG_STATE_B_SRP_INIT1,
	OTG_STATE_B_SRP_INIT2,
	OTG_STATE_B_DISCHRG1,
	OTG_STATE_B_DISCHRG2
};

struct port_handler{
	unsigned long data;
	int (*function)(void *target,int event);
};

/*
 * the otg driver needs to interact with both device side and host side
 * usb controllers.  it decides which controller is active at a given
 * moment, using the transceiver, ID signal, HNP and sometimes static
 * configuration information (including "board isn't wired for otg").
 */
struct otg_transceiver {
	struct device		*dev;
	const char		*label;

	u8			default_a;
	enum usb_otg_state	state;

	struct usb_bus		*host;
	struct usb_gadget	*gadget;

	/* to pass extra port status to the root hub */
	u16			port_status;
	u16			port_change;
	void *		port_handler; 

	/* bind/unbind the host controller */
	int	(*set_host)(struct otg_transceiver *otg,
				struct usb_bus *host);

	/* bind/unbind the peripheral controller */
	int	(*set_peripheral)(struct otg_transceiver *otg,
				struct usb_gadget *gadget);

	/* effective for B devices, ignored for A-peripheral */
	int	(*set_power)(struct otg_transceiver *otg,
				int is_on);

	/* for non-OTG B devices: set transceiver into suspend mode */
	int	(*set_suspend)(struct otg_transceiver *otg,
				int suspend);

	/* for B devices only:  start session with A-Host */
	int	(*start_srp)(struct otg_transceiver *otg);

	/* start or continue HNP role switch */
	int	(*start_hnp)(struct otg_transceiver *otg);

};


/* for board-specific init logic */
extern int otg_set_transceiver(struct otg_transceiver *);


/* for usb host and peripheral controller drivers */
extern struct otg_transceiver *otg_get_transceiver(void);
extern struct otg_transceiver *otg_get_transceiver_next(void);


static inline int
otg_start_hnp(struct otg_transceiver *otg)
{
	return otg->start_hnp(otg);
}


/* for HCDs */
static inline int
otg_set_host(struct otg_transceiver *otg, struct usb_bus *host)
{
	return otg->set_host(otg, host);
}


/* for usb peripheral controller drivers */
static inline int
otg_set_peripheral(struct otg_transceiver *otg, struct usb_gadget *periph)
{
	return otg->set_peripheral(otg, periph);
}

static inline int
otg_set_power(struct otg_transceiver *otg, unsigned mA)
{
	return otg->set_power(otg, mA);
}

static inline int
otg_set_suspend(struct otg_transceiver *otg, int suspend)
{
	if (otg->set_suspend != NULL)
		return otg->set_suspend(otg, suspend);
	else
		return 0;
}

static inline int
otg_start_srp(struct otg_transceiver *otg)
{
	return otg->start_srp(otg);
}


extern int otg_set_porthandler(struct otg_transceiver *transceiver,struct port_handler *process);
extern int otg_set_porthandler_next(struct otg_transceiver *transceiver,struct port_handler *process);//bq add

/* for OTG controller drivers (and maybe other stuff) */
extern int usb_bus_start_enum(struct usb_bus *bus, unsigned port_num);

#endif /* __LINUX_USB_OTG_H */
