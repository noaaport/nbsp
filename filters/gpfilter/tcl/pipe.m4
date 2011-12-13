dnl
dnl $Id$
dnl 

match_pipe($rc(wmoid), ^s[ap],
dcmetr, 
m4DCOPTS(dcmetr) -v 2 -a 500 -m 72 -s sfmetar_sa.tbl,
surface/YYYYMMDD_sao.gem)

match_pipe($rc(wmoid),
(^s(hv|hxx|s[^x]))|(^sx(vd|v.50|us(2[0-3]|08|40|82|86)))|(^y[ho]xx84),
dcmsfc,
m4DCOPTS(dcmsfc) -b 9 -a 10000,
ship/YYYYMMDDHH_sb.gem)

match_pipe($rc(wmoid), ^s[imn]v[^gins]|^s[imn]w[^kz],
dcmsfc,
m4DCOPTS(dcmsfc) -b 9 -a 10000,
ship/YYYYMMDDHH_sb.gem)

match_pipe($rc(wmoid), (^s[imn]v[^gins])|(^s[imn]w[^kz]),
dcmsfc,
m4DCOPTS(dcmsfc_6hr) -a 6,
ship6hr/YYYYMMDDHH_ship.gem)

match_pipe($rc(wmoid), ^u[abcdefghijklmnpqrstwxy],
dcuair,
m4DCOPTS(dcuair) -b 24 -m 16 -s snstns.tbl,
upperair/YYYYMMDD_upa.gem)

match_pipe($rc(wmoid), ^uz,
dcuair,
m4DCOPTS(dcuair_drop) -a 50 -m 24, 
drops/YYYYMMDD_drop.gem)

match_pipe($rc(wmoid), (^s[im]v[igns])|(^snv[ins])|(^s[imn](w[kz]|[^vw])),
dclsfc,
m4DCOPTS(dclsfc) -v 2 -s lsystns.upc,
syn/YYYYMMDD_syn.gem)

match_pipe($rc(wmoid), ^fo(us14|ak1[34]|ak2[5-9]),
dcnmos,
m4DCOPTS(dcnmos),
mos/YYYYMMDDHH_nmos.gem)

match_pipe($rc(wmoid), fous2[1-6]|foak3[7-9]|fopa20,
dcgmos, 
m4DCOPTS(dcgmos) m4GEMPAK,
mos/YYYYMMDDHH_gmos.gem)

match_pipe($rc(wmoid), feus2[1-6]|feak3[7-9]|fepa20,
dcxmos, 
m4DCOPTS(dcxmos) -v 2 m4GEMPAK,
mos/YYYYMMDDHH_xmos.gem)

match_pipe($rc(wmoid), ^fzxx41,
dcidft,
m4DCOPTS(dcidft) -v 2,
idft/YYYYMMDDHH.idft)

match_pipe($rc(wmoid), ^fzak41,
dcidft,
m4DCOPTS(dcidft) -v 2,
idft/YYYYMMDDHH.idak)

match_pipe($rc(fname), kwns_nwus2[02],
dcstorm, 
m4DCOPTS(dcstorm) -m 2000,
storm/sels/YYYYMMDD_sels.gem)

match_pipe($rc(fname), (kmkc_wwus40|kwns_wwus30),
dcwatch,
m4DCOPTS(dcwatch),
storm/watches/watches_YYYY_MM.gem)

match_pipe($rc(awips1), tor|svr|ffw,
dcwarn,
m4DCOPTS(dcwarn),
storm/warn/YYYYMMDDHH_warn.gem)

match_pipe($rc(wmoid), wwus(40|08)|wous20|wwus30,
dcwtch,
m4DCOPTS(dcwtch),
storm/wtch/YYYYMMDDHH_wtch.gem)

match_and_pipe($rc(station), kwns, $rc(wmoid), wwus60,
dcwcp,
m4DCOPTS(dcwcp),
storm/wcp/YYYYMMDDHH.wcp)

