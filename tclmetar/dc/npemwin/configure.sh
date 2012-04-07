#!/bin/sh

. ../../configure.inc

PROGNAME=npemwinmtrd

savedir=`pwd`
cd ../..
./configure.sh
cd $savedir

sed -e "/@include@/s||$INCLUDE|" \
    -e "/@q@/s||$Q|g" \
    -e "/@INSTALL@/s||$INSTALL|" \
    -e "/@TCLSH@/s||$TCLSH|" \
    -e "/@PROGNAME@/s||$PROGNAME|" Makefile.in > Makefile
