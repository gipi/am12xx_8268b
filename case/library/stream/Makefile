#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.
TARGET_TYPE = so

ROOT_DIR  = ./../../..
IMAGE     = stream
TARGET    = $(CASE_LIB_DIR)/$(IMAGE_SO)

INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \
	$(SDK_DIR)/inc \
	$(SDK_DIR)/library/wifi_subdisplay 
	

include $(ROOT_DIR)/scripts/common.mak

CFLAGS += -D_FILE_OFFSET_BITS=64
