#!/bin/sh

subdirs="tcl"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
