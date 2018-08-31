call ..\config.cmd
call %VS_PATH%\VC\bin\amd64\vcvars64.bat
cl.exe /c main.cpp
link.exe main.obj user32.lib kernel32.lib shell32.lib Advapi32.lib /RELEASE /SUBSYSTEM:WINDOWS /DLL /OUT:../build/gpm64.dll
del /s main.obj
exit