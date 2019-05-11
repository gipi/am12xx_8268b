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
COPY .\scripts\*.sh %ROOT_DIR%\case\scripts\1211\





