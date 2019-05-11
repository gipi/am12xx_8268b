#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.


TARGET_TYPE = app
ROOT_DIR  = ./../../../../..
IMAGE     = graph_test
TARGET    =./$(IMAGE).$(TARGET_TYPE)
#TARGET    =$(ROOT_DIR)/sdk/rootfs/test/$(IMAGE).$(TARGET_TYPE)
#POST_BUILD_CMD +=cp -r ./input $(ROOT_DIR)/sdk/rootfs/test/input


INCS = $(SDK_DIR)/include\
			 $(SDK_DIR)/library/graph 
			 
LIBS = $(SDK_LIB_DIR)/libgraph.so\
			 $(SDK_LIB_DIR)/liblib_ds.so\
			 $(SDK_DIR)/library/graph/lib_osapi.a\
			 -lpthread -lrt -ldl --start-group -lm -lgcc --end-group \


	
include $(ROOT_DIR)/scripts/common.mak



