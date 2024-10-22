#!/bin/sh

subdirs="nbspinsert nbspmcast"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
