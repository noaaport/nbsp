#!/bin/sh

. ../../configure.inc

# The defaults are for debian
controlfile="control.debian"

if [ $ubuntu != "0" ]
then
    controlfile="control.ubuntu"
fi

cp $controlfile control
