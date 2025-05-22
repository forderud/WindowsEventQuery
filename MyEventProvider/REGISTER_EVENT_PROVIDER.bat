echo Must be run from an administrative developer command prompt
pause

echo Compiling message text file...
mc.exe -U MyEventProvider.mc
rc.exe MyEventProvider.rc
link.exe -dll -noentry MyEventProvider.res

echo Please update the DLL file paths in MyEventProvider.reg before proceeding...
pause

reg import MyEventProvider.reg

echo [done]