match_pipe($rc(wmoid), ^wwus(6[1-6]|32),
dcsvrl,
m4DCOPTS(dcsvrl),
storm/svrl/YYYYMMDDHH_svrl.gem)

dnl
dnl Hurricanes
dnl
match_pipe($rc(wmoid), ^wtpz4,
dctrop,
m4DCOPTS(dctrop),
storm/tropic/epacific/YYYY_@@.gem)

match_pipe($rc(wmoid), ^wtpn3,
dctrop,
m4DCOPTS(dctrop),
storm/tropic/wpacific/YYYY_@@.gem)

match_pipe($rc(wmoid), ^wtnt4,
dctrop,
m4DCOPTS(dctrop),
storm/tropic/atlantic/YYYY_@@.gem)

match_pipe($rc(wmoid), ^wtpa4,
dctrop,
m4DCOPTS(dctrop),
storm/tropic/cpacific/YYYY_@@.gem)

dnl
dnl dchrcn
dnl
match_and_pipe($rc(station), knhc|phfo, $rc(wmoid), ^wt(nt|pz|pa)2[1-5],
dchrcn,
m4DCOPTS(dchrcn),
storm/hrcn/YYYYMMDDHH.hrcn)

dnl
dnl Decoder for Winter Warnings, Watches and Advisories
dnl
match_pipe($rc(wmoid), ^wwus4[1-6],
dcwstm,
m4DCOPTS(dcwstm),
storm/wstm/YYYYMMDDHH.wstm)

dnl
dnl flash flood guidance
dnl
match_pipe($rc(awips1), ffg|ffh,
dcffg, 
m4DCOPTS(dcffg),
storm/ffg/YYYYMMDD_ffg.gem)

match_pipe($rc(awips1), scd,
dcscd, 
m4DCOPTS(dcscd),
scd/YYYYMMDD_scd.gem)

match_pipe($rc(fname), kawn_(xrxx84|yixx84)|_u[abdr],
dcacft, 
m4DCOPTS(dcacft),
acft/YYYYMMDDHH_acf.gem)

match_pipe($rc(fname), kkci_waus4[1-5]|_waus01,
dcairm,
m4DCOPTS(dcairm),
airm/YYYYMMDDHH_airm.gem)

match_and_pipe($rc(station), kkci|phfo|pawu|pgum|panc,
$rc(wmoid), ^w[scv](pn|nt|pa|ak),
dcisig,
m4DCOPTS(dcisig),
isig/YYYYMMDDHH_isig.gem)

match_pipe($rc(fname), kkci_w[scv]us0[1-6]|_wsus01,
dcncon,
m4DCOPTS(dcncon),
ncon/YYYYMMDDHH_sgmt.gem)

match_and_pipe($rc(station), kkci, $rc(wmoid),
wsus4[0-2]|wcpa3[1-5]|wsus3[1-3],
dccsig,
m4DCOPTS(dccsig),
csig/YYYYMMDDHH.conv)

match_pipe($rc(wmoid), ^ft,
dctaf, 
m4DCOPTS(dctaf),
taf/YYYYMMDD00.taf)

match_pipe($rc(wmoid), ^fous5[1-5],
dcrdf, 
m4DCOPTS(dcrdf) -v 4,
rdf/YYYYMMDDHH.rdf)

match_pipe($rc(awips1), wou,
dcwou,
m4DCOPTS(dcwou),
storm/wou/YYYYMMDDHHNN.wou)

dnl I have seen this decoder hang forever, and in that case it
dnl has to be killed by hand.
match_pipe($rc(awips1), wcn,
dcwcn,
m4DCOPTS(dcwcn),
storm/wcn/YYYYMMDDHHNN.wcn)

dnl Old profiler hourly summaries in BUFR format
match_pipe($rc(fname), kbou_iupt0[1-4],
dcprof,
m4DCOPTS(dcprof) -v 4 -s profiler_fsl.stn tableb tabled,
profiler_bufr/YYYYMMDD_pro.gem)
