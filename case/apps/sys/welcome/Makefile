#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.
TARGET_TYPE = bin

ROOT_DIR  = ./../../../..
IMAGE     = recover
TARGET    = $(IMG_DIR)/$(IMAGE_BIN)

#OBJ_DIR := objs
#SRC_DIR := dir1 dir2

LDSCRIPT = link.xn

INCS = $(SDK_DIR)/include \
	
LIBS = $(SDK_LIB_DIR)/libbootdisplay.a \


include $(ROOT_DIR)/scripts/common.mak

CFLAGS_EXTRA += -fno-builtin
