#
# Makefile for the KS8841/2 ISA Ethernet driver
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...


obj-y += ksz884x.o

ksz884x-objs := device.o interrupt.o transmit.o hardware.o target.o \
	ks_config.o ks_mirror.o ks_qos.o ks_rate.o ks_stp.o \
	ks_table.o ks_vlan.o ks_Diag.o

EXTRA_CFLAGS := -DDEF_LINUX -DKS_ISA_BUS -DKS_ISA -DLINUX_2_6_27_29
EXTRA_CFLAGS += -DDEF_KS8841 #Why TX not called#
#EXTRA_CFLAGS += -DDEF_KS8842 #Tx called#


