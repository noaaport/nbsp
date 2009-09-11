#!/bin/sh
#
# $Id$

stationslist=`awk '!/^$|^#/{print $1;}' metarstations.txt`

for s in $stationslist
do
    ./add_metar1.sh $s
done
