#!/bin/sh

files="\
expire.ctl \
inn.conf \
newsfeeds \
readers.conf \
storage.conf"

etcdir="/usr/local/etc/news"

for f in $files
do
    fpath=$etcdir/$f
    if [ ! -f ${fpath}.orig ]
    then
	mv $fpath ${fpath}.orig
    else
	mv $fpath ${fpath}.bak
    fi
    install -m 0644 -o news -g news $f $etcdir
done
