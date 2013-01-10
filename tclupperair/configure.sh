#!/bin/sh

. ../configure.inc

PROGNAME1=nbspfm35d
PROGNAME2=nbspuatocsv

savedir=`pwd`
cd tclupperair
./configure.sh
cd $savedir

sed -e "/@include@/s||$INCLUDE|" \
    -e "/@q@/s||$Q|g" \
    -e "/@INSTALL@/s||$INSTALL|" \
    -e "/@TCLSH@/s||$TCLSH|" \
    -e "/@PROGNAME1@/s||$PROGNAME1|" \
    -e "/@PROGNAME2@/s||$PROGNAME2|" Makefile.in > Makefile
