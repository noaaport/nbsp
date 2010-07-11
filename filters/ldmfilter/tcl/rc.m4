dnl
dnl $Id$
dnl

# First the specific ones.
match_ldmfeed($rc(wmoid), ^sdus, NEXRAD,
$rc(WMOHEADER) " /p" $rc(AWIPS))

match_ldmfeed($rc(nawips), ^grib$, NGRID,
$rc(WMOHEADER) " /m" $rc(gribmodelgridldm))

match_ldmfeed($rc(wmoid), ^ti, NIMAGE,
[mk_ldm_sat_prodid $rc(fname) $rc(fpath)])

# These catch the rest.
match_ldmfeed_not($rc(awips), ^$, WMO, 
$rc(WMOHEADER) " /p" $rc(AWIPS))

match_ldmfeed_not($rc(nawips), ^$, HDS,
$rc(WMOHEADER))
