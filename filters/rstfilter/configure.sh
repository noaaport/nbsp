#!/bin/sh

subdirs="tcl satmap radmap radmos"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
