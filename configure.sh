#!/bin/sh

. ./configure.inc

config_dirs="conf src utils scripts doc filters \
    tclhttpd \
    tclgrads/ext/nbsp \
    tclgempak/ext/nbsp \
    tclmetar \
    tclssh/ext/nbsp \
    tclupperair/dc/nbsp"

configure_default
configure_default Makefile.inc

savedir=`pwd`
for d in $config_dirs
do
    cd $d
    ./configure.sh
    cd $savedir
done
