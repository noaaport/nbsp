#!/bin/sh

config_dirs="exporter weatherscope npstats catalog inbsp"

for d in $config_dirs
do
    cd $d
    ./configure.sh
    cd ..
done
