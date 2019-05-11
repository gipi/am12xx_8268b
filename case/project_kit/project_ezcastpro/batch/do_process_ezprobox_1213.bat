@echo off

rem /*************************************************************
rem * This is the batch file for doing work like copy scripts etc. 
rem * When export files ends,the linux tool will call this batch 
rem *    to finish the remaining work.
rem **************************************************************/


SET PROJECT_DIR=..
SET ROOT_DIR=%PROJECT_DIR%\..\..\..


rem /*************************************************************
rem * copy scripts to the related directory. 
rem **************************************************************/
COPY %PROJECT_DIR%\..\project_qc\batch\scripts\1213\*.sh %ROOT_DIR%\case\scripts\1213\
COPY .\*.png %ROOT_DIR%\sdk\user1\ezdata\
COPY .\welcome.bin %ROOT_DIR%\case\images\ezcast\
COPY .\welcome.bin %ROOT_DIR%\case\images\
COPY .\config.projector %ROOT_DIR%\scripts\cfg\1213\config.linux
COPY .\config.linux.8251 %ROOT_DIR%\scripts\cfg\1213\
COPY .\udhcpd_ezcastpro.conf %ROOT_DIR%\case\images\ezcast\
COPY .\udhcpd_ezcastpro_nodns_01.conf %ROOT_DIR%\case\images\ezcast\
COPY .\fwimage_1213_ezcastpro.cfg %ROOT_DIR%\case\images\
COPY .\manual.conf %ROOT_DIR%\sdk\user1\lan\
COPY .\bootarg.txt %ROOT_DIR%\case\images\ezcast\
COPY .\bootarg.txt %ROOT_DIR%\sdk\bin\1213\
COPY .\uartcom.h %ROOT_DIR%\sdk\include\
COPY .\customer_ssid.h %ROOT_DIR%\case\include\
COPY .\Kconfig_projector %ROOT_DIR%\sdk\linux\drivers\Kconfig
COPY .\Makefile_projector %ROOT_DIR%\sdk\linux\drivers\Makefile
COPY .\major.h %ROOT_DIR%\sdk\linux\include\linux\
COPY .\EZ_USB.iso %ROOT_DIR%\sdk\cdrom\
COPY .\ez_remoteconfig.h %ROOT_DIR%\sdk\library\wifi_subdisplay\
COPY .\dongle.png %ROOT_DIR%\sdk\user1\thttpd\html\img\
COPY .\etc\user.sh %ROOT_DIR%\sdk\rootfs\etc\
COPY .\etc\hosts %ROOT_DIR%\sdk\rootfs\etc\
COPY .\etc\thttpd.conf %ROOT_DIR%\sdk\rootfs\etc\
COPY .\sbin\atop_prod_ap %ROOT_DIR%\sdk\rootfs\sbin\
COPY .\sbin\check_DHCP_eth %ROOT_DIR%\sdk\rootfs\sbin\
COPY .\sbin\gwd %ROOT_DIR%\sdk\rootfs\sbin\
COPY .\sbin\set_eth %ROOT_DIR%\sdk\rootfs\sbin\
if not exist %ROOT_DIR%\sdk\linux\drivers\uartcom\uartcom.c (
XCOPY .\uartcom %ROOT_DIR%\sdk\linux\drivers\uartcom /I
)else echo already copy uartcom
if not exist %ROOT_DIR%\sdk\linux\drivers\am_pipe\am_pipe.c (
XCOPY .\am_pipe %ROOT_DIR%\sdk\linux\drivers\am_pipe /I
XCOPY .\atop %ROOT_DIR%\sdk\rootfs\mnt\atop /I /S
)else echo already copy am_pipe and atop
COPY .\thttpd.conf %ROOT_DIR%\sdk\rootfs\etc\






