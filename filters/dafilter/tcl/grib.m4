dnl
dnl $Id$
dnl 

# The only grib files processed here are the most common ones,
# and in particular those that DA can read. The rules in which the files
# are saved all in one big file are disabled here because that is
# done in the gribfilter.

# NAM(ETA)/GFS(AVN)/NGM/RUC (211) model on Lambert conformal CONUS grid
matchstop_and_grib($rc(station), ^kwb, $rc(wmoid) s s1 s2, ^([yz]).(q.),
grib/$rc(gribmodel)/$rc(gribymdh), [dafilter_make_default_grbname rc 1])
#
matchstop_and_grib($rc(station), ^kwb, $rc(wmoid) s s1 s2, ^([yz]).(q.),
grib/$rc(gribmodel), [dafilter_make_default_grbname rc 0])

# RUC2 model on Lambert conformal CONUS grid
matchstop_and_grib($rc(station), kwbg, $rc(wmoid) s s1 s2, ^(y).(w.),
grib/ruc2/$rc(gribymdh), [dafilter_make_default_grbname rc 1 ruc2])

#
matchstop_and_grib($rc(station), kwbg, $rc(wmoid) s s1 s2, ^(y).(w.),
[dafilter_make_default_grbname rc 0])

# ECMWF model on global 2.5 x 2.5 degree grid
matchstop_and_grib($rc(station), ecmf, $rc(wmoid) s s1 s2, ^(h).([a-l].),
grib/ecmwf/$rc(gribymdh), [dafilter_make_default_grbname rc 1])
#
matchstop_and_grib($rc(station), ecmf, $rc(wmoid) s s1 s2, ^(h).([a-l].),
[dafilter_make_default_grbname rc 0])

# MRF Grid 205 (Puerto Rico)
matchstop_and_grib($rc(station), kwbh, $rc(wmoid) s s1 s2, ^(y).(l.),
grib/mrf/$rc(gribymdh), [dafilter_make_default_grbname rc 1])
#
matchstop_and_grib($rc(station), kwbh, $rc(wmoid) s s1 s2, ^(y).(l.),
grib/mrf, [dafilter_make_default_grbname rc 0])

dnl This prevents reading the rest of the entire set of rules
dnl for the other grib files.
match_stop($rc(nawips), ^grib)

dnl # GFS model on thinned grids, interpolated to global 5.0 x 2.5 regular grid
dnl matchstop_and_grib($rc(station), kwbc, $rc(wmoid) s s1 s2, ^(h).)[i-p].),
dnl grib/gfs-thin/$rc(gribymdh), [dafilter_make_default_grbname rc 1])

dnl # MRF model on global 5.0 x 2.5 degree grids
dnl matchstop_and_grib($rc(station), kwbh, $rc(wmoid) s s1 s2, ^(h).([a-d].),
dnl grib/mrf25/$rc(gribymdh), [dafilter_make_default_grbname rc 1])

dnl # MRF model on global 5.0 x 5.0 degree grid
dnl matchstop_and_grib($rc(station), kwbh, $rc(wmoid) s s1 s2, ^(h).([ef].),
dnl grib/mrf50/$rc(gribymdh), [dafilter_make_default_grbname rc 1])

dnl # SST model on global 2.0 x 2.0 degree grid
dnl matchstop_and_grib($rc(station), kwbi, $rc(wmoid) s s1 s2, ^(h).([t-w].),
dnl grib/sst/$rc(gribymdh), [dafilter_make_default_grbname rc 1])
