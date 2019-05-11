#**************************************************************************
#                                                                         *
#   PROJECT     : MIPS port for LINUX                                     *
#   MODULE      : MakeFile                                                *
#   AUTHOR      : Pan Ruochen (reachpan@actions-micro.com)                *
#   PROCESSOR   : MIPS 4KEc (32 bit RISC) - ActionsMicro 7XXX Family      *
#   Tool-chain  : CodeSourcery_G++_Lite                                   *
#                                                                         *
#   DESCRIPTION :                                                         *
#   The top-level Makefile for ActionsMicro Linux SDK.                    *
#                                                                         *
#   USAGE       :                                                         *
#   make [z=cbd] targets...                                               *
#                                                                         *
#   The targets can be these phony targets:                               *
#   clean:  Clean ELF image files and immediate object files              *
#           for all modules.                                              *
#   depend: Create dependency files for all modules.                      *
#   build:  Create ELF image files for all modules.                       *
#   ++++++++                                                              *
#   The targets can be system modules or predefined aliases:              *
#   MOD:    Do actions to the very specified module named MOD.            *
#   ALIAS:  Do actions to the very modules represented by the alias       *
#           named ALIAS.                                                  *
#   .sdk:   All modules related to SDK.                                   *
#   .case:  All modules related to CASE.                                  *
#   The environment `z' Specify the actions to be taken.                  *
#   The meaningful values are:                                            *
#     - b : build (This is the default action to be taken)                *
#     - c : clean                                                         *
#     - d : depend                                                        *
#                                                                         *
#   It is NOT recommanded that the phony targets are mixed with           *
#   the module targets.                                                   *
#                                                                         *
#   History:                                                              *
#   1.  The first version. (2009/12/24)                                   *
#**************************************************************************

define NL
$1

endef

sys-goals     = clean depend build all
normal-goals  = $(filter-out $(sys-goals),$(MAKECMDGOALS))
special-goals = $(shell echo $z | awk 'BEGIN{FS=""}{for(i=1;i<=NF;i++)a[$$i]++}\
END{if(a["c"])s="clean ";if(a["d"])s=s"depend ";if(a["b"])s=s"build";if(s)print s;else print "build"}')
include config.mak

.PHONY: $(sys-goals)

ifeq ($(normal-goals),)
dirs = $(shell gawk '$$3{print $$3}' $(CFG_MODS_SDK) $(CFG_MODS_CASE))
__get_cmds = $(foreach d,$(dirs),$(call NL,@echo '  ENTER $d'&&$(MAKE) -C $(TOPDIR)/$d $1&&echo))
build: ; $(call __get_cmds,$@)
clean: ; $(call __get_cmds,$@)
depend:; $(call __get_cmds,$@)
all: .linux .busybox clean depend build .rtsp
else
build:
clean:
depend:
MAKE_ACTION = $(special-goals)
include .Rules
ifneq ($(wildcard $(CFG_ALIAS)),)
alias-names = $(shell /bin/bash \$$resolve-alias.sh $(CFG_ALIAS))
 include $(CFG_ALIAS)
$(eval $(foreach a,$(alias-names),$(call NL,$a: $($a))))
endif
endif

.PHONY: rmtmp
rmtmp: ; @rm -f AM_sdk.cfg AM_case.cfg config.mak case_config.mak .Rules
.PHONY: .linux
.linux:      ; @make -C $(TOPDIR)/sdk/linux -j4
.PHONY: .busybox
.busybox: 
	@make -C $(TOPDIR)/sdk/busybox -j4
	@make -C $(TOPDIR)/sdk/busybox install

.PHONY: .menuconfig
.menuconfig:      ; @make -C $(TOPDIR)/sdk/linux menuconfig

.PHONY: .rtsp
.rtsp:    ; @make -C $(TOPDIR)/case/apps/miracast

