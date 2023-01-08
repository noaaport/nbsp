#!/bin/sh

. ../../configure.inc

configure_default

# files from "filterslib/c"
LIB_FILES_DIR="../../filters/filterlib/c"
LIB_HEADERS="${LIB_FILES_DIR}/err.h \
	${LIB_FILES_DIR}/util.h \
	${LIB_FILES_DIR}/config.h"
LIB_SOURCES="${LIB_FILES_DIR}/err.c ${LIB_FILES_DIR}/util.c"

for f in ${LIB_HEADERS} ${LIB_SOURCES}
do
    cp $f .
done
