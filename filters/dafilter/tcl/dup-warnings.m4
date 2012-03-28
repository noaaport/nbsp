dnl
dnl $Id$
dnl

# 
# Duplicates of warnings for grlevelx programs
#
dnl
dnl (2012-03-25: see pqact-grwarnings.conf in dev notes for the new scheme)
dnl
dnl match_or_file($rc(awips1),
dnl cem|cfw|ffw|fls|flw|hls|hwo|npw|rfw|sps|svr|svs|tor|wsw,
dnl $rc(body), EAS ACTIVATION, warnings, ${ymdh}.$rc(AWIPS1))
dnl
match_file($rc(awips1),
svr|tor|ffw|svs|smw|mws,
warnings, ${ymdh}.$rc(AWIPS1))

#
# The new GR scheme
#
match_file($rc(awips1),
svr|tor|ffw|svs|smw|mws,
warnings, warnings_${ymd_h}.txt)
