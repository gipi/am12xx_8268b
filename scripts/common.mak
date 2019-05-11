#**************************************************************************
#                                                                         *
#   PROJECT     : MIPS port for LINUX                                     *
#   MODULE      : MakeFile                                                *
#   AUTHOR      : Pan Ruochen (reachpan@actions-micro.com)                *
#   PROCESSOR   : MIPS 4KEc (32 bit RISC) - ActionsMicro 7XXX Family      *
#   Tool-chain  : CodeSourcery_G++_Lite                                   *
#                                                                         *
#   DESCRIPTION :                                                         *
#   A common Makefile template for ActionsMicro Linux SDK.                *
#                                                                         *
#   Please refer to the document 'User Manual.doc' for more detailed      *
#   information.                                                          *
#                                                                         *
#   History:                                                              *
#   1.  The first version. (2009/12/15)                                   *
#   2.  Simplify the script. (2010/4/7)                                   *
#   3.  Support raw binary format. (2010/9/7)                             *
#**************************************************************************

cpp_suffix := cpp cxx cc
src_suffix := S c $(cpp_suffix)

sv_src_dir   := SRC_DIR
sv_src_files := SRC_EXTRA
sv           := $(sv_src_dir) $(sv_src_files)

#@ strip both the name and the value of the variable
#@ in order to be tolerant to spaces in function calls
__strip_name_and_value = $(strip $($(strip $1)))

#@ Assert the variables not to be empty !
__assert_nez = $(if $(call __strip_name_and_value,$1),,$(error $1 is empty!))
#@ Assert the variables to be empty !
__assert_ez = $(if $(call __strip_name_and_value,$1),$(error $1 is not empty!),)

__trace = $(foreach i,$1,$(warning $i = '$($i)'))
__eq = $(findstring $(strip $2),$(findstring $(strip $1),$(strip $2)))

## `make v=1' to display verbose commands
ifeq ($(v),)
__invoke = $(if $1,@$(if $2,printf '  %-06s%s\n' $2;)$1)
else
__invoke = $1
endif

$(eval $(call __assert_nez, TARGET_TYPE))

#@ Strip the value of the variable
define __strip_variable
$1 := $(strip $($1))

endef
$(eval $(foreach i,IMAGE TARGET_TYPE ROOT_DIR OBJ_DIR STRIP_UNUSED,\
$(if $($i),$(call __strip_variable,$i))))

#@ Add a backsplash to the end of the string if it is not empty
#__get_dir_prefix = $(if $1,$1/)
__get_dir_prefix = $(filter-out /,$1/)
am-scripts := scripts

include $(ROOT_DIR)/$(am-scripts)/config.mak
$(eval $(call __assert_nez, AM_CHIP_ID_MAIN))

cflags-chipid := -DAM_CHIP_ID=$(AM_CHIP_ID_MAIN) $(if $(AM_CHIP_ID_MINOR),-DAM_CHIP_MINOR=$(AM_CHIP_ID_MINOR))

supported-types := ar app so bin
ifeq ($(findstring $(TARGET_TYPE),$(supported-types)),)
$(error The supported target types are: $(supported-types))
endif

#----------------------------------------------------
#             GNU Toolchains
#----------------------------------------------------
CROSS    ?= mips-linux-gnu-
GCC      := $(CROSS)gcc
LD        = $(if $(filter $(foreach i,$(cpp_suffix),%.$i),$(src-all)),$(CROSS)g++,$(CROSS)gcc)
OBJCOPY  := $(CROSS)objcopy
OBJDUMP  := $(CROSS)objdump
STRIP    := $(CROSS)strip
AR       := $(CROSS)ar
CC       := $(GCC)

#endian-flag  := -EL
endian-flag := $(if $(shell $(CROSS)cpp -EL /dev/null 2>&1 >/dev/null),,-EL)

debug_on_x86 = $(if $(filter mips-%,$(CROSS)),,y)

#----------------------------------------------------
#             System directories
#----------------------------------------------------
CASE_DIR     = $(ROOT_DIR)/$(DEV_CASE_DIR)
SDK_DIR      = $(ROOT_DIR)/$(DEV_SDK_DIR)
IMG_DIR      = $(CASE_DIR)/images
CASE_LIB_DIR = $(CASE_DIR)/lib
SDK_BIN_DIR  = $(SDK_DIR)/bin/$(AM_CHIP_ID_MAIN)
SDK_LIB_DIR  = $(SDK_DIR)/lib/$(AM_CHIP_ID_MAIN)

include $(ROOT_DIR)/$(am-scripts)/case_config.mak

#----------------------------------------------------
#             File names
#----------------------------------------------------
ifeq ($(IMAGE),) # Guess the image name
IMAGE := $(shell basename $$PWD)
endif

default-so-prefix := lib

$(eval $(call __assert_nez, IMAGE))

