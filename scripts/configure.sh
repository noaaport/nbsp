#!/bin/sh

. configure.inc

sed \
    -e "/@include@/ s||$INCLUDE|" \
    -e "/@q@/ s||$Q|g" \
    -e "/@INSTALL@/ s||$INSTALL|" \
    -e "/@TCLSH@/ s||$TCLSH|" \
    -e "/@NBSPDSH@/s||$NBSPDSH|" \
    -e "/@HOURLYCONF@/s||$HOURLYCONF|" \
    -e "/@STARTCLEANCONF@/s||$STARTCLEANCONF|" \
    -e "/@STARTSTOPRC@/s||$STARTSTOPRC|" \
    Makefile.in > Makefile
