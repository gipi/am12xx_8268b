#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.
TARGET_TYPE = so

ROOT_DIR  = ./../../..
IMAGE     = facebook
TARGET    = $(CASE_LIB_DIR)/$(IMAGE_SO)

INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \
	$(SDK_DIR)/inc 
	
LIBS = $(SDK_LIB_DIR)/../libcurl.so \
				$(SDK_LIB_DIR)/../libjson.so \
			-lpthread 

include $(ROOT_DIR)/scripts/common.mak

CC=$(GCC)  -std=c99