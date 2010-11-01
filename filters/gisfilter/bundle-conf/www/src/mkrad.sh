#!/bin/sh
#
# $Id$
#

rad_body="rad_body.tml.in"
rad_header_static="rad_header_static.tml.in"
rad_header_dynamic="rad_header_dynamic.tml.in"

title_n0r="N0R Base reflectivity, 124 nmi, 0.5 degree elevation"
title_n1r="N1R Base reflectivity, 124 nmi, 1.45\/1.5 degree elevation"
title_n2r="N2R Base reflectivity, 124 nmi, 2.4\/2.5 degree elevation"
title_n3r="N3R Base reflectivity, 124 nmi, 3.35\/3.5 degree elevation"
title_n0z="N0Z Base reflectivity, 248 nmi, 0.5 degree elevation"
#
title_n0q="N0Q Base reflectivity, 248 nmi, 0.5 degree elevation"
title_naq="NAQ Base reflectivity, 248 nmi, 0.9 degree elevation"
title_n1q="N1Q Base reflectivity, 248 nmi, 1.3\/1.5 degree elevation"
title_nbq="NBQ Base reflectivity, 248 nmi, 1.8 degree elevation"
title_n2q="N2Q Base reflectivity, 248 nmi, 2.4\/2.5 degree elevation"
title_n3q="N3Q Base reflectivity, 248 nmi, 3.1\/3.4\/3.5 degree elevation"
#
title_n0v="N0V Base radial velocity, 124 nmi, 0.5 degree elevation"
title_n1v="N1V Base radial velocity, 124 nmi, 1.45\/1.5 degree elevation"
#
title_n0u="N0U Base radial velocity, 162 nmi, 0.5 degree elevation"
title_nau="NAU Base radial velocity, 162 nmi, 0.9 degree elevation"
title_n1u="N1U Base radial velocity, 162 nmi, 1.3\/1.5 degree elevation"
title_nbu="NBU Base radial velocity, 162 nmi, 1.8 degree elevation"
title_n2u="N2U Base radial velocity, 162 nmi, 2.5\/2.5 degree elevation"
title_n3u="N3U Base radial velocity, 162 nmi, 3.1\/3.4\/3.5 degree elevation"

legend_n0r="gis_rad_legend_bref"
legend_n1r="gis_rad_legend_bref"
legend_n2r="gis_rad_legend_bref"
legend_n3r="gis_rad_legend_bref"
legend_n0z="gis_rad_legend_bref"
#
legend_n0q="gis_rad_legend_bref"
legend_naq="gis_rad_legend_bref"
legend_n1q="gis_rad_legend_bref"
legend_nbq="gis_rad_legend_bref"
legend_n2q="gis_rad_legend_bref"
legend_n3q="gis_rad_legend_bref"
legend_n0q="gis_rad_legend_bref"
#
legend_n0v="gis_rad_legend_rvel"
legend_n1v="gis_rad_legend_rvel"
#
legend_n0u="gis_rad_legend_rvel"
legend_nau="gis_rad_legend_rvel"
legend_n1u="gis_rad_legend_rvel"
legend_nbu="gis_rad_legend_rvel"
legend_n2u="gis_rad_legend_rvel"
legend_n3u="gis_rad_legend_rvel"

#
# main
#
directory=
[ $# -ne 0 ] && directory=$1

for awips1 in n0r n1r n2r n3r n0z \
    n0v n1v \
    n0q naq n1q nbq n2q n3q \
    n0u nau n1u nbu n2u n3u
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
	$rad_header_static > $outputfile

    sed -e "/^#/d" $rad_body >> $outputfile
  done

done

# The dynamic template
outputfile="awips1_name.tml"
[ -n "$directory" ] && outputfile="$directory/$outputfile"
sed -e "/^#/d" $rad_header_dynamic > $outputfile
sed -e "/^#/d" $rad_body >> $outputfile
