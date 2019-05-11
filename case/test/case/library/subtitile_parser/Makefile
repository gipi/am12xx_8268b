TARGET_TYPE = app
ROOT_DIR  = ./../../../../..
IMAGE     = subt_parser_test
TARGET    = $(IMG_DIR)/$(IMAGE_APP)



INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include 

LIBS = $(CASE_LIB_DIR)/libsubtitle_parser.a \
	-lpthread -lrt -ldl --start-group -lm -lgcc --end-group 


include $(ROOT_DIR)/scripts/common.mak