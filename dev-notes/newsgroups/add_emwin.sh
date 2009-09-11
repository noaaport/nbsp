#!/bin/sh

# The collective emwin groups are
#
#	noaaport.emwin.misc.adm
#	noaaport.emwin.{urgent,warnings,data,forecast,summary}
#	noaaport.emwin.img
#
# The individual station groups are created by add_station1.sh,
# which is called by add_stations.sh.

file=
if [ $# -eq 1 ]
then
    file=$1
fi

subgrouplist="emwin.misc.adm \
emwin.urgent \
emwin.warnings \
emwin.data \
emwin.forecast \
emwin.summary
emwin.img"

if [ -z "$file" ]
then
    cd
    for g in $subgrouplist
    do
      echo -n "noaaport.$g: "
      bin/ctlinnd newgroup noaaport.$g y
    done
else
    for g in $subgrouplist
    do
      echo noaaport.$g >> $file
    done
fi
