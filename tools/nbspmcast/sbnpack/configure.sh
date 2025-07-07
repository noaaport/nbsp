#!/bin/sh

# pack files from "src"
UTIL_DIR="../../../src"
UTIL_FILES="${UTIL_DIR}/util.h \
	     ${UTIL_DIR}/util.c"

for f in ${UTIL_FILES}
do
    cp $f .
done
