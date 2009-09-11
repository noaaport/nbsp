#!/bin/sh
#
# $Id$
#
hostport="diablo:8015"
basedir=/var/noaaport/nbsp

#expr="{-n0(r|s|v|z)|n1(p|r|s|v)|n2(r|s)|n3(r|s)|ncr|nvl|net|ntp|nvw}"
#expr="{_s[ap]}"
expr="_sdus,!\-d,!\-nm"

nbspbatch get -b $basedir -e $expr $hostport
echo "Executing filter"
nbspbatch filter -b $basedir -a

#echo "Executing dafilter"
#nbspbatch filter -b $basedir -p dafilter -a -k
#echo "Executing gpfilter"
#nbspbatch filter -b $basedir -p gpfilter -a
#echo "Executing metarfilter"
#nbspbatch filter -b $basedir -p metarfilter -a
