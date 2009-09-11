#!/bin/sh

distdir=/usr/local/news
datadir=/var/news

cd $distdir

mv db $datadir
rm -r run
rm -r spool

# syslog.conf
#
# uncomment news.. entries
