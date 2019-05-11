TARGET_TYPE = app
ROOT_DIR  = ./../../../../..
IMAGE     = sqldb
TARGET    = $(IMG_DIR)/$(IMAGE_APP)



INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \

LIBS = $(SDK_LIB_DIR)/../libsqlite3.so \
	$(CASE_LIB_DIR)/libsqldb.so \
	$(CASE_LIB_DIR)/libfilelist.so \
	$(SDK_LIB_DIR)/libmnavi.so  \
	$(SDK_LIB_DIR)/libid3.a \
	$(SDK_LIB_DIR)/lib_osapi.so \
	$(SDK_LIB_DIR)/libiconv.so \
	$(SDK_LIB_DIR)/libmusic.so \
	-lpthread -lrt -ldl --start-group -lm -lgcc --end-group 

include $(ROOT_DIR)/scripts/common.mak
