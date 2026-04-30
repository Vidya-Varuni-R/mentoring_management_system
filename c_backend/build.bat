@echo off
echo Compiling MMS C Backend...
gcc auth.c    -o auth.exe    -lm
gcc mentor.c  -o mentor.exe  -lm
gcc mentee.c  -o mentee.exe  -lm
gcc manager.c -o manager.exe -lm
echo Done.
