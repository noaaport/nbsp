#!/bin/sh

os=`uname`

# default is freebsd
template="Makefile.freebsd"

[ $os = "Linux" ] && template="Makefile.linux"

cp $template Makefile

for f in mcast.c mcast.h
do
    cp ../mcast-llvm/${f} .
done

