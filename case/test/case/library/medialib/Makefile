TARGET_TYPE = app
ROOT_DIR  = ./../../../../..
IMAGE     = medialib_test
TARGET    = $(IMG_DIR)/$(IMAGE_APP)



INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \
	$(SDK_DIR)/inc \
	$(COMMON_DIR)
	
	
LIBS = $(SDK_LIB_DIR)/libmlib.a \
	 $(SDK_LIB_DIR)/libvram.so\
	$(SDK_LIB_DIR)/libimageinfo.a \
	$(SDK_LIB_DIR)/libid3.a \
	$(SDK_LIB_DIR)/lib_osapi.so \
	-lpthread -lrt -ldl --start-group -lm -lgcc --end-group 

include $(ROOT_DIR)/scripts/common.mak