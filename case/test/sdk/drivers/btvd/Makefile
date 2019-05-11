#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.
TARGET_TYPE = app

ROOT_DIR  = ./../../../../..
IMAGE     = btvd
TARGET    = ./$(IMAGE).$(TARGET_TYPE)

INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \
	$(SDK_DIR)/inc \
	$(COMMON_DIR) \
	$(SDK_INCLUDE)


LIBS = $(SDK_LIB_DIR)/libswfdec.so\
	   $(SDK_LIB_DIR)/libgraph.so\
	   $(SDK_LIB_DIR)/liblib_ds.so\
	   $(SDK_LIB_DIR)/lib_osapi.so\
	   $(SDK_LIB_DIR)/libsres.so \
	   $(SDK_LIB_DIR)/hantro_misc.so \
	   -lpthread -lrt -ldl --start-group -lm -lgcc --end-group \


include $(ROOT_DIR)/scripts/common.mak

