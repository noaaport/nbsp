#!/bin/sh

# The station group is
#
#	noaaport.metar.kkkk

[ $# -ne 1 ] && { echo "One station as argument."; exit 1; }
station=$1

subgrouplist="metar.$station"

cd
for g in $subgrouplist
do
  echo -n "noaaport.$g: "
  bin/ctlinnd newgroup noaaport.$g y
done
