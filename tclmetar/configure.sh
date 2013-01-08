#!/bin/sh

. ../configure.inc

PROGNAME=nbspmtrd

savedir=`pwd`
cd tclmetar
./configure.sh
cd $savedir

sed -e "/@include@/s||$INCLUDE|" \
    -e "/@q@/s||$Q|g" \
    -e "/@INSTALL@/s||$INSTALL|" \
    -e "/@TCLSH@/s||$TCLSH|" \
    -e "/@PROGNAME@/s||$PROGNAME|" Makefile.in > Makefile
