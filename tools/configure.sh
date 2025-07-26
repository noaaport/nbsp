#!/bin/sh

subdirs="nbspinsert nbspmcast nbspcraftinsert"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
