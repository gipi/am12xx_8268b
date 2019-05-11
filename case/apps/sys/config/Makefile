#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.
TARGET_TYPE = app

ROOT_DIR  = ./../../../..
IMAGE     = config
TARGET    = $(IMG_DIR)/$(IMAGE_APP)

#OBJ_DIR := objs
#SRC_DIR := dir1 dir2

INCS = $(SDK_DIR)/include \
	$(CASE_DIR)/include \
	$(SDK_DIR)/inc \
	$(COMMON_DIR) \
	
LIBS = \
	$(CASE_LIB_DIR)/libapps_vram.a \
	$(SDK_LIB_DIR)/libvram.so\
	$(SDK_LIB_DIR)/libsres.so\
	-lrt

-include $(ROOT_DIR)/scripts/module_config.mak
__module_config_inited := 1

 ifeq ($(findstring MODULE_CONFIG_UCLIBC,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_UCLIBC)
 
 else
 	LIBS +=  $(SDK_LIB_DIR)/lib_osapi.so 
       ifeq ($(findstring MODULE_CONFIG_DISABLE_SWF_ENGINE,$(shell echo ${AM_MODULE_CONFIG_FLAGS})),MODULE_CONFIG_DISABLE_SWF_ENGINE) 
	LIBS += $(SDK_LIB_DIR)/hantro_misc.so 
	else
	LIBS += $(SDK_LIB_DIR)/libgraph.so \
		$(SDK_LIB_DIR)/libswfdec.so \
		$(SDK_LIB_DIR)/hantro_misc.so \
		$(SDK_LIB_DIR)/liblib_ds.so
	endif
endif


include $(ROOT_DIR)/scripts/common.mak

