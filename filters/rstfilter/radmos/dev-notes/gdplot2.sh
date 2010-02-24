#!/bin/sh

. /etc/local/gempak/gempaksh.conf

rm -f output.gif

#
# COLORS = 101=white
#
gpcolor <<EOF1
DEVICE = gif|output.gif
COLORS = 101=255:255:255
r

e
EOF1

background="101";
blue="27;26;25"
green="21;22;23"
yellow="19;18;17"
red="16;15;14;7;29"
white="31"
#
#fint="0;5;10;15;20;25;30;35;40;45;50;55;60;65;70;75"
#fline=${background};${background};${blue};${green};${yellow};${red};${white}
#FLINE = 101;101;27;26;25;21;22;23;19;18;17;16;15;14;7;29;31

gdplot2 <<EOF
GDFILE = 20100224_0146.gem
DEVICE = gif|output.gif
GDPFUN = n0r
GAREA = us
PROJ = def
TYPE = F
FINT = 0;5;10;15;20;25;30;35;40;45;50;55;60;65;70;75
fline=${background};${background};${blue};${green};${yellow};${red};${white}
GDATTIM = LAST
GLEVEL = 0
GVCORD = none
TITLE = 14/-2/n0r - `date -u`
r

e
EOF

gpend
rm -f last.nts gemglb.nts
