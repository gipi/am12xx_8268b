TARGET_TYPE = app

ROOT_DIR  = ./../../..
IMAGE     = switchToAoa
TARGET    = $(IMG_DIR)/$(IMAGE_APP)

INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \
	$(SDK_DIR)/inc \
	$(SDK_INCLUDE)\
	
 LIBS = $(SDK_LIB_DIR)/../libusb-1.0.so\
 		-lpthread -lrt -ldl --start-group -lm -lgcc -lgcc_s -lc --end-group 
 
 
 
include $(ROOT_DIR)/scripts/common.mak   

