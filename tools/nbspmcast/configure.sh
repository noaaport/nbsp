#!/bin/sh

. ../../configure.inc

configure_default

# files from "filterslib/c"
LIB_FILES_DIR="../../filters/filterlib/c"
LIB_FILES="${LIB_FILES_DIR}/err.h \
	${LIB_FILES_DIR}/config.h \
	${LIB_FILES_DIR}/err.c"

# pack_unintN files from src
SRC_UTIL_FILES="${SRC_FILES_DIR}/util.h \
	        ${SRC_FILES_DIR}/util.c"

for f in ${LIB_FILES}
do
    cp $f .
done

for d in mcast sbnpack
do
    cd $d
    ./configure.sh
    cd ..
done
