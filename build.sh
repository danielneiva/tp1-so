#!/bin/bash
rm *.o
gcc list.c -o list.o -c
gcc dccthread.c -o dccthread.o -c
gcc -o prog list.o dccthread.o
./prog