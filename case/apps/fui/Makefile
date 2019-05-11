#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.
TARGET_TYPE = app

ROOT_DIR  = ./../../..
IMAGE     = fui
TARGET    = $(IMG_DIR)/$(IMAGE_APP)

INCS = $(SDK_DIR)/include \
	$(SDK_DIR)/include/libetpan \
	$(CASE_DIR)/include \
	$(CASE_DIR)/include/stream\
	$(SDK_DIR)/include/curl \
	$(SDK_DIR)/include/flickcurl \
	$(SDK_DIR)/include/raptor\
	$(SDK_DIR)/inc \
	$(SDK_DIR)/include/dlna\
	$(SDK_DIR)/include/dlna_dmr\
	$(SDK_DIR)/include/libmms\
	$(SDK_DIR)/include/libwebsocket\
	$(SDK_DIR)/include/librtmp\
	$(SDK_DIR)/library/net/wpa_supplicant/wpa_supplicant-0.8/src/common\
	$(CASE_DIR)/include/bluetooth\
	$(COMMON_DIR) \
	$(SDK_INCLUDE)\
	./include \

LIBS = 	$(SDK_LIB_DIR)/libswfdec.so \
    $(CASE_LIB_DIR)/libfilelist.so \
    $(SDK_LIB_DIR)/libgraph.so \
    $(SDK_LIB_DIR)/libsres.so \
    $(SDK_LIB_DIR)/libmnavi.so  \
    $(SDK_LIB_DIR)/libmlib.a	\
    $(SDK_LIB_DIR)/libid3.a \
    $(CASE_LIB_DIR)/libapps_vram.a\
    $(SDK_LIB_DIR)/libvram.so\
    $(SDK_LIB_DIR)/libimageinfo.a\
    $(SDK_LIB_DIR)/liblib_ds.so \
    $(SDK_LIB_DIR)/lib_load_photo.so \
    $(CASE_LIB_DIR)/libalarm.a \
    $(SDK_LIB_DIR)/hantro_misc.so \
    $(SDK_LIB_DIR)/liblinein.so \
    $(SDK_LIB_DIR)/lib_osapi.so \
    $(SDK_LIB_DIR)/lib_enc_buf.so \
    $(SDK_LIB_DIR)/libiconv.so \
    $(CASE_LIB_DIR)/libstream.so\
    $(SDK_LIB_DIR)/libcurl.so \
    $(SDK_LIB_DIR)/libz.so    \
    $(SDK_LIB_DIR)/libavahi-core.so \
    $(SDK_LIB_DIR)/libavahi-common.so \
    $(SDK_LIB_DIR)/libcrypto.so \
    $(SDK_LIB_DIR)/libfreetype.so \
    $(SDK_LIB_DIR)/libfairplay.so \
    $(SDK_LIB_DIR)/libssl.so \
    $(SDK_LIB_DIR)/libsub00000000xy.so \
    $(SDK_LIB_DIR)/libplist.so \
    $(SDK_LIB_DIR)/libxml2.so \
    $(SDK_LIB_DIR)/libwifi_subdisplay.so \
		$(SDK_LIB_DIR)/libaudio_player.so \
	$(SDK_LIB_DIR)/libnetutils.so	\
	$(CASE_LIB_DIR)/libezcast.so \
    $(CASE_LIB_DIR)/libiw.so \
    $(CASE_LIB_DIR)/libmultilang.so    \
    -lpthread -lrt -ldl --start-group -lm -lgcc -lgcc_s -lc --end-group 

EZWIFI_CONFIG_FILE = $(ROOT_DIR)/scripts/ezwifi_config.h
 
-include $(ROOT_DIR)/scripts/module_config.mak
__module_config_inited := 1 
 
