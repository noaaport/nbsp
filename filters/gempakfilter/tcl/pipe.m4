dnl
dnl $Id$
dnl 

match_pipe($rc(wmoid), ^s[ap],
dcmetr, 
-v 2 -a 500 -m 72 -d $gpfilter(dec_logdir)/dcmetr.log m4GEMTBL -s sfmetar_sa.tbl,
surface/YYYYMMDD_sao.gem)

match_pipe($rc(wmoid),
(^s(hv|hxx|s[^x]))|(^sx(vd|v.50|us(2[0-3]|08|40|82|86)))|(^y[ho]xx84),
dcmsfc,
-b 9 -a 10000 -d $gpfilter(dec_logdir)/dcmsfc.log m4GEMTBL,
ship/YYYYMMDDHH_sb.gem)

match_pipe($rc(wmoid), ^s[imn]v[^gins]|^s[imn]w[^kz],
dcmsfc,
-b 9 -a 10000 -d $gpfilter(dec_logdir)/dcmsfc.log m4GEMTBL,
ship/YYYYMMDDHH_sb.gem)

match_pipe($rc(wmoid), (^s[imn]v[^gins])|(^s[imn]w[^kz]),
dcmsfc,
-a 6 -d $gpfilter(dec_logdir)/dcmsfc_6hr.log m4GEMTBL,
ship6hr/YYYYMMDDHH_ship.gem)

match_pipe($rc(wmoid), ^u[abcdefghijklmnpqrstwxy],
dcuair,
-b 24 -m 16 -d $gpfilter(dec_logdir)/dcuair.log m4GEMTBL -s snstns.tbl,
upperair/YYYYMMDD_upa.gem)

match_pipe($rc(wmoid), ^uz,
dcuair,
-a 50 -m 24 -d $gpfilter(dec_logdir)/dcuair_drop.log m4GEMTBL, 
drops/YYYYMMDD_drop.gem)

match_pipe($rc(wmoid), (^s[im]v[igns])|(^snv[ins])|(^s[imn](w[kz]|[^vw])),
dclsfc,
-v 2 -s lsystns.upc -d $gpfilter(dec_logdir)/dclsfc.log m4GEMTBL,
syn/YYYYMMDD_syn.gem)

match_pipe($rc(wmoid), ^fo(us14|ak1[34]|ak2[5-9]),
dcnmos,
-d $gpfilter(dec_logdir)/dcnmos.log m4GEMTBL,
mos/YYYYMMDDHH_nmos.gem)

match_pipe($rc(wmoid), fous2[1-6]|foak3[7-9]|fopa20,
dcgmos, 
-d $gpfilter(dec_logdir)/dcgmos.log m4GEMTBL m4GEMPAK,
mos/YYYYMMDDHH_gmos.gem)

match_pipe($rc(wmoid), feus2[1-6]|feak3[7-9]|fepa20,
dcxmos, 
-v 2 -d $gpfilter(dec_logdir)/dcxmos.log m4GEMTBL m4GEMPAK,
mos/YYYYMMDDHH_xmos.gem)

match_pipe($rc(wmoid), ^fzxx41,
dcidft,
-v 2 -d $gpfilter(dec_logdir)/dcidft.log m4GEMTBL,
idft/YYYYMMDDHH.idft)

match_pipe($rc(wmoid), ^fzak41,
dcidft,
-v 2 -d $gpfilter(dec_logdir)/dcidft.log m4GEMTBL,
idft/YYYYMMDDHH.idak)

match_pipe($rc(fname), kwns_nwus2[02],
dcstorm, 
-m 2000 -d $gpfilter(dec_logdir)/dcstorm.log m4GEMTBL,
storm/sels/YYYYMMDD_sels.gem)

match_pipe($rc(fname), (kmkc_wwus40|kwns_wwus30),
dcwatch,
-t 30 -d $gpfilter(dec_logdir)/dcwatch.log m4GEMTBL,
storm/watches/watches_YYYY_MM.gem)

match_pipe($rc(awips1), tor|svr|ffw,
dcwarn,
-d $gpfilter(dec_logdir)/dcwarn.log m4GEMTBL,
storm/warn/YYYYMMDDHH_warn.gem)

match_pipe($rc(wmoid), ^wwus(40|08)|wous20|wwus30,
dcwtch,
-t 30 -d $gpfilter(dec_logdir)/dcwtch.log m4GEMTBL,
storm/wtch/YYYYMMDDHH_wtch.gem)

match_and_pipe($rc(station), kwns, $rc(wmoid), wwus60,
dcwcp,
-d $gpfilter(dec_logdir)/dcwcp.log m4GEMTBL,
storm/wcp/YYYYMMDDHH.wcp)

