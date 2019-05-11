#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.
TARGET_TYPE = so

ROOT_DIR  = ./../../..
IMAGE     = flickr
TARGET    = $(CASE_LIB_DIR)/$(IMAGE_SO)

INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \
	$(SDK_DIR)/inc \
	 $(SDK_DIR)/include/curl \
	 $(SDK_DIR)/include/flickcurl \
	 $(SDK_DIR)/include/raptor
	 
	
LIBS = $(SDK_LIB_DIR)/../libcurl.so \
				$(SDK_LIB_DIR)/../libxml2.so \
				$(SDK_LIB_DIR)/../libraptor2.so \
				$(SDK_LIB_DIR)/../libflickcurl.so \
			-lpthread 

include $(ROOT_DIR)/scripts/common.mak