call ..\config.cmd
set CUR_DIR=%~dp0
call %DDK_PATH%\bin\setenv.bat %DDK_PATH% fre x64 WIN7
cd %CUR_DIR%
cd win64
mkdir ..\..\build
build
RMDIR /S /Q objfre_win7_amd64
DEL buildfre_win7_amd64.log
cd ..
exit