match_pipe($rc(wmoid), ^wwus(6[1-6]|32),
dcsvrl,
-d $gpfilter(dec_logdir)/dcsvrl.log m4GEMTBL,
storm/svrl/YYYYMMDDHH_svrl.gem)

dnl
dnl Hurricanes
dnl
match_pipe($rc(wmoid), ^wtpz4,
dctrop,
-d $gpfilter(dec_logdir)/dctrop.log m4GEMTBL,
storm/tropic/epacific/YYYY_@@.gem)

match_pipe($rc(wmoid), ^wtpn3,
dctrop,
-d $gpfilter(dec_logdir)/dctrop.log m4GEMTBL,
storm/tropic/wpacific/YYYY_@@.gem)

match_pipe($rc(wmoid), ^wtnt4,
dctrop,
-d $gpfilter(dec_logdir)/dctrop.log m4GEMTBL,
storm/tropic/atlantic/YYYY_@@.gem)

match_pipe($rc(wmoid), ^wtpa4,
dctrop,
-d $gpfilter(dec_logdir)/dctrop.log m4GEMTBL,
storm/tropic/cpacific/YYYY_@@.gem)

dnl
dnl dchrcn
dnl
match_and_pipe($rc(station), knhc|phfo, $rc(wmoid), ^wt(nt|pz|pa)2[1-5],
dchrcn,
-d $gpfilter(dec_logdir)/dchrcn.log m4GEMTBL,
storm/hrcn/YYYYMMDDHH.hrcn)

dnl
dnl Decoder for Winter Warnings, Watches and Advisories
dnl
match_pipe($rc(wmoid), ^wwus4[1-6],
dcwstm,
-d $gpfilter(dec_logdir)/dcwstm.log m4GEMTBL,
storm/wstm/YYYYMMDDHH.wstm)

dnl
dnl flash flood guidance
dnl
match_pipe($rc(awips1), ffg|ffh,
dcffg, 
-d $gpfilter(dec_logdir)/dcffg.log m4GEMTBL,
storm/ffg/YYYYMMDD_ffg.gem)

match_pipe($rc(awips1), scd,
dcscd, 
-d $gpfilter(dec_logdir)/dcscd.log m4GEMTBL,
scd/YYYYMMDD_scd.gem)

match_pipe($rc(fname), kawn_(xrxx84|yixx84)|_u[abdr],
dcacft, 
-d $gpfilter(dec_logdir)/dcacft.log m4GEMTBL,
acft/YYYYMMDDHH_acf.gem)

match_pipe($rc(fname), kkci_waus4[1-5]|_waus01,
dcairm,
-d $gpfilter(dec_logdir)/dcairm.log m4GEMTBL,
airm/YYYYMMDDHH_airm.gem)

match_and_pipe($rc(station), kkci|phfo|pawu|pgum|panc,
$rc(wmoid), ^w[scv](pn|nt|pa|ak),
dcisig,
-d $gpfilter(dec_logdir)/dcisig.log m4GEMTBL,
isig/YYYYMMDDHH_isig.gem)

match_pipe($rc(fname), kkci_w[scv]us0[1-6]|_wsus01,
dcncon,
-d $gpfilter(dec_logdir)/dcncon.log m4GEMTBL,
ncon/YYYYMMDDHH_sgmt.gem)

match_and_pipe($rc(station), kkci, $rc(wmoid),
wsus4[0-2]|wcpa3[1-5]|wsus3[1-3],
dccsig,
-d $gpfilter(dec_logdir)/dccsig.log m4GEMTBL,
csig/YYYYMMDDHH.conv)

match_pipe($rc(wmoid), ^ft,
dctaf, 
-d $gpfilter(dec_logdir)/dctaf.log m4GEMTBL,
taf/YYYYMMDD00.taf)

match_pipe($rc(wmoid), ^fous5[1-5],
dcrdf, 
-v 4 -d $gpfilter(dec_logdir)/dcrdf.log m4GEMTBL,
rdf/YYYYMMDDHH.rdf)

match_pipe($rc(awips1), wou,
dcwou,
-d $gpfilter(dec_logdir)/dcwou.log m4GEMTBL,
storm/wou/YYYYMMDDHHNN.wou)

dnl I have seen this decoder hang forever, and in that case it
dnl has to be killed by hand.
match_pipe($rc(awips1), wcn,
dcwcn,
-d $gpfilter(dec_logdir)/dcwcn.log m4GEMTBL,
storm/wcn/YYYYMMDDHHNN.wcn)

dnl Old profiler hourly summaries in BUFR format
match_pipe($rc(fname), kbou_iupt0[1-4],
dcprof,
-v 4 -s profiler_fsl.stn -d $gpfilter(dec_logdir)/dcprof.log m4GEMTBL tableb tabled,
profiler_bufr/YYYYMMDD_pro.gem)
