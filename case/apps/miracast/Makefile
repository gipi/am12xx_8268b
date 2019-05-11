# @file Makefile
#
# Makefile for RTSP sink module
# 
# INTEL CONFIDENTIAL
# Copyright 2010-2012 Intel Corporation All Rights Reserved.
#
# The source code contained or described herein and all documents related to 
# the source code ("Material") are owned by Intel Corporation or its 
# suppliers or licensors.  Title to the Material remains with Intel 
# Corporation or its suppliers and licensors.  The Material contains trade 
# secrets and proprietary and confidential information of Intel or its 
# suppliers and licensors.  The Material is protected by worldwide copyright 
# and trade secret laws and treaty provisions. No part of the Material may 
# be used, copied, reproduced, modified, published, uploaded, posted, 
# transmitted, distributed, or disclosed in any way without Intel's prior 
# express written permission.
# 
# No license under any patent, copyright, trade secret or other intellectual 
# property right is granted to or conferred upon you by disclosure or 
# delivery of the Materials,  either expressly, by implication, inducement, 
# estoppel or otherwise.  Any license under such intellectual property 
# rights must be express and approved by Intel in writing.

LIB = librtspsink.a
LIB_OBJ = librtspsink.o sha2.o affinity.o rtsp_abstraction.o

RTSP_LIB = librtsprds.a
RTSP_LIB_OBJ = $(patsubst %.c,%.o,$(wildcard rtsp/*.c))

RTSPWFD_LIB = librtspwfd.a
RTSPWFD_LIB_OBJ = $(patsubst %.c,%.o,$(wildcard rtspwfd/*.c))

ALL_LIB_OBJS = $(LIB_OBJ) $(RTSP_LIB_OBJ) $(RTSPWFD_LIB_OBJ)
ALL_LIBS = $(LIB) $(RTSP_LIB) $(RTSPWFD_LIB)

EXE = sample_rtsp_sink
EXE_OBJ = sample_rtsp_sink.o

CPPFLAGS += -DLOGGER_PROVIDED
LDLIBS += -lpthread

ifeq ($(TEST),1)
	CPPFLAGS += -DNDEBUG
	CFLAGS += --coverage
	LDFLAGS += --coverage
endif

#----------------------------------------------------
#             GNU Toolchains
#----------------------------------------------------
CROSS    ?= mips-linux-gnu-
GCC      := $(CROSS)gcc -EL
LD        = $(if $(filter $(foreach i,$(cpp_suffix),%.$i),$(src-all)),$(CROSS)g++,$(CROSS)gcc)
OBJCOPY  := $(CROSS)objcopy
OBJDUMP  := $(CROSS)objdump
STRIP    := $(CROSS)strip
AR       := $(CROSS)ar
CC       := $(GCC)

# =====

all: clear $(ALL_LIBS) $(EXE)
	cp -dprf  $(EXE) ../../images/miracast.app

$(LIB): $(LIB)($(LIB_OBJ))

$(RTSP_LIB): $(RTSP_LIB)($(RTSP_LIB_OBJ))

$(RTSPWFD_LIB): $(RTSPWFD_LIB)($(RTSPWFD_LIB_OBJ))

$(EXE): $(EXE_OBJ) $(ALL_LIBS)

# This target must be built with "make TEST=1 clean test"
.PHONY: test clear
test: $(EXE)_test.o $(ALL_LIBS)
	rm -f test
	$(CC) $(LDFLAGS) $(LDFLAGS2) -o $@ $^ $(LDLIBS) $(LDLIBS2) -lcheck
	./test
	
clear:
	rm -rf $(ALL_LIBS) $(ALL_LIB_OBJS) $(EXE) $(EXE_OBJ)

coverage:
	gcov librtspsink.c

clean:
	rm -rf $(ALL_LIBS) $(ALL_LIB_OBJS) $(EXE) $(EXE_OBJ)
