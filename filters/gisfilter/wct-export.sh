#!/bin/sh
#
# $Id$
#
# install -m 0755 wct-export.sh /usr/local/bin/wct-export
# 
version="3.2.0"
wctdir=/usr/local/lib/wct-${version}

${wctdir}/wct-export "$@"
