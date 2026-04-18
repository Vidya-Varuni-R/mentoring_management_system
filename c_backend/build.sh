#!/usr/bin/env bash
# Compile all four C backend modules.
# Usage:  ./build.sh        (Linux/macOS)
# Windows: run each line in MinGW / WSL, or use build.bat.
set -e
cd "$(dirname "$0")"
gcc -Wall -O2 auth.c    -o auth
gcc -Wall -O2 mentor.c  -o mentor
gcc -Wall -O2 mentee.c  -o mentee
gcc -Wall -O2 manager.c -o manager
echo "Built: auth mentor mentee manager"
