#!/bin/sh
#
# $Id$

stationsfile=stations.txt

stationslist=`awk '!/^$|^#/{print $1;}' $stationsfile`
for s in $stationslist
do
    ./add_station1.sh $s $1
done
