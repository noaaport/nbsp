#!/bin/sh

. ../configure.inc

config_dirs="extensions"

configure_default

savedir=`pwd`
for d in $config_dirs
do
    cd $d
    ./configure.sh
    cd $savedir
done
