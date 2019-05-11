# Makefile for lcm module 
PREFIX := drivers/char/lcm

obj-$(CONFIG_AM7X_LCM) := am7x_lcm.o 

am7x_lcm-objs += lcm.o  \
					DEdrive.o deapi.o	\
					hdmi/hdmi.o  \
					hdmi/hdcp_drv.o \
					hdmi/hdmi_ddc.o


EXTRA_CFLAGS := -I $(PREFIX)/include
