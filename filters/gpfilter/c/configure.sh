#!/bin/sh

. ./configure.inc

sed \
    -e "/@include@/ s||$INCLUDE|" \
    -e "/@q@/ s||$Q|g" \
    -e "/@INSTALL@/ s||$INSTALL|" \
    -e "/@CC@/ s||$CC|" \
    -e "/@CCWFLAGS@/ s||$CCWFLAGS|" \
    -e "/@SUFFIXRULES@/ s||$SUFFIXRULES|" \
    -e "/@INCDIRS@/s%%$INCDIRS%" \
    -e "/@LIBDIRS@/s||$LIBDIRS|" \
    -e "/@LIBS@/s||$LIBS|" \
    Makefile.in > Makefile
