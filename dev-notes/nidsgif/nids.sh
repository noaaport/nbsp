#!/bin/sh

.  /home/gempak/Gemenviron.profile
#setenv DISPLAY localhost:0.0

inputfile=$1
outputfile=${inputfile}.gif
type=`echo $inputfile | cut -c 1-3`

# Default (N?R)
lut=osf_ref16.tbl

[ $type = N0S ] && lut=nids_vel16.tbl
[ $type = N1S ] && lut=nids_vel16.tbl
[ $type = N2S ] && lut=nids_vel16.tbl
[ $type = N3S ] && lut=nids_vel16.tbl
[ $type = N0V ] && lut=nids_vel16.tbl
[ $type = N1V ] && lut=nids_vel16.tbl
[ $type = N1P ] && lut=nids_pre.tbl
[ $type = NET ] && lut=nids_tops.tbl
[ $type = NTP ] && lut=nids_pre.tbl
[ $type = NVL ] && lut=nids_vil.tb
[ $type = NVW ] && lut=
 
gpmap_gif << EOF
 \$MAPFIL = states + county
 MAP = 1
 IMCBAR = 1
 GAREA = dset
 RADFIL = $inputfile
 LUT = default
 PROJ = rad
 DEVICE = gif|$outputfile|800;600
 LATLON = 0
 r

 e
EOF
