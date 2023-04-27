#!/bin/bash
i=2
rm *.o
gcc -g -Wall -c dlist.c &>> gcc.log
gcc -g -Wall -c dccthread.c &>> gcc.log
gcc -g -Wall -I. tests/test$i.c dccthread.o dlist.o -o test$i -lrt
./test$i