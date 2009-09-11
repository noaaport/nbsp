define(m4GEMTBL, -e GEMTBL=$gdec_GEMTBL)dnl
define(m4GEMPAK, -e GEMPAK=$gdec_GEMPAK)dnl

match_pipe($notawips, grib, $gdec_bindir/dcgrib2, 
-v 1 -d $gdec_logdir/dcgrib.log m4GEMTBL)dnl

match_pipe($wmoid, ^s[ap], $gdec_bindir/dcmetr, 
-v 2 -a 500 -m 72 -d $gdec_logdir/dcmetr.log m4GEMTBL -s sfmetar_sa.tbl,
surface/YYYYMMDD_sao.gem)dnl

match_pipe($wmoid, 
(^s(hv|hxx|s[^x]))|(^sx(vd|v.50|us(2[0-3]|08|40|82|86)))|(^y[ho]xx84), 
$gdec_bindir/dcmsfc, -b 9 -a 10000 -d $gdec_logdir/dcmsfc.log m4GEMTBL,
ship/YYYYMMDDHH_sb.gem)dnl

match_pipe_more($wmoid, 
(^s[imn]v[^gins])|(^s[imn]w[^kz]),
$gdec_bindir/dcmsfc, -b 9 -a 10000 -d $gdec_logdir/dcmsfc.log m4GEMTBL,
ship/YYYYMMDDHH_sb.gem)dnl

match_pipe($wmoid, (^s[imn]v[^gins])|(^s[imn]w[^kz]),
$gdec_bindir/dcmsfc, -a 6 -d $gdec_logdir/dcmsfc_6hr.log m4GEMTBL,
ship6hr/YYYYMMDDHH_ship.gem)dnl

match_pipe($wmoid, ^u[abcdefghijklmnpqrstwxy], $gdec_bindir/dcuair,
-b 24 -m 16 -d $gdec_logdir/dcuair.log m4GEMTBL -s snstns.tbl,
upperair/YYYYMMDD_upa.gem)dnl

match_pipe($wmoid, ^uz, $gdec_bindir/dcuair,
-a 50 -m 24 -d $gdec_logdir/dcuair_drop.log m4GEMTBL, 
drops/YYYYMMDD_drop.gem)dnl

match_pipe($wmoid, (^s[im]v[igns])|(^snv[ins])|(^s[imn](w[kz]|[^vw])),
$gdec_bindir/dclsfc, -v 2 -s lsystns.upc -d $gdec_logdir/dclsfc.log m4GEMTBL,
syn/YYYYMMDD_syn.gem)dnl


match_pipe($wmoid, fo(us14|ak1[34]|ak2[5-9]), $gdec_bindir/dcnmos,
-d $gdec_logdir/dcnmos.log m4GEMTBL, mos/YYYYMMDDHH_nmos.gem)dnl

match_pipe($wmoid, fous2[1-6]|foak3[7-9]|fopa20, $gdec_bindir/dcgmos, 
-d $gdec_logdir/dcgmos.log m4GEMTBL m4GEMPAK, mos/YYYYMMDDHH_gmos.gem)dnl

match_pipe($wmoid, feus2[1-6]|feak3[7-9]|fepa20, $gdec_bindir/dcxmos, 
-v 2 -d $gdec_logdir/dcxmos.log m4GEMTBL m4GEMPAK, mos/YYYYMMDDHH_xmos.gem)dnl

match_pipe($wmoid, fzxx41, $gdec_bindir/dcidft,
-v 2 -d $gdec_logdir/dcidft.log m4GEMTBL, idft/YYYYMMDDHH.idft)dnl

match_pipe($wmoid, ^fzak41, $gdec_bindir/dcidft,
-v 2 -d $gdec_logdir/dcidft.log m4GEMTBL, idft/YYYYMMDDHH.idak)dnl

match_pipe($fname, kwns_nwus2[02], $gdec_bindir/dcstorm, 
-m 2000 -d $gdec_logdir/dcstorm.log m4GEMTBL, storm/sels/YYYYMMDD_sels.gem)dnl

match_pipe($fname, (kmkc_wwus40|kwns_wwus30), $gdec_bindir/dcwatch,
-t 30 -d $gdec_logdir/dcwatch.log m4GEMTBL, storm/watches/watches_YYYY_MM.gem)dnl

match_pipe($awips1, tor|svr|ffw, $gdec_bindir/dcwarn,
-d $gdec_logdir/dcwarn.log m4GEMTBL, storm/warn/YYYYMMDDHH_warn.gem)dnl

match_pipe($wmoid, wwus(40|08)|wous20|wwus30, $gdec_bindir/dcwtch,
-t 30 -d $gdec_logdir/dcwtch.log m4GEMTBL, storm/wtch/YYYYMMDDHH_wtch.gem)dnl

match_pipe($wmoid, wwus(6[1-6]|32), $gdec_bindir/dcsvrl,
-d $gdec_logdir/dcsvrl.log m4GEMTBL, storm/svrl/YYYYMMDDHH_svrl.gem)dnl

include(gp_storms.m4)dnl

match_pipe($awips1, ffg|ffh, $gdec_bindir/dcffg, 
-d $gdec_logdir/dcffg.log m4GEMTBL, storm/ffg/YYYYMMDD_ffg.gem)dnl

match_pipe($fname, kawn_(xrxx84|yixx84|....u[abdr]), $gdec_bindir/dcacft, 
-d $gdec_logdir/dcacft.log m4GEMTBL, acft/YYYYMMDDHH_acf.gem)dnl

match_pipe($wmoid, waus01, $gdec_bindir/dcairm,
-d $gdec_logdir/dcairm.log m4GEMTBL, airm/YYYYMMDDHH_airm.gem)dnl

match_pipe($wmoid, ^ft, $gdec_bindir/dctaf, 
-d $gdec_logdir/dctaf.log m4GEMTBL, taf/YYYYMMDD00.taf)dnl

match_pipe($wmoid, fous5[1-5], $gdec_bindir/dcrdf, 
-v 4 -d $gdec_logdir/dcrdf.log m4GEMTBL, rdf/YYYYMMDDHH.rdf)dnl

match_pipe($awips1, wou, $gdec_bindir/dcwou,
-d $gdec_logdir/dcwou.log m4GEMTBL, storm/wou/YYYYMMDDHHNN.wou)dnl

match_pipe($awips1, wcn, $gdec_bindir/dcwcn,
-d $gdec_logdir/dcwcn.log m4GEMTBL, storm/wcn/YYYYMMDDHHNN.wcn)dnl
