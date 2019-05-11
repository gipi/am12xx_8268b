#
# linux/drivers/devices/Makefile
#
#PREFIX := drivers/card
 
#CORE := ../Core
#INCLUDES := $(PREFIX)/Inc

KDIR = ../../

obj-$(CONFIG_UART_COM) += am7x_uartcom.o
#obj-m += am7x_uartcom.o

am7x_uartcom-objs := uartcom.o

#EXTRA_CFLAGS := -I $(INCLUDES)	

all: 
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
