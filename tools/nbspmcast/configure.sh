#!/bin/sh

. ../../configure.inc

configure_default

# files from "filterslib/c"
LIB_FILES_DIR="../../filters/filterlib/c"
LIB_FILES="${LIB_FILES_DIR}/err.h \
	${LIB_FILES_DIR}/config.h \
	${LIB_FILES_DIR}/err.c"

# stoi files from src
STOI_FILES_DIR="../../src"
STOI_FILES="${STOI_FILES_DIR}/stoi.h \
	 ${STOI_FILES_DIR}/stoi.c"

for f in ${LIB_FILES}
do
    cp $f .
done

for f in ${STOI_FILES}
do
    cp $f .
done

for d in mcast sbnpack
do
    cd $d
    ./configure.sh
    cd ..
done