ifeq ($(findstring MODULE_CONFIG_FLASH_TYPE,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_FLASH_TYPE) 
    ifeq ($(findstring MODULE_CONFIG_EZWIRE_ENABLE,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_EZWIRE_ENABLE)
        LIBS += $(SDK_LIB_DIR)/../libusb-1.0.so
    endif
else
    LIBS += $(SDK_LIB_DIR)/../libusb-1.0.so
endif  
 
ifeq ($(findstring MODULE_CONFIG_VIDEO_SUBTITLE,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_VIDEO_SUBTITLE) 
LIBS += $(CASE_LIB_DIR)/libsubtitle_parser.a
endif  

ifeq ($(findstring MODULE_CONFIG_EBOOK,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_EBOOK) 
LIBS += $(CASE_LIB_DIR)/libfv.so
endif 

ifeq ($(findstring MODULE_CONFIG_FTP_CLIENT,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_FTP_CLIENT) 
LIBS += $(CASE_LIB_DIR)/libftp_client.so
endif 

ifeq ($(findstring MODULE_CONFIG_NETWORK,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_NETWORK) 
LIBS += $(SDK_LIB_DIR)/wpa_cli.so
endif 

LIBS += $(CASE_LIB_DIR)/libmirartp_stream.so


ifeq ($(findstring MODULE_CONFIG_PICASA,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_PICASA) 
#LIBS += $(SDK_LIB_DIR)/../libcurl.so
LIBS += $(SDK_LIB_DIR)/../libxml2.so
LIBS += $(CASE_LIB_DIR)/libwebalbum.so
endif

ifeq ($(findstring MODULE_CONFIG_FLICKR,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_FLICKR) 
#LIBS += $(SDK_LIB_DIR)/../libcurl.so
LIBS += $(SDK_LIB_DIR)/../libxml2.so
LIBS += $(SDK_LIB_DIR)/../libraptor2.so
LIBS += $(SDK_LIB_DIR)/../libflickcurl.so
LIBS += $(CASE_LIB_DIR)/libflickr.so
endif

ifeq ($(findstring MODULE_CONFIG_FACEBOOK,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_FACEBOOK) 
#LIBS += $(SDK_LIB_DIR)/../libcurl.so
LIBS += $(SDK_LIB_DIR)/../libjson.so
LIBS += $(CASE_LIB_DIR)/libfacebook.so
endif

ifeq ($(findstring MODULE_CONFIG_WEBMAIL,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_WEBMAIL) 
LIBS += $(SDK_LIB_DIR)/../libetpan.so
endif

#ifeq ($(findstring MODULE_CONFIG_WIFI_SUBDISPLAY,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_WIFI_SUBDISPLAY) 
#LIBS += $(SDK_LIB_DIR)/libwifi_subdisplay.so
#endif

ifeq ($(findstring MODULE_CONFIG_DLNA,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_DLNA) 
LIBS += $(SDK_LIB_DIR)/../libdlna.so
endif

ifeq ($(findstring MODULE_CONFIG_DLNA,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_DLNA) 
LIBS += $(SDK_LIB_DIR)/../libdlnadmr.so
endif

ifeq ($(findstring MODULE_CONFIG_QRCODE_GENERATOR,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_QRCODE_GENERATOR) 
LIBS += $(CASE_LIB_DIR)/libqrcode.so
endif

ifeq ($(findstring MODULE_CONFIG_SQLITE,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_SQLITE) 
LIBS += $(CASE_LIB_DIR)/libsqldb.so
LIBS += $(SDK_LIB_DIR)/../libsqlite3.so
LIBS += $(SDK_LIB_DIR)/../libmusic.so
endif

ifeq ($(findstring MODULE_CONFIG_HDCP_ENABLE,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_HDCP_ENABLE) 
LIBS += $(SDK_LIB_DIR)/../libhdcp2.so
endif

ifeq ($(findstring MODULE_CONFIG_IOS_CAPTURE,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_IOS_CAPTURE) 
LIBS += $(SDK_LIB_DIR)/../libimirror.so
LIBS += $(SDK_LIB_DIR)/../libimobiledevice.a
LIBS += $(SDK_LIB_DIR)/../libusbmuxd.a
LIBS += $(SDK_LIB_DIR)/../libplist.a
LIBS += $(CASE_LIB_DIR)/libwirePlayer.so
endif

ifeq ($(findstring MODULE_CONFIG_CHROMECAST,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_CHROMECAST)
LIBS += -Wl,-rpath-link,$(SDK_DIR)/lib/ -lChromecastSDK -lChromecastSDK_HW
endif

ifeq ($(shell grep \-w EZWIFI_CONFIG_MMSX_STREAMING ${EZWIFI_CONFIG_FILE} | cut -f5),1)
LIBS += $(SDK_LIB_DIR)/../libmms.so
endif

ifeq ($(shell grep \-w EZWIFI_CONFIG_CLOUD_CONTROL ${EZWIFI_CONFIG_FILE} | cut -f5),1)
LIBS += $(SDK_LIB_DIR)/../libwebsockets.so
endif

ifeq ($(shell grep \-w EZWIFI_CONFIG_EZCHANNEL_JS ${EZWIFI_CONFIG_FILE} | cut -f5),1)
LIBS += $(SDK_LIB_DIR)/../libduktape.so
endif

ifeq ($(shell grep \-w EZWIFI_CONFIG_RTMP_STREAMING ${EZWIFI_CONFIG_FILE} | cut -f5),1)
LIBS += $(SDK_LIB_DIR)/../librtmp.so
endif

SRC_DIR =  ./common 

ifeq ($(findstring MODULE_CONFIG_BLUETOOTH,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_BLUETOOTH) 
LIBS += $(CASE_DIR)/libs/lib/libbluetooth.so 
LIBS += $(SDK_DIR)/lib/libbfb.so 
LIBS += 	$(SDK_DIR)/lib/libexpat.so 
LIBS += 	$(SDK_DIR)/lib/libdbus-1.so 
LIBS += 	$(SDK_DIR)/lib/libmulticobex.so 
LIBS += 	$(SDK_DIR)/lib/libusbpp.so 
LIBS += 	$(SDK_DIR)/lib/libusb.so 
LIBS += 	$(SDK_DIR)/lib/libobexftp.so 
LIBS += 	$(SDK_DIR)/lib/libopenobex.so
endif
ifeq ($(findstring MODULE_CONFIG_SNMP_ENABLE,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_SNMP_ENABLE) 
LIBS += $(SDK_DIR)/libnetsnmpsrc.so
endif
#CFLAGS(fui.c) = -msoft-float

include $(ROOT_DIR)/scripts/common.mak   

