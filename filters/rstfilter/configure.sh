#!/bin/sh

subdirs="tcl satmap radmap"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
