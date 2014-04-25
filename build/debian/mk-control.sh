#!/bin/sh

. ../../configure.inc

# The defaults are for debian
controlfile="control.debian"

if [ $ubuntu = "14.04" ]
then
    controlfile="control.ubuntu"
fi

cp $controlfile control
