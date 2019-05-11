#
# Makefile for ACTIONS C67X00 USB Controller
#
ifeq ($(CONFIG_USB_DEBUG),y)
	EXTRA_CFLAGS		+= -DDEBUG
endif

obj-$(CONFIG_USB_AM7X_UOC)		+= am7x_uoc.o
obj-$(CONFIG_USB_GADGET_AM7X_UDC)	+= am7x_udc.o
obj-$(CONFIG_USB_AM7X_HCD)		+= am7x_hcd.o
ifeq ($(CONFIG_AM_CHIP_ID),1213)
obj-$(CONFIG_USB_AM7X_UOC)		+= am7x_uoc_next.o
endif
ifeq ($(CONFIG_AM_CHIP_ID),1213)
obj-$(CONFIG_USB_GADGET_AM7X_UDC)	+= am7x_udc_next.o
endif
ifeq ($(CONFIG_AM_CHIP_ID),1213)
obj-$(CONFIG_USB_AM7X_HCD)		+= am7x_hcd_next.o
ifeq ($(CONFIG_HID_BETWEEN_HUB),y)
obj-$(CONFIG_USB_AM7X_HCD)		+= am7x_hcd_multi.o
endif
endif
obj-$(CONFIG_USB_GAGET_AM7X_USB_SDRAM)	+= sbull/
