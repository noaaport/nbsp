#!/bin/sh

. ../../VERSION

cat > changelog <<EOF
$name ($version-1) stable; urgency=low

  * Initial debian package release

 -- Jose F Nieves <nieves@ltp.upr.clu.edu>  `./date-R.sh`
EOF
