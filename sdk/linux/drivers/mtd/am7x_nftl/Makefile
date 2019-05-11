#
# linux/drivers/devices/Makefile
#
obj-$(CONFIG_AM_NFTL)	+= am7x_nftl.o

PREFIX := drivers/mtd/am7x_nftl
COMMON := Flash/common

lib-src        := $(shell find -L $(PREFIX)/$(COMMON) -name '*.c')
am7x_nftl-objs := am7x_flash.o am7x_flash_test.o $(patsubst $(PREFIX)/%,%,$(lib-src:.c=.o))

INCLUDES := . Flash include $(COMMON)/include $(COMMON)/include/include-1201
INCLUDES := $(foreach dir,$(INCLUDES),-I $(PREFIX)/$(dir))

EXTRA_CFLAGS :=  -D_NAND_TYPE_=11 $(INCLUDES) \
	-DAM_CHIP_ID=CONFIG_AM_CHIP_ID

