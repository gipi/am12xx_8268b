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
COPY .\config.linux %ROOT_DIR%\scripts\cfg\1213\
COPY .\config.linux.8251 %ROOT_DIR%\scripts\cfg\1213\
COPY .\udhcpd_ezcastpro.conf %ROOT_DIR%\case\images\ezcast\
COPY .\udhcpd_ezcastpro_nodns_01.conf %ROOT_DIR%\case\images\ezcast\
COPY .\fwimage_1213_ezcastpro.cfg %ROOT_DIR%\case\images\
COPY .\manual.conf %ROOT_DIR%\sdk\user1\lan\
COPY .\bootarg.txt %ROOT_DIR%\case\images\ezcast\
COPY .\bootarg.txt %ROOT_DIR%\sdk\bin\1213\
COPY .\uartcom.h %ROOT_DIR%\sdk\include\
COPY .\customer_ssid.h %ROOT_DIR%\case\include\
COPY .\Kconfig %ROOT_DIR%\sdk\linux\drivers\
COPY .\Makefile %ROOT_DIR%\sdk\linux\drivers\
COPY .\major.h %ROOT_DIR%\sdk\linux\include\linux\
COPY .\dongle.png %ROOT_DIR%\sdk\user1\thttpd\html\img\
XCOPY .\uartcom %ROOT_DIR%\sdk\linux\drivers\uartcom /I
