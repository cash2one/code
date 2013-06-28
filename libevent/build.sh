#!/bin/sh
gcc socket_libevent.c -o socket_libevent -Wl,-rpath,/usr/local/lib/ -L/usr/local/lib/ -levent -I/usr/local/include/
