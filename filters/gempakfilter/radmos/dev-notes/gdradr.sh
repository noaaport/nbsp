#!/bin/sh

. /etc/local/gempak/gempaksh.conf

#GRDAREA = 23.0;-120.0;47.0;-65.0

rm -f last.nts gemglb.nts

gdradr <<EOF
GDPFUN = n0r
GDFILE = YYYYMMDD_radr.gem
RADTIM  = current
RADDUR  = 30
GRDAREA = us
PROJ    = lcc/25;-103;60
KXKY    = 720;500
CPYFIL=
MAXGRD  = 1000
r

e
EOF

rm -f last.nts gemglb.nts
