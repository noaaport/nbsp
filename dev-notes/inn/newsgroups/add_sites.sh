#!/bin/sh
#
# $Id$

sitelist=`awk '!/^$|^#/{print $1;}' radsites.txt`

for s in $sitelist
do
    ./add_site1.sh $s $1
done
