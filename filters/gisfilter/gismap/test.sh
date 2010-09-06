#!/bin/sh

outputdir="sat/img/tig"
outputfile="tig01.png"
inputdir="/var/noaaport/data/gis/sat/tif/tig"

# cd /var/noaaport/data/gis
mkdir -p $outputdir

./nbspgismap -d $outputdir -o $outputfile -g geodata -m map_sat_conus.tmpl \
    -I $inputdir -p "*.tif" tigw01 tige01
