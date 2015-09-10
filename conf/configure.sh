#!/bin/sh

. ./configure.inc

sed -e "/@include@/ s||$INCLUDE|" \
    -e "/@q@/ s||$Q|g" \
    -e "/@INSTALL@/ s||$INSTALL|" \
    -e "/@NBSPDDEFAULTS@/s||$NBSPDDEFAULTS|" \
    -e "/@SYSCTLDEFAULTS@/s||$SYSCTLDEFAULTS|" \
    -e "/@SYSCTLCONFLOCAL@/s||$SYSCTLCONFLOCAL|" \
    -e "/@UDPRECVSIZEDEFAULTS@/s||$UDPRECVSIZEDEFAULTS|" \
    -e "/@NBSPCONFIGURE_UPDATEDB@/s||$NBSPCONFIGURE_UPDATEDB|" \
    -e "/@TCLSH@/ s||$TCLSH|" \
    Makefile.in > Makefile
