#!/bin/sh

# The sat subgroups are:
#
# noaaport.sat.img.<type>
# noaaport.sat.raw.<type>
#
# where <type> is:
#
# tiga, tigb, tige, tigf, tigh, tigi, tign, tigp, tigq, tigw

file=
if [ $# -eq 1 ]
then
    file=$1
fi

subgrouplist="tiga tigb tige tigf tigh tigi tign tigp tigq tigw"

if [ -z "$file" ]
then
    cd
    for g in $subgrouplist
    do
      echo -n "noaaport.sat.img.$g: "
      bin/ctlinnd newgroup noaaport.sat.img.$g y
      echo -n "noaaport.sat.raw.$g: "
      bin/ctlinnd newgroup noaaport.sat.raw.$g y
    done
else
    for g in $subgrouplist
    do
      echo noaaport.sat.img.$g >> $file
      echo noaaport.sat.raw.$g >> $file
    done
fi
