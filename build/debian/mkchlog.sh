#!/bin/sh

. ../../VERSION
debrelease=`cat deb-release`

cat > changelog <<EOF
$name ($version-$debrelease) stable; urgency=low

  * Initial debian package release

 -- Jose F Nieves <nieves@ltp.uprrp.edu>  `./date-R.sh`
EOF
