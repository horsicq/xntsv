call config.cmd

set BUILD_NAME=xntsv_win32

cd %SOURCE_PATH%/dll_source
start /wait _build32.bat
cd %SOURCE_PATH%
cd %SOURCE_PATH%/driver_source
start /wait _build32.bat
cd %SOURCE_PATH%
cd %SOURCE_PATH%/gui_source
start /wait _build32.bat
cd %SOURCE_PATH%
cd %SOURCE_PATH%/sql
start /wait _build.bat
cd %SOURCE_PATH%

mkdir %SOURCE_PATH%\release
rm -f %SOURCE_PATH%\release\%BUILD_NAME%
mkdir %SOURCE_PATH%\release\%BUILD_NAME%
mkdir %SOURCE_PATH%\release\%BUILD_NAME%\platforms
mkdir %SOURCE_PATH%\release\%BUILD_NAME%\sqldrivers

copy %SOURCE_PATH%\build\release\xntsv32.exe %SOURCE_PATH%\release\%BUILD_NAME%\
copy %SOURCE_PATH%\build\gpm32.dll %SOURCE_PATH%\release\%BUILD_NAME%\
copy %SOURCE_PATH%\build\i386\memoryaccess32.sys %SOURCE_PATH%\release\%BUILD_NAME%\
xcopy %SOURCE_PATH%\build\db %SOURCE_PATH%\release\%BUILD_NAME%\db /E /I

copy %QT_PATH32%\bin\Qt5Widgets.dll %SOURCE_PATH%\release\%BUILD_NAME%\
copy %QT_PATH32%\bin\Qt5Gui.dll %SOURCE_PATH%\release\%BUILD_NAME%\
copy %QT_PATH32%\bin\Qt5Core.dll %SOURCE_PATH%\release\%BUILD_NAME%\
copy %QT_PATH32%\bin\Qt5Sql.dll %SOURCE_PATH%\release\%BUILD_NAME%\
copy %QT_PATH32%\plugins\platforms\qwindows.dll %SOURCE_PATH%\release\%BUILD_NAME%\platforms\
copy %QT_PATH32%\plugins\sqldrivers\qsqlite.dll %SOURCE_PATH%\release\%BUILD_NAME%\sqldrivers\
copy %VS_PATH%\VC\Redist\MSVC\14.16.27012\x86\Microsoft.VC141.CRT\msvcp140.dll %SOURCE_PATH%\release\%BUILD_NAME%\
copy %VS_PATH%\VC\Redist\MSVC\14.16.27012\x86\Microsoft.VC141.CRT\vcruntime140.dll %SOURCE_PATH%\release\%BUILD_NAME%\

cd %SOURCE_PATH%\release
if exist %BUILD_NAME%_%XNTSV_VERSION%.zip del %BUILD_NAME%_%XNTSV_VERSION%.zip
%SEVENZIP_PATH%\7z.exe a %BUILD_NAME%_%XNTSV_VERSION%.zip %BUILD_NAME%\*
rmdir /s /q %SOURCE_PATH%\release\%BUILD_NAME%\
cd ..