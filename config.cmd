set DDK_PATH=C:\WinDDK\7600.16385.1
set VS_PATH="C:\Program Files (x86)\Microsoft Visual Studio 12.0"
set SQLITE_PATH="C:\Utils\sqlite-tools-win32-x86-3240000"
set SEVENZIP_PATH="C:\Program Files\7-Zip"
set QT_PATH32="C:\Qt\5.6.3\msvc2013"
set QT_PATH64="C:\Qt\5.6.3\msvc2013_64"

set SOURCE_PATH=%~dp0
mkdir %SOURCE_PATH%\build
mkdir %SOURCE_PATH%\build\db
mkdir %SOURCE_PATH%\release
set /p XNTSV_VERSION=<release_version.txt