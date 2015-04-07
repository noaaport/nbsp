dnl
dnl $Id$
dnl

# First the specific ones.
match_ldmfeed($rc(wmoid), ^sdus, NNEXRAD,
$rc(WMOHEADER) " /p" $rc(AWIPS))

match_ldmfeed($rc(wmoid), ^ti[^p], NIMAGE,
[mk_ldm_sat_prodid rc])

match_ldmfeed($rc(wmoid), ^tip, NOTHER,
$rc(WMOHEADER))

match_ldmfeed($rc(nawips), ^grib$, NGRID,
[mk_ldm_grib_prodid rc])

# These catch the rest.
match_ldmfeed_not($rc(awips), ^$, WMO, 
$rc(WMOHEADER) " /p" $rc(AWIPS))

match_ldmfeed_not($rc(nawips), ^$, HDS,
$rc(WMOHEADER))
