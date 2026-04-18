@echo off
cd /d "%~dp0"
gcc -Wall -O2 auth.c    -o auth.exe    || goto :err
gcc -Wall -O2 mentor.c  -o mentor.exe  || goto :err
gcc -Wall -O2 mentee.c  -o mentee.exe  || goto :err
gcc -Wall -O2 manager.c -o manager.exe || goto :err
echo Built: auth.exe mentor.exe mentee.exe manager.exe
exit /b 0
:err
echo Build failed.
exit /b 1
