#!/bin/sh

dbhome=/var/noaaport/nbsp/cache
dbname=cachedb
dbcache_mb=32
nslots=4

# [ $# -eq 0 ] && exit 0
# key=$1
# nbspcspoolr -d $dbhome -f $dbname -c $dbcache_mb -n $nslots $key

# Read from stdin
nbspcspoolr -b -d $dbhome -f $dbname -c $dbcache_mb -n $nslots
