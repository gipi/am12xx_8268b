TARGET_TYPE = app

ROOT_DIR  = ./../../..
IMAGE     = reset
TARGET    = $(IMG_DIR)/$(IMAGE_APP)

INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \
	$(SDK_DIR)/inc \
	$(SDK_INCLUDE)\
	
 LIBS = \
 		-lpthread -lrt -ldl --start-group -lm -lgcc -lgcc_s -lc --end-group 
 
 
 
include $(ROOT_DIR)/scripts/common.mak   

