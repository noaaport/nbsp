#!/bin/sh

. ./configure.inc

sed \
    -e "/@include@/ s||$INCLUDE|" \
    -e "/@q@/ s||$Q|g" \
    -e "/@INSTALL@/ s||$INSTALL|" \
    -e "/@TCLSH@/ s||$TCLSH|" \
    -e "/@RCINIT@/s||$RCINIT|" \
    -e "/@RCFPATH@/s||$RCFPATH|" \
    -e "/@RCCONF@/s||$RCCONF|" \
    -e "/@SYSTEMDCONF@/s||$SYSTEMDCONF|" \
    -e "/@CLEANUP@/s||$CLEANUP|" \
    -e "/@HOURLYCONF@/s||$HOURLYCONF|" \
    -e "/@STARTCLEANCONF@/s||$STARTCLEANCONF|" \
    -e "/@STARTSTOPRC@/s||$STARTSTOPRC|" \
    -e "/@POSTINSTALLSRC@/s||$POSTINSTALLSRC|" \
    Makefile.in > Makefile
