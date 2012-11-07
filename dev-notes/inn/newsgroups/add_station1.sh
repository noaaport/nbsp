#!/bin/sh

# The station groups are
#
#	noaaport.txt.kkkk
#	noaaport.emwin.{urgent,warnings,data,forecast,summary}.kkkk

if [ $# -eq 1 ]
then
    station=$1
    file=
elif [ $# -eq 2 ]
then
    station=$1
    file=$2
else
    echo "One station as argument."
    exit 1
fi

subgrouplist="txt
emwin.urgent
emwin.warnings
emwin.data
emwin.forecast
emwin.summary"

if [ -z "$file" ]
then
    cd
    for g in $subgrouplist
    do
      echo -n "noaaport.$g.$station: "
      bin/ctlinnd newgroup noaaport.$g.$station y
    done
else
    for g in $subgrouplist
    do
      echo noaaport.$g.$station >> $file
    done
fi
