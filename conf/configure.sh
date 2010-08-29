#!/bin/sh

. ./configure.inc

sed -e "/@include@/ s||$INCLUDE|" \
    -e "/@q@/ s||$Q|g" \
    -e "/@INSTALL@/ s||$INSTALL|" \
    -e "/@NBSPDDEFAULTS@/s||$NBSPDDEFAULTS|" \
    Makefile.in > Makefile
