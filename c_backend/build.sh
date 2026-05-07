#!/bin/bash
echo "Compiling MMS C Backend..."
gcc auth.c    -o auth    -lm
gcc mentor.c  -o mentor  -lm
gcc mentee.c  -o mentee  -lm
gcc manager.c -o manager -lm
echo "Done."