IMAGE_APP = $(IMAGE).app
IMAGE_BIN = $(IMAGE).bin
IMAGE_SO  = $(default-so-prefix)$(IMAGE).so
IMAGE_AR  = $(default-so-prefix)$(IMAGE).a
IMAGE_LST = $(IMAGE).lst
IMAGE_MAP = $(IMAGE).map

unstripped-elf = $(notdir $(TARGET))-unstripped

DEPEND_FILE ?= .depend

# Let's guess the target path.
ifeq ($(TARGET),)
  ifeq ($(TARGET_TYPE),ar)
	TARGET = $(SDK_LIB_DIR)/$(IMAGE_AR)
  endif
  ifeq ($(TARGET_TYPE),so)
	TARGET = $(SDK_LIB_DIR)/$(IMAGE_SO)
  endif
  ifeq ($(TARGET_TYPE),app)
	TARGET = $(SDK_BIN_DIR)/$(IMAGE_APP)
  endif
  ifeq ($(TARGET_TYPE),bin)
	TARGET = $(SDK_BIN_DIR)/$(IMAGE_BIN)
  endif
endif

#----------------------------------------------------
#         Source and object directories
#----------------------------------------------------
$(sv_src_dir) := $(sort . $($(sv_src_dir)))

ifneq ($(OBJ_DIR),)
obj-dir-prefix  = $(shell echo $(OBJ_DIR)|sed 's:\(\./*\)*::')
obj-dir-prefix := $(call __get_dir_prefix,$(obj-dir-prefix))
endif


#----------------------------------------------------
#         Compiler and linker options
#----------------------------------------------------
inc-dirs   = $(foreach dir,$(INCS) $(dir $<) .,-I$(dir))

OPTIMIZE ?= -Os # -O2
cflags-warning := -Wall -Wno-unused -Wno-implicit -Wno-format -Wno-missing-field-initializers
#cflags-warning := -Wall -Werror -Wno-format -Wno-missing-field-initializers
ifeq ($(debug_on_x86),)
arch-flags := -mips32r2 $(endian-flag) -msoft-float $(CFLAG_LIBC)
else
arch-flags :=
endif

basic-cflags  = $(OPTIMIZE) $(arch-flags) -pipe $(cflags-warning) \
	-fno-builtin-sin -fno-builtin-cos $(DEFS) $(cflags-chipid) \
	$(CFLAGS_EXTRA)
CFLAGS   = $(inc-dirs) $(basic-cflags)
ASFLAGS  = $(CFLAGS) $(ASFLAGS_EXTRA)

map-file  = $(obj-dir-prefix)$(IMAGE_MAP)
list-file = $(obj-dir-prefix)$(IMAGE_LST)

LDFLAGS  = $(arch-flags) #-Wl,-Map,$(map-file)

ifneq ($(strip $(STRIP_UNUSED)),)
CFLAGS  += -ffunction-sections -fdata-sections
ASFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += --gc-sections
endif

run-objdump = $(OBJDUMP) -t -S -j .text -j .data -j .rodata $(output-elf) >$(list-file)

run-strip-elf = $(if $(findstring $(TARGET_TYPE),so app),$(if $(debug_on_x86),\
cp -f $@ $(notdir $(TARGET)),cp -f $@ $(unstripped-elf); $(STRIP) --strip-unneeded $@))

__ls = $(foreach d,$2,$(wildcard $(d)/$1))

define NL
$1

endef

define __set_src
src-$1 = $$(call $2,$$(call __ls,*.$1,$$($(sv_src_dir)))) \
	$$(call $2,$$(filter %.$1,$$($(sv_src_files))))
endef
__filter_out_exclude = $(filter-out $(SRC_EXCLUDED),$1)
$(eval $(foreach i,$(src_suffix),$(call NL,$(call __set_src,$i,__filter_out_exclude))))

src-all = $(foreach i,$(src_suffix),$(src-$i))

__add_obj_dir_prefix = $(if $(obj-dir-prefix),$(foreach i,$1,$(obj-dir-prefix)$i),$1)
define __set_obj
obj-$1 = $$(call __add_obj_dir_prefix,$$(notdir $$(src-$1:.$1=.o)))
endef
$(eval $(foreach i,$(src_suffix),$(call NL,$(call __set_obj,$i))))

obj-all = $(foreach i,$(src_suffix),$(obj-$i))

__dir_to_vpath  = $(foreach i,$(src_suffix),$(call NL,vpath %.$i $1))

$(eval $(foreach f,$($(sv_src_files)),$(call NL,vpath $(notdir $f) $(dir $f))))
$(eval $(foreach d,$($(sv_src_dir)),$(foreach i,$(src_suffix),$(call NL,vpath %.$i $d))))

-include $(ROOT_DIR)/$(am-scripts)/module_config.mak

# This is a bug of GNU make version 3.80
# The eval function can't be put between `ifeq' and `endif'


