TARGET_TYPE = app
ROOT_DIR  = ./../../../../..
IMAGE     = webalbum_test
TARGET    = $(IMG_DIR)/$(IMAGE_APP)



INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include 

LIBS = $(CASE_LIB_DIR)/libwebalbum.so \
			$(SDK_LIB_DIR)/../libcurl.so \
			$(SDK_LIB_DIR)/../libxml2.so \
	-lpthread -lrt -ldl --start-group -lm -lgcc --end-group 


include $(ROOT_DIR)/scripts/common.mak