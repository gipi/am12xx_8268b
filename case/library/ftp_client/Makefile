#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.
TARGET_TYPE = so

ROOT_DIR  = ./../../..
LINUX_DIR = $(ROOT_DIR)/sdk/linux
SDK_INCLUDE = $(LINUX_DIR)/include/asm-mips/mach-am-mips/include

IMAGE     = ftp_client
TARGET    = $(CASE_LIB_DIR)/$(IMAGE_SO)

INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \
	$(SDK_DIR)/inc \
	$(SDK_INCLUDE)/

LIBS = -lpthread

include $(ROOT_DIR)/scripts/common.mak

