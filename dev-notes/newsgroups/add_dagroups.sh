#!/bin/sh
#
# $Id$
#

# Initially, the data subgroups are:
#
#	sao
#	syn
#	ship
#	nwx.spc.day1
#	nwx.spc.day2
#	nwx.spc.day3
#	nwx.hpc.fronts

daprodlist=`cat dagroups.txt`

for p in $daprodlist
do
    ./add_dagroup1.sh $p $1
done
