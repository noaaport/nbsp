include(gp.defs.m4)dnl
define(m4GEMTBL, -e GEMTBL=$gtabledir)dnl
define(m4GEMPAK, -e GEMPAK=$gempak_datadir)dnl

match($notawips, grib, $gdecdir/dcgrib2, 
-v 1 -d $glogdir/dcgrib.log m4GEMTBL)dnl

match($wmoid, ^s[ap], $gdecdir/dcmetr, 
-v 2 -a 500 -m 72 -d $glogdir/dcmetr.log m4GEMTBL -s sfmetar_sa.tbl,
surface/YYYYMMDD_sao.gem)dnl

match($wmoid, 
(^s(hv|hxx|s[^x]))|(^sx(vd|v.50|us(2[0-3]|08|40|82|86)))|(^y[ho]xx84), 
$gdecdir/dcmsfc, -b 9 -a 10000 -d $glogdir/dcmsfc.log m4GEMTBL,
ship/YYYYMMDDHH_sb.gem)dnl

matchmore($wmoid, 
(^s[imn]v[^gins])|(^s[imn]w[^kz]),
$gdecdir/dcmsfc, -b 9 -a 10000 -d $glogdir/dcmsfc.log m4GEMTBL,
ship/YYYYMMDDHH_sb.gem)dnl

match($wmoid, (^s[imn]v[^gins])|(^s[imn]w[^kz]),
$gdecdir/dcmsfc, -a 6 -d $glogdir/dcmsfc_6hr.log m4GEMTBL,
ship6hr/YYYYMMDDHH_ship.gem)dnl

match($wmoid, ^u[abcdefghijklmnpqrstwxy], $gdecdir/dcuair,
-b 24 -m 16 -d $glogdir/dcuair.log m4GEMTBL -s snstns.tbl,
upperair/YYYYMMDD_upa.gem)dnl

match($wmoid, ^uz, $gdecdir/dcuair,
-a 50 -m 24 -d $glogdir/dcuair_drop.log m4GEMTBL, 
drops/YYYYMMDD_drop.gem)dnl

match($wmoid, (^s[im]v[igns])|(^snv[ins])|(^s[imn](w[kz]|[^vw])),
$gdecdir/dclsfc, -v 2 -s lsystns.upc -d $glogdir/dclsfc.log m4GEMTBL,
syn/YYYYMMDD_syn.gem)dnl


match($wmoid, fo(us14|ak1[34]|ak2[5-9]), $gdecdir/dcnmos,
-d $glogdir/dcnmos.log m4GEMTBL, mos/YYYYMMDDHH_nmos.gem)dnl

match($wmoid, fous2[1-6]|foak3[7-9]|fopa20, $gdecdir/dcgmos, 
-d $glogdir/dcgmos.log m4GEMTBL m4GEMPAK, mos/YYYYMMDDHH_gmos.gem)dnl

match($wmoid, feus2[1-6]|feak3[7-9]|fepa20, $gdecdir/dcxmos, 
-v 2 -d $glogdir/dcxmos.log m4GEMTBL m4GEMPAK, mos/YYYYMMDDHH_xmos.gem)dnl

match($wmoid, fzxx41, $gdecdir/dcidft,
-v 2 -d $glogdir/dcidft.log m4GEMTBL, idft/YYYYMMDDHH.idft)dnl

match($wmoid, ^fzak41, $gdecdir/dcidft,
-v 2 -d $glogdir/dcidft.log m4GEMTBL, idft/YYYYMMDDHH.idak)dnl

match($fname, kwns_nwus2[02], $gdecdir/dcstorm, 
-m 2000 -d $glogdir/dcstorm.log m4GEMTBL, storm/sels/YYYYMMDD_sels.gem)dnl

match($fname, (kmkc_wwus40|kwns_wwus30), $gdecdir/dcwatch,
-t 30 -d $glogdir/dcwatch.log m4GEMTBL, storm/watches/watches_YYYY_MM.gem)dnl

match($awips1, tor|svr|ffw, $gdecdir/dcwarn,
-d $glogdir/dcwarn.log m4GEMTBL, storm/warn/YYYYMMDDHH_warn.gem)dnl

match($wmoid, wwus(40|08)|wous20|wwus30, $gdecdir/dcwtch,
-t 30 -d $glogdir/dcwtch.log m4GEMTBL, storm/wtch/YYYYMMDDHH_wtch.gem)dnl

match($wmoid, wwus(6[1-6]|32), $gdecdir/dcsvrl,
-d $glogdir/dcsvrl.log m4GEMTBL, storm/svrl/YYYYMMDDHH_svrl.gem)dnl

match($awips1, ffg|ffh, $gdecdir/dcffg, 
-d $glogdir/dcffg.log m4GEMTBL, storm/ffg/YYYYMMDD_ffg.gem)dnl

match($fname, kawn_(xrxx84|yixx84|....u[abdr]), $gdecdir/dcacft, 
-d $glogdir/dcacft.log m4GEMTBL, acft/YYYYMMDDHH_acf.gem)dnl

match($wmoid, waus01, $gdecdir/dcairm,
-d $glogdir/dcairm.log m4GEMTBL, airm/YYYYMMDDHH_airm.gem)dnl

match($wmoid, ^ft, $gdecdir/dctaf, 
-d $glogdir/dctaf.log m4GEMTBL, taf/YYYYMMDD00.taf)dnl

match($wmoid, fous5[1-5], $gdecdir/dcrdf, 
-v 4 -d $glogdir/dcrdf.log m4GEMTBL, rdf/YYYYMMDDHH.rdf)dnl

match($awips1, wou, $gdecdir/dcwou,
-d $glogdir/dcwou.log m4GEMTBL, storm/wou/YYYYMMDDHHNN.wou)dnl

match($awips1, wcn, $gdecdir/dcwcn,
-d $glogdir/dcwcn.log m4GEMTBL, storm/wcn/YYYYMMDDHHNN.wcn)dnl
