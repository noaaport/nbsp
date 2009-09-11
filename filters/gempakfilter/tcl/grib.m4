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
dcgrib2, -d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL)

# All eta/nam models
match_and_pipe($rc(nawips), ^grib, $rc(gribmodel), eta|nam,
dcgrib2,
-d $gpfilter(dec_logdir)/dcgrib_eta.log m4GEMTBL)

# ruc
match_and_pipe($rc(nawips), ^grib, $rc(gribmodel), ruc,
dcgrib2,
-d $gpfilter(dec_logdir)/dcgrib_ruc.log m4GEMTBL)

# ngm
match_and_pipe($rc(nawips), ^grib, $rc(gribmodel), ngm,
dcgrib2,
-d $gpfilter(dec_logdir)/dcgrib_ngm.log m4GEMTBL)

# mrf
match_and_pipe($rc(nawips), ^grib, $rc(gribmodel), mrf,
dcgrib2,
-d $gpfilter(dec_logdir)/dcgrib_mrf.log m4GEMTBL)

# gfs/avn
match_and_pipe($rc(nawips), ^grib, $rc(gribmodel), avn|gfs|ssiavn|ssigfs,
dcgrib2,
-d $gpfilter(dec_logdir)/dcgrib_gfs.log m4GEMTBL)

dnl
dnl Originally commented out because it causes dcgrib2 to dump core too often
dnl
match_and_pipe($rc(station), kwbc, $rc(wmoid), ^[lm].r,
dcgrib2,
-m 20000 -d $gpfilter(dec_logdir)/dcgrib_gfsconus.log m4GEMTBL,
model/gfs/YYYYMMDDHH_gfs212.gem)

match_and_pipe($rc(station), kwbc, $rc(wmoid), ^[lm].g,
dcgrib2,
-m 20000 -d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL,
model/gfs/YYYYMMDDHH_gfs160.gem)

match_and_pipe($rc(station), kwbc, $rc(wmoid), ^[lm].o,
dcgrib2,
-m 20000 -d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL,
model/gfs/YYYYMMDDHH_gfs254.gem)

match_and_pipe($rc(station), kwbc, $rc(wmoid), ^[lm].t,
dcgrib2,
-m 20000 -d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL,
model/gfs/YYYYMMDDHH_gfs161.gem)

match_and_pipe($rc(station), kwbc, $rc(wmoid), ^h.[i-p],
dcgrib2,
-d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL)

match_and_pipe($rc(station), kwbk, $rc(wmoid), ^h,
dcgrib2,
-d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL)

match_and_pipe($rc(station), ^kwb, $rc(wmoid), ^h.[ef][a-z][0-9][0-9],
dcgrib2,
-d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL)

match_and_pipe($rc(station), ^kwb, $rc(wmoid), ^h.[abcd][a-z][0-9][0-9],
dcgrib2,
-d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL)

match_pipe($rc(wmoid), ^h.[t-w],
dcgrib2,
-d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL)

match_and_pipe($rc(station), kkci, $rc(wmoid), ^[yz],
dcgrib2,
-d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL)

match_pipe($rc(wmoid), ^o,
dcgrib2,
-d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL)

match_and_pipe($rc(station), k([^wk]..|w[^b].|k[^c].), $rc(wmoid), ^[yz],
dcgrib2, 
-d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL)

match_and_pipe($rc(station), ^k, $rc(wmoid), haxa00,
dcgrib2, 
-d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL)

match_and_pipe($rc(station), ^ecm, $rc(wmoid), ^h,
dcgrib2, 
-d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL)

match_and_pipe($rc(station), egrr, $rc(wmoid), ^h,
dcgrib2, 
-d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL)

match_and_pipe($rc(station), kwbd, $rc(wmoid), ^[lm].e,
dcgrib2, 
-m 200 -d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL,
model/dgex/YYYYMMDDHHfFFF_dgex.gem)

match_and_pipe($rc(station), kwbd, $rc(wmoid), ^[lm].f,
dcgrib2, 
-d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL,
model/dgex-ak/YYYYMMDDHHfFFF_dgex.gem)

match_and_pipe($rc(station), kwbn, $rc(wmoid), ^[lm].u,
dcgrib2, 
-v 1 -d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL,
model/ndfd/YYYYMMDDHH_ndfd.gem)

match_and_pipe($rc(station), kwbq, $rc(wmoid), ^[lm].u,
dcgrib2, 
-v 1 -d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL,
model/mos/YYYYMMDDHH_gfsmos.gem)

match_and_pipe($rc(station), knhc, $rc(wmoid), ^[lm].g,
dcgrib2, 
-v 1 -d $gpfilter(dec_logdir)/dcgrib.log m4GEMTBL,
model/nhc/YYYYMMDDHH_forecast.gem)

match_stop($rc(nawips), ^grib)
