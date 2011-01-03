#!/bin/sh
#
# $Id$
#

sat_tml="sat.tml.in"

title_tige02="TIGE02 - IR 4 km @ 25N, 11 microns"
title_tigw02="TIGW02 - IR 4 km @ 25N, 11 microns"
title_tigp02="TIGP02 - IR 4 km @ 20N, 11 microns"
title_tigq02="TIGQ02 - IR 8 km @ 60N, 11 microns"
#

#
# main
#
directory=
[ $# -ne 0 ] && directory=$1

for name in tige02 tigw02 tigp02 tigq02
do
  eval title=\$title_$name
  outputfile=${name}.tml
  [ -n "$directory" ] && outputfile="$directory/$outputfile"
  sed -e "/@name@/s//$name/g" \
      -e "/@title@/s//$title/g" \
      -e "/^#/d" \
      $sat_tml > $outputfile
done
