#!/bin/sh

rm -f active
cp control active

./add_misc.sh active.tmp
./add_dagroups.sh active.tmp
./add_stations.sh active.tmp
./add_emwin.sh active.tmp
./add_sites.sh active.tmp
./add_sat.sh active.tmp

awk -f mkactive.awk active.tmp >> active

rm active.tmp
