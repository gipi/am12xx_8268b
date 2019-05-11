# Makefile for the kernel block device drivers.
#
# 12 June 2000, Christoph Hellwig <hch@infradead.org>
# Rewritten to use lists instead of if-statements.
# 
#upgrade_common.o : upgrade_common.c   upgrade_common.h            

obj-$(CONFIG_AM7X_OTA)		    := am7x_ota_upgrade.o 
am7x_ota_upgrade-objs += ota_upgrade.o ota_common.o \
		     zlib/adler32.o	\
			zlib/crc32.o    \
			zlib/inffast.o  \
			zlib/inflate.o  \
			zlib/inftrees.o \
			zlib/zutil.o	\
			zlib/zmem.o

