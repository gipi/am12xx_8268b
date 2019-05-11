
#################################################
##   LOAD PHOTO DLL
##   Using GNU gcc Compiler
##   First created by chengjiali 2010-10-28
##
#################################################

### add your directory here

LIB_DIR			= ./lib
OUT_DIR			= ./lib

HEADERS_FLAG	 = -I./
HEADERS_FLAG	+= -I../midware/osapi
HEADERS_FLAG	+= -I../midware/hantro/1211/source/inc
HEADERS_FLAG    += -I./jpeg_baseline_dec_soft
HEADERS_FLAG    += -I../../include

OPTI_FLAG 		= -O3

DEFINE_FLAG		= -DMIPS #-DDEBUG
#-DNDEBUG

#CCFLAG		= -EL -muclibc -fPIC -c -mips32 -Wall $(OPTI_FLAG) $(HEADERS_FLAG) $(DEFINE_FLAG) -msoft-float -fvisibility=hidden
#LDFLAG		= -EL -muclibc -O1 -lpthread -lrt -ldl -shared -msoft-float

ifneq ($(ARCH_CCFLAGS),)
CCFLAG		= $(ARCH_CCFLAGS) -c -mips32r2 -Wall $(OPTI_FLAG) $(HEADERS_FLAG) $(DEFINE_FLAG)
else
CCFLAG		= -EL -muclibc -fPIC -msoft-float -c -mips32 -Wall $(OPTI_FLAG) $(HEADERS_FLAG) $(DEFINE_FLAG) -fvisibility=hidden
endif

ifneq ($(ARCH_LDFLAGS),)
LDFLAG		= $(ARCH_LDFLAGS) -O1 -lpthread -lrt -ldl -shared
else
LDFLAG		= -EL -muclibc -O1 -msoft-float -lpthread -lrt -ldl -shared
endif

GCC			= mips-linux-gnu-gcc
OBJDUMP		= mips-linux-gnu-objdump
OBJCOPY		= mips-linux-gnu-objcopy
OBJSIZE		= mips-linux-gnu-size
AR			= mips-linux-gnu-ar rvs

CC			= $(GCC) $(CCFLAG)
LD			= $(GCC) $(LDFLAG)

LOAD_PHOTO_OBJS	= load_photo.o
LOAD_PHOTO_OBJS += jpg_prog_dec.o
LOAD_PHOTO_OBJS += ./jpeg_baseline_dec_soft/jpeg_dec.o


LOAD_PHOTO_OBJS	+= ../midware/lib/lib_dec_jpeg.a
LOAD_PHOTO_OBJS	+= ../midware/lib/lib_pp_jpeg.a
LOAD_PHOTO_OBJS	+= ../midware/lib/lib_dwl_irq.a
LOAD_PHOTO_OBJS	+= $(LIB_DIR)/libz.a

DEPS	+= Fixed.h
DEPS	+= idct.h
DEPS	+= jpg_prog_dec.h
DEPS	+= load_photo.h
DEPS	+= zconf.h
DEPS	+= zlib.h
DEPS	+= ./jpeg_baseline_dec_soft/idct.h
DEPS	+= ./jpeg_baseline_dec_soft/jpeg_dec.h
DEPS	+= ../midware/hantro/1211/source/inc/basetype.h
DEPS	+= ../midware/hantro/1211/source/inc/dwl.h
DEPS	+= ../midware/hantro/1211/source/inc/jpegdecapi.h
DEPS	+= ../midware/hantro/1211/source/inc/ppapi.h
DEPS	+= ../midware/osapi/osapi.h
DEPS	+= makefile

OUT_ALL		 = $(OUT_DIR)/lib_load_photo.so

all: $(OUT_ALL)
	$(OBJSIZE) $(OUT_DIR)/lib_load_photo.so
	
load_photo.o: load_photo.c $(DEPS)
	$(CC) -o $@ $<
	
jpg_prog_dec.o: jpg_prog_dec.c $(DEPS)
	$(CC) -o $@ $<
	
./jpeg_baseline_dec_soft/jpeg_dec.o: ./jpeg_baseline_dec_soft/jpeg_dec.c $(DEPS)
	$(CC) -o $@ $<


		
### link all obj files 

$(OUT_DIR)/lib_load_photo.so: $(LOAD_PHOTO_OBJS)
	rm -f $(OUT_DIR)/lib_load_photo.so
	$(LD) -o $(OUT_DIR)/lib_load_photo.so $(LOAD_PHOTO_OBJS)
	$(OBJDUMP) $(OUT_DIR)/lib_load_photo.so --disassemble >lib_load_photo_disasm.txt
	
clean:
	rm -f *.o
	rm -f *.txt
	rm -f $(OUT_DIR)/*.so
	rm -f ./gif/*.o
	rm -f ./jpeg_baseline_dec_soft/*.o
