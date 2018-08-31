call ..\config.cmd
set CUR_DIR=%~dp0
call %DDK_PATH%\bin\setenv.bat %DDK_PATH% fre x86 WXP
cd %CUR_DIR%
cd win32 
build
RMDIR /S /Q objfre_wxp_x86
DEL buildfre_wxp_x86.log
cd ..
exit
