#!/bin/sh

subdirs="nbspqdump nbspinvrm nbspexec"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
