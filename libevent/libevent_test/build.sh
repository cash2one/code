#!/bin/sh
gcc libevent.c -o libevent.o -Wl,-rpath,/usr/local/lib/ -L/usr/local/lib/ -levent -I/usr/local/include/
