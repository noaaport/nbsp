#!/bin/sh
#
# $Id$
#

groupfile=gribgroups.txt

grouplist=`awk '!/^$|^#/{print $1;}' $groupfile`
for g in $grouplist
do
    ./add_grib1.sh $g $1
done
