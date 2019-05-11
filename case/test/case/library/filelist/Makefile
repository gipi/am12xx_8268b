TARGET_TYPE = app
ROOT_DIR  = ./../../../../..
IMAGE     = filelist_test
TARGET    = $(IMG_DIR)/$(IMAGE_APP)



INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \

LIBS = $(SDK_LIB_DIR)/libmnavi.so \
	$(CASE_LIB_DIR)/libfilelist.so \
	-lpthread -lrt -ldl --start-group -lm -lgcc --end-group 


include $(ROOT_DIR)/scripts/common.mak