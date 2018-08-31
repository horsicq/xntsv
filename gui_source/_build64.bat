call ..\config.cmd
call %VS_PATH%\VC\bin\amd64\vcvars64.bat
%QT_PATH64%\bin\qmake.exe gui_source.pro -r -spec win32-msvc2013 "CONFIG+=release"
nmake Makefile.Release clean
nmake

del Makefile
del Makefile.Release
del Makefile.Debug

exit