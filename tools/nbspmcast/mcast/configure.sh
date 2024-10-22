#!/bin/sh

# mcast files from "src"
MCAST_DIR="../../../src"
MCAST_FILES="${MCAST_DIR}/mcast.h \
	     ${MCAST_DIR}/mcast.c"

for f in ${MCAST_FILES}
do
    cp $f .
done
