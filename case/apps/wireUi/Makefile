#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.
TARGET_TYPE = app

ROOT_DIR  = ./../../..
IMAGE     = wireUI
TARGET    = $(IMG_DIR)/$(IMAGE_APP)

INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \
	$(SDK_DIR)/inc \
	$(SDK_INCLUDE)\

LIBS = $(SDK_LIB_DIR)/libsres.so\
		$(SDK_LIB_DIR)/../libusb-1.0.so    \
		$(SDK_LIB_DIR)/lib_osapi.so \
		$(SDK_LIB_DIR)/hantro_misc.so \
		$(SDK_LIB_DIR)/lib_load_photo.so	\
		$(SDK_LIB_DIR)/libstream.so	\
		$(CASE_LIB_DIR)/libwirePlayer.so    

		
-include $(ROOT_DIR)/scripts/module_config.mak
__module_config_inited := 1 

ifeq ($(findstring MODULE_CONFIG_IOS_MIRROR_ONLY,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_IOS_MIRROR_ONLY) 
LIBS +=	$(SDK_LIB_DIR)/libezota.so \
		$(SDK_LIB_DIR)/../libimirror.so	\
		$(SDK_LIB_DIR)/../libimobiledevice.a	\
		$(SDK_LIB_DIR)/../libusbmuxd.a	\
		$(SDK_LIB_DIR)/../libplist.a	\
		-lssl -lcrypto  -lpthread -lrt -ldl --start-group -lm -lgcc -lgcc_s -lc --end-group 
	
else ifeq ($(findstring MODULE_CONFIG_ADB_MIRROR_ONLY,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_ADB_MIRROR_ONLY) 
LIBS += $(SDK_LIB_DIR)/libezota.so \
		-lpthread -lrt -ldl --start-group -lm -lgcc -lgcc_s -lc --end-group
else
LIBS += $(SDK_LIB_DIR)/../libimirror.so	\
		$(SDK_LIB_DIR)/../libimobiledevice.a	\
		$(SDK_LIB_DIR)/../libusbmuxd.a	\
		$(SDK_LIB_DIR)/../libplist.a	\
		$(SDK_LIB_DIR)/libswfdec.so \
		$(SDK_LIB_DIR)/libgraph.so \
		$(SDK_LIB_DIR)/liblib_ds.so \
		$(SDK_LIB_DIR)/libezcast.so \
		$(SDK_LIB_DIR)/libwifi_subdisplay.so \
		$(SDK_LIB_DIR)/libaudio_player.so 
	ifeq ($(findstring MODULE_CONFIG_UCLIBC,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_UCLIBC) 
	LIBS += 	-lssl -lcrypto  -lpthread -lrt -ldl --start-group -lm -lgcc -lgcc_s -lc --end-group 
	else
	LIBS +=	-lssl -lcrypto -lxml2 -lpthread -lrt -ldl --start-group -lm -lgcc -lgcc_s -lc --end-group 
	endif
endif

ifeq ($(findstring MODULE_CONFIG_AD_UI_ENABLE,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_AD_UI_ENABLE) 
LIBS += $(CASE_LIB_DIR)/libqrcode.so
LIBS += $(SDK_LIB_DIR)/lib_enc_buf.so
endif
 
include $(ROOT_DIR)/scripts/common.mak   