#----------------------------------------------------
#                Common rules
#----------------------------------------------------
.PHONY: build
.PHONY: .make_obj_dir

__check_suffix = $(if $(findstring $(suffix $2),.$1),,$(error The shared library must have the suffix '$1'))
__check_prefix = $(if $(filter $1%,$(basename $(notdir $2))),,$(error The shared library must have the prefix '$1'))


LIBS    := $(shell env LIBS="$(LIBS)" $(ROOT_DIR)/$(am-scripts)/\$$resolve-libs.sh)
ld-tool := LD
ifeq ($(TARGET_TYPE),ar)
$(call __check_suffix,a,$(TARGET))
$(call __check_prefix,lib,$(TARGET))
run-ld       = rm -f $(output-elf) && $(AR) rcs $(output-elf) $(obj-all)
output-elf   = $(TARGET)
run-objdump  =
ld-tool      = AR
else
run-ld       = $(LD) $(LDFLAGS) -o $(output-elf) $(obj-all) $(LIBS)
output-elf   = $(TARGET)
 ifeq ($(TARGET_TYPE),app)
 CFLAGS += $(if $(debug_on_x86),,-fno-pic -mno-long-calls)
 endif
 ifeq ($(TARGET_TYPE),so)
 $(call __check_suffix,so,$(TARGET))
 $(call __check_prefix,lib,$(TARGET))
 CFLAGS  += -fpic -fvisibility=hidden
 LDFLAGS += -fpic -shared
 endif
 ifeq ($(TARGET_TYPE),bin)
 $(call __assert_nez,LDSCRIPT)
 output-elf = $(unstripped-elf)
 LDFLAGS += -nostartfiles -nodefaultlibs -nostdlib -T $(LDSCRIPT)
 POST_BUILD_CMD := $(strip $(OBJCOPY) -O binary $(output-elf) $(TARGET); $(POST_BUILD_CMD))
 endif
endif

run-cc = $(CC) $(CFLAGS) -c $< -o $@
run-as = $(CC) $(ASFLAGS) -c $< -o $@

build: .make_obj_dir $(TARGET)
$(TARGET) : $(obj-all) $(TARGET_DEP)
	$(call __invoke,$(run-ld),$(ld-tool) $@)
	$(call __invoke,$(run-objdump))
	$(call __invoke,$(run-strip-elf))
	$(call __invoke,$(POST_BUILD_CMD))

mkdir = if [ ! -d '$1' ]; then mkdir '$1' 2>/dev/null; \
	if [ $$? -ne 0 ]; then echo "Can't make directory $1">&2; exit 1; fi; fi
rmdir = if [ -d '$1' ]; then rmdir --ignore-fail-on-non-empty $1 2>/dev/null; fi

define compile-x
$$(obj-$1) : $$(obj-dir-prefix)%.o : %.$1 $$(MAKEFILE_LIST)
	$$(call __invoke,$$(run-cc),CC $$@)
endef
$(eval $(foreach i,$(src_suffix),$(call NL,$(call compile-x,$i))))

.make_obj_dir:
	$(if $(obj-dir-prefix),$(call __invoke,$(call mkdir,$(OBJ_DIR)),MKDIR $(obj-dir-prefix)))

.PHONY: clean depend

clean: $(CLEAN_DEP)
	$(call __invoke,rm -f $(call __add_obj_dir_prefix, *.o *.lst *.map *unstripped)\
	$(if $(obj-dir-prefix),;$(call rmdir,$(obj-dir-prefix))),CLEAN)

mkdep-cmd = $(CC) $(basic-cflags) \
	$(foreach i,$($(sv_src_files)),-I$(dir $i)) $(foreach d,$($(sv_src_dir)),-I$d) $(foreach d,$(INCS),-I$d) \
	-M -MM $(src-all) \
	$(if $(obj-dir-prefix),|gawk '{if($$1~/^.*[.]o:/)print "$(obj-dir-prefix)"$$0;else print $$0;}')

depend:
	$(call __invoke,$(mkdep-cmd)>$(DEPEND_FILE),MKDEP)

.PHONY: distclean

distclean: clean
	@rm -f $(DEPEND_FILE)

#
#  Targets for debug
#
print-%:
	@echo "$($*)"
query-target: target = $(patsubst $(ROOT_DIR)/%,%,$(TARGET))
query-target:
	@echo $(notdir $(target)) $(dir $(target))

ifneq ($(MAKECMDGOALS),clean)
need-depend := y
else
need-depend :=
endif
ifneq ($(NEED_FOR_SPEED),y)
need-depend := y
endif

ifeq  ($(need-depend),y)
ifeq  ($(shell env MKDEP_CMD='$(mkdep-cmd)' DEPFILE=$(DEPEND_FILE) $(ROOT_DIR)/scripts/check_dep.sh),changed)
.PHONY: $(TARGET)
endif
include $(DEPEND_FILE)
endif

.DEFAULT_GOAL = build

