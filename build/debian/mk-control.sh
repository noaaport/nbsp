#!/bin/sh

# The defaults are for debian-7
ID=debian
VERSION_ID="7"

if [ -f /etc/os-release ]
then
    . /etc/os-release
fi

cp control.${ID}-${VERSION_ID} control
