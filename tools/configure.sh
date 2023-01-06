#!/bin/sh

subdirs="nbspinsert"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
