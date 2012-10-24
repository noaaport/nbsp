#!/bin/sh

. /home/gempak/Gemenviron.profile

dir=$GEMDATA/images/sat/SUPER-NATIONAL/8km/IR

cd $dir

## satlist=`ls IR_* | tail -1`
satlist=`ls * | tail -1`

cd /home/users/nieves

for _f in $satlist
do
gpmap << EOF
MAP      = 1
TITLE    = 1/-2/
DEVICE   = gf|sat.gif|350;300
SATFIL   = $dir/${_f}
RADFIL   = 
PROJ     = sat
GAREA    = usnps
CLEAR    = y
PANEL    = 0
TEXT     = 1.2/23/1/hw
LATLON   = 0
\$mapfil = hipowo.gsf
r

e
EOF

gpend

done





