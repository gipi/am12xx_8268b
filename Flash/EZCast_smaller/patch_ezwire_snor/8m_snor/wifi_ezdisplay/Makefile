#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.
TARGET_TYPE = so

ROOT_DIR  = ./../../..
IMAGE     = wifi_subdisplay
TARGET    = $(SDK_LIB_DIR)/$(IMAGE_SO)

INCS = $(SDK_DIR)/include \
	$(SDK_DIR)/inc \
	$(CASE_DIR)/include

#INCS = $(SDK_DIR)/include $(SDK_DIR)/library/dlna_dmr $(SDK_DIR)/library/wifi_ezdisplay_dep/include \
#	$(SDK_DIR)/inc \
#	$(CASE_DIR)/include \
#	$(SDK_DIR)/include/dlna_dmr \
#	$(SDK_DIR)/include/curl \
#	$(SDK_DIR)/include/libmms \
#	$(SDK_DIR)/include/libwebsocket \
#	$(SDK_DIR)/include/librtmp

LIBS = -L$(SDK_DIR)/lib -lpthread -lrt -lcrypto -lssl

#LIBS = -L$(SDK_DIR)/lib -ldlnadmr -lpthread -lrt -lcrypto -lfreetype -lz -lfairplay -lavahi-core -lavahi-common -lcurl -lmms -lrtmp -lssl -lduktape -lsub00000000xy -lplist -lxml2 -lwebsockets

include $(ROOT_DIR)/scripts/common.mak

