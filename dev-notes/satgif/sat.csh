#!/bin/csh -f

#setenv DISPLAY unix:0
source /home/gempak/Gemenviron

#
# GOES8 variable set in Gemenviron
#   GOES images follow NSAT naming convention
#     Platform/Resolution/type_yymmdd_hhmm
#

cd IR

#cd $GOES8/8km
#
set CURRENT_SAT=`ls IR_* | tail -1`

# generate gifs in /tmp
# cd /tmp

if(-e sat.gif ) then
   rm -f sat.gif*
endif

foreach FILE ($CURRENT_SAT)

gpmap << EOF
MAP      = 1
TITLE    = 1/-2/
DEVICE   = gf|sat.gif|350;300
SATFIL   = $FILE
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

end




