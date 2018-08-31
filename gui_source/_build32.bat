call ..\config.cmd
call %VS_PATH%\VC\bin\vcvars32.bat
%QT_PATH32%\bin\qmake.exe gui_source.pro -r -spec win32-msvc2013 "CONFIG+=release"
nmake Makefile.Release clean
nmake

del Makefile
del Makefile.Release
del Makefile.Debug

exit