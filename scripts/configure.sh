#!/bin/sh

. configure.inc

sed \
    -e "/@include@/ s||$INCLUDE|" \
    -e "/@q@/ s||$Q|g" \
    -e "/@INSTALL@/ s||$INSTALL|" \
    -e "/@TCLSH@/ s||$TCLSH|" \
    -e "/@NBSPDSH@/s||$NBSPDSH|" \
    -e "/@NBSPCLEANUP@/s||$NBSPCLEANUP|" \
    -e "/@HOURLYCONF@/s||$HOURLYCONF|" \
    -e "/@STARTCLEANCONF@/s||$STARTCLEANCONF|" \
    -e "/@STARTSTOPRC@/s||$STARTSTOPRC|" \
    -e "/@NBSPD_RC_FPATH@/s||$NBSPD_RC_FPATH|" \
    Makefile.in > Makefile
