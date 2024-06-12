#!/bin/bash
clear
# Compile the C code
gcc client.c -o compiled/client socket_utilities.c

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable created. Running file now"
else
    echo "Compilation failed"
fi
echo ""
echo ""
cd compiled
./client