TARGET_TYPE = app
ROOT_DIR  = ./../../../../..
IMAGE     = flickr_test
TARGET    = $(IMG_DIR)/$(IMAGE_APP)



INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \
	$(SDK_DIR)/include/curl \
	 $(SDK_DIR)/include/flickcurl \
	 $(SDK_DIR)/include/raptor

LIBS = $(CASE_LIB_DIR)/libflickr.so \
			$(SDK_LIB_DIR)/../libcurl.so \
			$(SDK_LIB_DIR)/../libxml2.so \
			$(SDK_LIB_DIR)/../libraptor2.so \
			$(SDK_LIB_DIR)/../libflickcurl.so \
	-lpthread -lrt -ldl --start-group -lm -lgcc --end-group 


include $(ROOT_DIR)/scripts/common.mak