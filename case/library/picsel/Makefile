#1601 Quantum Plaza, No.27 Zhichun Road, Haidian District, Beijing, Actions MicroElectronics Co., Ltd.

ROOT_DIR  = ./../../..
include $(ROOT_DIR)/scripts/config.mak

TARGET_TYPE = so

build:	
	make -C linuxgtk-alien -f Makefile_libfv
	cp ./linuxgtk-alien/libfv.so $(ROOT_DIR)/case/lib/libfv.so
	cp ./linuxgtk-alien/fileviewer.h $(ROOT_DIR)/case/include
	cp ./linuxgtk-alien/fileviewer.cfg $(ROOT_DIR)/case/data/
	make -C linuxgtk-alien -f Makefile_libfv clean
					
clean:
	make -C linuxgtk-alien -f Makefile_libfv clean

	

depend:

query-target:
	@echo libfv.so case/lib
	


