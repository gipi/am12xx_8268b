CFLAGS=-g -Wall
CC=gcc
AR=ar
RANLIB=ranlib
LIBS:=-L./ -lcgic  \
	-L../../../case/lib  -lamsocket
INCS:=-I../../../case/library/socket
CFLAGS+=${INCS}
all: libcgic.a capture conference_control.cgi upload2.cgi upload3.cgi upload4.cgi upload1.cgi upload.cgi get_my_mac.cgi get_my_ip.cgi pushdongleinfo.cgi htmlsetting_snapshot.cgi password_POST.cgi wifi_info_GET.cgi set_wifi_POST.cgi set_hotspot_ap.cgi delete_password_POST.cgi add_network_POST.cgi set_lan_POST.cgi set_resolution_POST.cgi set_defaultmode_POST.cgi dir.cgi set_EQ.cgi set_music_output_POST.cgi rs232cmd.cgi set_router_ctl_POST.cgi factory_test.cgi windir.cgi set_devicename.cgi set_devicename_reboot.cgi set_autoplay.cgi user_password_post.cgi user_get_cookie.cgi websetting.cgi get_jpg.cgi


release: CC=mips-linux-gnu-gcc -EL
release: AR=mips-linux-gnu-ar
release: RANLIB=mips-linux-gnu-ranlib
release: all
	cp -f *.cgi ../../../sdk/user1/thttpd/html/cgi-bin

install: libcgic.a
	cp libcgic.a /usr/local/lib
	cp cgic.h /usr/local/include
	@echo libcgic.a is in /usr/local/lib. cgic.h is in /usr/local/include.

libcgic.a: cgic.o cgic.h htmlsetting_cgimain.o htmlsetting_cgi.h 
	rm -f libcgic.a
	$(AR) rc libcgic.a cgic.o htmlsetting_cgimain.o 
	$(RANLIB) libcgic.a

#mingw32 and cygwin users: replace .cgi with .exe
upload.cgi: upload.o libcgic.a
	$(CC) upload.o -o upload.cgi ${LIBS}
upload1.cgi: upload1.o libcgic.a
	$(CC) upload1.o -o upload1.cgi ${LIBS}
upload2.cgi: upload2.o libcgic.a
	$(CC) upload2.o -o upload2.cgi ${LIBS}
upload3.cgi: upload3.o libcgic.a
	$(CC) upload3.o -o upload3.cgi ${LIBS}
upload4.cgi: upload4.o libcgic.a
	$(CC) upload4.o -o upload4.cgi ${LIBS}
set_EQ.cgi: set_EQ.o libcgic.a
	$(CC) set_EQ.o -o set_EQ.cgi ${LIBS}

set_music_output_POST.cgi: set_music_output_POST.o libcgic.a
	$(CC) set_music_output_POST.o -o set_music_output_POST.cgi ${LIBS}
	
websetting.cgi: websetting.o libcgic.a
	$(CC) websetting.o -o websetting.cgi ${LIBS}

get_my_mac.cgi: get_my_mac.o libcgic.a
	$(CC) get_my_mac.o -o get_my_mac.cgi ${LIBS}

get_my_ip.cgi: get_my_ip.o libcgic.a
	$(CC) get_my_ip.o -o get_my_ip.cgi ${LIBS}
	
pushdongleinfo.cgi: pushdongleinfo.o libcgic.a
	$(CC) pushdongleinfo.o -o pushdongleinfo.cgi ${LIBS}
	
password_POST.cgi: password_POST.o libcgic.a
	$(CC) password_POST.o -o password_POST.cgi ${LIBS}

wifi_info_GET.cgi: wifi_info_GET.o libcgic.a
	$(CC) wifi_info_GET.o -o wifi_info_GET.cgi ${LIBS}

set_wifi_POST.cgi: set_wifi_POST.o libcgic.a
	$(CC) set_wifi_POST.o -o set_wifi_POST.cgi ${LIBS}
	
set_hotspot_ap.cgi: set_hotspot_ap.o libcgic.a
	$(CC) set_hotspot_ap.o -o set_hotspot_ap.cgi ${LIBS}

set_resolution_POST.cgi: set_resolution_POST.o libcgic.a
	$(CC) set_resolution_POST.o -o set_resolution_POST.cgi ${LIBS}

delete_password_POST.cgi: delete_password_POST.o libcgic.a
	$(CC) delete_password_POST.o -o delete_password_POST.cgi ${LIBS}

add_network_POST.cgi: add_network_POST.o libcgic.a
	$(CC) add_network_POST.o -o add_network_POST.cgi ${LIBS}
	
htmlsetting_snapshot.cgi: htmlsetting_snapshot.o libcgic.a
	$(CC) htmlsetting_snapshot.o -o htmlsetting_snapshot.cgi ${LIBS}

conference_control.cgi: conference_control.o libcgic.a
	$(CC) conference_control.o -o conference_control.cgi ${LIBS}

set_lan_POST.cgi: set_lan_POST.o libcgic.a
	$(CC) set_lan_POST.o -o set_lan_POST.cgi ${LIBS}

set_defaultmode_POST.cgi: set_defaultmode_POST.o libcgic.a
	$(CC) set_defaultmode_POST.o -o set_defaultmode_POST.cgi ${LIBS}
set_devicename.cgi: set_devicename.o libcgic.a
	$(CC) set_devicename.o -o set_devicename.cgi ${LIBS} 
set_autoplay.cgi: set_autoplay.o libcgic.a
	$(CC) set_autoplay.o -o set_autoplay.cgi ${LIBS}    

user_password_post.cgi: user_password_post.o libcgic.a
	$(CC) user_password_post.o -o user_password_post.cgi ${LIBS}   

user_get_cookie.cgi: user_get_cookie.o libcgic.a
	$(CC) user_get_cookie.o -o user_get_cookie.cgi ${LIBS}
	
set_devicename_reboot.cgi: set_devicename_reboot.o libcgic.a
	$(CC) set_devicename_reboot.o -o set_devicename_reboot.cgi ${LIBS} 
	
cgictest.cgi: cgictest.o libcgic.a
	$(CC) cgictest.o -o cgictest.cgi ${LIBS}
dir.cgi: dir.o libcgic.a
	$(CC) dir.o -o dir.cgi ${LIBS}
windir.cgi: windir.o libcgic.a
	$(CC) windir.o -o windir.cgi ${LIBS}
factory_test.cgi: factory_test.o libcgic.a
	$(CC) factory_test.o -o factory_test.cgi ${LIBS}	
capture: capture.o libcgic.a
	$(CC) capture.o -o capture ${LIBS}
rs232cmd.cgi: rs232cmd.o libcgic.a
	$(CC) rs232cmd.o -o rs232cmd.cgi ${LIBS}

set_router_ctl_POST.cgi: set_router_ctl_POST.o libcgic.a
	$(CC) set_router_ctl_POST.o -o set_router_ctl_POST.cgi ${LIBS}		

get_jpg.cgi: get_jpg.o libcgic.a
	$(CC) get_jpg.o -o get_jpg.cgi ${LIBS}

clean:
	rm -f *.o *.a *.cgi capture
