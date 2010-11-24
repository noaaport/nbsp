#!/bin/sh

. ./configure.inc

configure_default () {

  makefile_out=Makefile
  [ $# -eq 1 ] && makefile_out=$1
  makefile_in=${makefile_out}.in

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
      ${makefile_in} > ${makefile_out}
}

  configure_default Makefile
  configure_default dcnids/Makefile
  configure_default dcgini/Makefile
  configure_default dcgini-new/Makefile
