#!/bin/sh

os=`uname`

# default is freebsd
template="Makefile.freebsd"

[ $os = "Linux" ] && template="Makefile.linux"

cp $template Makefile

