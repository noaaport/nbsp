#!/bin/sh

subdirs="tcl satmap satgc radmap"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
