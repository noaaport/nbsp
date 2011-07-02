dnl
dnl $Id$
dnl 

# Instead of using individual rules for the various grib files,
# we will throw to dcgrib2 everything that looks like a grib file,
# and let it handle it, including figuring out what name to use (from
# the gribkey.tbl file). This rule makes the rest of the grib rules
# unoperative. If we want to use the individual rules, this global rule
# can be commented out.

matchstop_pipe($rc(nawips), ^grib,
dcgrib2,
m4DCOPTS(dcgrib))

# All eta/nam models
match_and_pipe($rc(nawips), ^grib, $rc(gribmodel), eta|nam,
dcgrib2,
m4DCOPTS(dcgrib_eta))

# ruc
match_and_pipe($rc(nawips), ^grib, $rc(gribmodel), ruc,
dcgrib2,
m4DCOPTS(dcgrib_ruc))

# ngm
match_and_pipe($rc(nawips), ^grib, $rc(gribmodel), ngm,
dcgrib2,
m4DCOPTS(dcgrib_ngm))

# mrf
match_and_pipe($rc(nawips), ^grib, $rc(gribmodel), mrf,
dcgrib2,
m4DCOPTS(dcgrib_mrf))

# gfs/avn
match_and_pipe($rc(nawips), ^grib, $rc(gribmodel), avn|gfs|ssiavn|ssigfs,
dcgrib2,
m4DCOPTS(dcgrib_gfs))

dnl
dnl Originally commented out because it causes dcgrib2 to dump core too often
dnl
match_and_pipe($rc(station), kwbc, $rc(wmoid), ^[lm].r,
dcgrib2,
m4DCOPTS(dcgrib_gfsconus) -m 20000,
model/gfs/YYYYMMDDHH_gfs212.gem)

match_and_pipe($rc(station), kwbc, $rc(wmoid), ^[lm].g,
dcgrib2,
m4DCOPTS(dcgrib) -m 20000,
model/gfs/YYYYMMDDHH_gfs160.gem)

match_and_pipe($rc(station), kwbc, $rc(wmoid), ^[lm].o,
dcgrib2,
m4DCOPTS(dcgrib) -m 20000,
model/gfs/YYYYMMDDHH_gfs254.gem)

match_and_pipe($rc(station), kwbc, $rc(wmoid), ^[lm].t,
dcgrib2,
m4DCOPTS(dcgrib) -m 20000,
model/gfs/YYYYMMDDHH_gfs161.gem)

match_and_pipe($rc(station), kwbc, $rc(wmoid), ^h.[i-p],
dcgrib2,
m4DCOPTS(dcgrib))

match_and_pipe($rc(station), kwbk, $rc(wmoid), ^h,
dcgrib2,
m4DCOPTS(dcgrib))

match_and_pipe($rc(station), ^kwb, $rc(wmoid), ^h.[ef][a-z][0-9][0-9],
dcgrib2,
m4DCOPTS(dcgrib))

match_and_pipe($rc(station), ^kwb, $rc(wmoid), ^h.[abcd][a-z][0-9][0-9],
dcgrib2,
m4DCOPTS(dcgrib))

match_pipe($rc(wmoid), ^h.[t-w],
dcgrib2,
m4DCOPTS(dcgrib))

match_and_pipe($rc(station), kkci, $rc(wmoid), ^[yz],
dcgrib2,
m4DCOPTS(dcgrib))

match_pipe($rc(wmoid), ^o,
dcgrib2,
m4DCOPTS(dcgrib))

match_and_pipe($rc(station), k([^wk]..|w[^b].|k[^c].), $rc(wmoid), ^[yz],
dcgrib2, 
m4DCOPTS(dcgrib))

match_and_pipe($rc(station), ^k, $rc(wmoid), haxa00,
dcgrib2, 
m4DCOPTS(dcgrib))

match_and_pipe($rc(station), ^ecm, $rc(wmoid), ^h,
dcgrib2,
m4DCOPTS(dcgrib))

match_and_pipe($rc(station), egrr, $rc(wmoid), ^h,
dcgrib2, 
m4DCOPTS(dcgrib))

match_and_pipe($rc(station), kwbd, $rc(wmoid), ^[lm].e,
dcgrib2, 
m4DCOPTS(dcgrib) -m 200,
model/dgex/YYYYMMDDHHfFFF_dgex.gem)

match_and_pipe($rc(station), kwbd, $rc(wmoid), ^[lm].f,
dcgrib2, 
m4DCOPTS(dcgrib),
model/dgex-ak/YYYYMMDDHHfFFF_dgex.gem)

match_and_pipe($rc(station), kwbn, $rc(wmoid), ^[lm].u,
dcgrib2, 
m4DCOPTS(dcgrib) -v 1,
model/ndfd/YYYYMMDDHH_ndfd.gem)

match_and_pipe($rc(station), kwbq, $rc(wmoid), ^[lm].u,
dcgrib2, 
m4DCOPTS(dcgrib) -v 1,
model/mos/YYYYMMDDHH_gfsmos.gem)

match_and_pipe($rc(station), knhc, $rc(wmoid), ^[lm].g,
dcgrib2, 
m4DCOPTS(dcgrib) -v 1,
model/nhc/YYYYMMDDHH_forecast.gem)

match_stop($rc(nawips), ^grib)
