#!/bin/sh

. ./configure.inc

config_subdirs="libtclconf libconnth libqdb libspoolbdb \
slavenbs slavefp slavein"

sed \
    -e "/@include@/ s||$INCLUDE|" \
    -e "/@q@/ s||$Q|g" \
    -e "/@INSTALL@/ s||$INSTALL|" \
    -e "/@INCDIRS@/s||$INCDIRS|" \
    -e "/@LIBDIRS@/s||$LIBDIRS|" \
    -e "/@LIBS@/s||$LIBS|" \
    -e "/@CC@/s||$CC|" \
    -e "/@CCWFLAGS@/s||$CCWFLAGS|" \
    -e "/@SUFFIXRULES@/ s||$SUFFIXRULES|" \
    -e "/@DEFINES@/ s||$DEFINES|" \
    Makefile.in > Makefile

# copy the config.h file appropriate for the OS
cd config.h.d
./configure.sh
cd ..

# the libdirs and subdirs
for d in $config_subdirs
do
  cd $d
  ./configure.sh
  cd ..
done
