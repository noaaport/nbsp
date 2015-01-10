dnl
dnl $Id$
dnl 


# After a few special cases below, the first rule matches all the grib files.
# The directory names are chosen according to the definitions in
# filters-gridid.def, and also makes unoperative the remaining grib rules.
# To use individual rules, the first one must be
# commented out, or the invidual ones placed first in this file and leave
# this one as the last one as a catch-all.

# This is a special case: a grib2 with center = 7, subc = 0. If the rule
# is not put here, it is saved under "knes". (Google on "knes lama98".)
matchstop_and_file($rc(station), knes, $rc(wmoid), ^lama,
$gribfilter(grbdatadir)/rtma/$rc(gribymdh),
[gribfilter_make_default_name rc 1 rtma])

# These are special cases: when the "model name" is a generic name (i.e. the
# name of the center or subcenter) the files will be saved in a common
# directory but with the filenames distinguished only by the model number
# or by the "station".
matchstop_and_file($rc(gribmodel), ^(rfc|ffg)$,
$rc(wmoid) s s1 s2, ^(.).(..),
$gribfilter(grbdatadir)/$rc(gribmodel)/$rc(gribymdh),
[gribfilter_make_default_name rc 1 $rc(gribmodel)-$rc(station)])

#
matchstop_and_file($rc(gribmodel),
^(ens|ncep|emc|hpc|mpc|cpc|awc|spc|nhc|nesdis|nwsmdl|ukm)$,
$rc(wmoid) s s1 s2, ^(.).(..),
$gribfilter(grbdatadir)/$rc(gribmodel)/$rc(gribymdh),
[gribfilter_make_default_name rc 1 $rc(gribmodel)-$rc(gribmodelnum)])

# Match all. The rest of rules after this one are not read.
matchstop_file($rc(wmoid) s s1 s2, ^(.).(..),
$gribfilter(grbdatadir)/$rc(gribmodel)/$rc(gribymdh),
[gribfilter_make_default_name rc 1])

# All from kwb.
matchstop_and_file($rc(station), ^kwb, $rc(wmoid) s s1 s2, ^([ehlmoyz]).(..),
$gribfilter(grbdatadir)/$rc(gribmodel)/$rc(gribymdh),
[gribfilter_make_default_name rc 1])

matchstop_and_file($rc(station), kwbn, $rc(wmoid), ^[lm].u.,
$gribfilter(grbdatadir)/ndfd/$rc(gribymdh), $rc(gribymdh)_ndfd.grb)

matchstop_and_file($rc(station), ^k, $rc(wmoid), ^haxa00,
$gribfilter(grbdatadir)/ndfd/$rc(gribymdh), $rc(gribymdh)_radar.grb)

# The next rules are examples of what can be used, for example to
# customize the directory and/or file name conventions for special
# purposes.

# NAM(ETA)/GFS(AVN)/NGM/RUC (211) model on Lambert conformal CONUS grid
matchstop_and_file($rc(station), ^kwb, $rc(wmoid) s s1 s2, ^([yz]).(q.),
$gribfilter(grbdatadir)/$rc(gribmodel)/$rc(gribymdh),
[gribfilter_make_default_name rc 1])

# RAP model on Lambert conformal CONUS grid
matchstop_and_file($rc(station), kwbg, $rc(wmoid) s s1 s2, ^(y).(w.),
$gribfilter(grbdatadir)/ruc2/$rc(gribymdh),
[gribfilter_make_default_name rc 1 ruc2])

# ECMWF model on global 2.5 x 2.5 degree grid
matchstop_and_file($rc(station), ecmf, $rc(wmoid) s s1 s2, ^(h).([a-l].),
$gribfilter(grbdatadir)/ecmwf/$rc(gribymdh),
[gribfilter_make_default_name rc 1 ecmwf])

# UKMET
matchstop_and_file($rc(station), egrr, $rc(wmoid) s s1 s2, ^(h).(..),
$gribfilter(grbdatadir)/ukmet/$rc(gribymdh),
[gribfilter_make_default_name rc 1 ukmet])

# GFS model on thinned grids, interpolated to global 5.0 x 2.5 regular grid
matchstop_and_file($rc(station), kwbc, $rc(wmoid) s s1 s2, ^(h).([i-p].),
$gribfilter(grbdatadir)/gfs/$rc(gribymdh),
[gribfilter_make_default_name rc 1 gfs])

# SST model on global 2.0 x 2.0 degree grid
matchstop_and_file($rc(station), kwbi, $rc(wmoid) s s1 s2, ^(h).([t-w].),
$gribfilter(grbdatadir)/sst/$rc(gribymdh),
[gribfilter_make_default_name rc 1 sst])
