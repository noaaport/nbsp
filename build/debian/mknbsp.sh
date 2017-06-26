#!/bin/sh

DPKG_COLORS="none"
export DPKG_COLORS

name=nbsp

cd $name/build/debian
./mk.sh
