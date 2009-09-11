#!/bin/sh
#
# $Id$
#

# Script to create a directory listing in each radar data subdirectory
# under a given base directory. Should be installed as a cron job.

# the (optional) configuration file
configfile=/usr/local/etc/nbsp/mkdirlist.conf

# defaults
mkdirlist_enable=1
basedir=/var/noaaport/data/digatmos/nexrad/nids
dirlistname=dir.list

# read overrides
[ -f $configfile ] && . $configfile

[ $mkdirlist_enable -eq 0 ] && exit

[ ! -d $basedir ] && { echo "$basedir not found."; exit 1; }
cd $basedir
for s in *
do
    [ ! -d $s ] && continue
    [ $s = "." -o $s = ".." ] && continue
    cd $s
    for p in *
    do
	[ ! -d $p ] && continue
	[ $p = "." -o $p = ".." ] && continue
	cd $p
	# only those with the radar data extension
	for _f in *.$p
	do
	  echo ${_f}
	done | sort > $dirlistname
	cd ..
    done
    cd ..
done
