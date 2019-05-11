#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.
TARGET_TYPE = app

ROOT_DIR  = ./../../../..
IMAGE     = mousectrl
TARGET    = $(IMG_DIR)/$(IMAGE_APP)

INCS = $(SDK_DIR)/include 

LIBS = $(SDK_LIB_DIR)/libgraph.so \
 $(SDK_LIB_DIR)/lib_osapi.so \
 $(SDK_LIB_DIR)/liblib_ds.so \
 -lpthread -lrt -ldl --start-group -lm -lgcc --end-group 

include $(ROOT_DIR)/scripts/common.mak

