#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.
TARGET_TYPE = app

ROOT_DIR  = ./../../../../../..
LINUX_DIR = $(ROOT_DIR)/sdk/linux
SDK_INCLUDE = $(LINUX_DIR)/include/asm-mips/mach-am-mips/include

IMAGE     = I2C_IOCTL
TARGET    = $(IMG_DIR)/$(IMAGE_APP)

INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \
	$(SDK_DIR)/inc \
	$(COMMON_DIR) \
	$(SDK_INCLUDE)


LIBS = -L$(SDK_LIB_DIR) -lsres -lrt

include $(ROOT_DIR)/scripts/common.mak

