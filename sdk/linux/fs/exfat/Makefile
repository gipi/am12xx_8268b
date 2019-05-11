#
# Makefile for the linux exfat-filesystem routines.
#

obj-$(CONFIG_EXFAT_FS) += exfat.o

exfat-y := bitmap.o cache.o cluster.o dir.o fatent.o \
	  inode.o super.o upcase.o utils.o	 

#EXTRA_CFLAGS = -DNTFS_VERSION=\"2.1.29\"

ifeq ($(CONFIG_EXFAT_DEBUG),y)
EXTRA_CFLAGS += -DDEBUG
endif

