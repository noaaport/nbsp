#!/bin/sh
#
# $Id$
#

rad_header="rad_header.tml.in"
rad_body="rad_body.tml.in"

title_n0r="Base reflectivity, 124 nmi, 0.50 degree elevation"
title_n1r="Base reflectivity, 124 nmi, 1.45\/1.5 degree elevation"
title_n2r="Base reflectivity, 124 nmi, 2.40\/2.50 degree elevation"
title_n3r="Base reflectivity, 124 nmi, 3.35\/3.50 degree elevation"
title_n0z="Base reflectivity, 248 nmi, 0.50 degree elevation"
title_n0v="Base radial velocity, 124 nmi, 0.50 degree elevation"
title_n1v="Base radial velocity, 124 nmi, 1.45\/1.5 degree elevation"

legend_n0r="gis_rad_legend_bref"
legend_n1r="gis_rad_legend_bref"
legend_n2r="gis_rad_legend_bref"
legend_n3r="gis_rad_legend_bref"
legend_n0z="gis_rad_legend_bref"
legend_n0v="gis_rad_legend_brvel"
legend_n1v="gis_rad_legend_brvel"

#
# main
#
directory=
[ $# -ne 0 ] && directory=$1

for awips1 in n0r n1r n2r n3r n0z n0v n1v
do
  eval legend=\$legend_$awips1
  eval title=\$title_$awips1

  for name in ak hi pr west south central east conus
  do
    outputfile=${awips1}_${name}.tml
    [ -n "$directory" ] && outputfile="$directory/$outputfile"
    sed -e "/@awips1@/s//$awips1/g" \
	-e "/@name@/s//$name/g" \
	-e "/@title@/s//$title/g" \
	-e "/@legend@/s//$legend/g" \
	-e "/^#/d" \
	$rad_header > $outputfile

    sed -e "/^#/d" $rad_body >> $outputfile
  done
done
