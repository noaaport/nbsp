dnl
dnl $Id$
dnl

# 
# Duplicates of warnings for grlevelx programs
#
match_or_file($rc(awips1),
cem|cfw|ffw|fls|flw|hls|hwo|npw|rfw|sps|svr|svs|tor|wsw,
$rc(body), EAS ACTIVATION, warnings, $ymdh.$rc(AWIPS1))
