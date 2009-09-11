dnl
dnl $Id$
dnl 

dnl This file is not used. It is copied from ncep/pqact.conf and
dnl I am keeping only for reference purposes.

match_pipe($rc(wmoid), ^s[ap],
dcmetr,
-d $gpfilter(dec_logdir)/dcmetr.log m4GEMTBL,
decoders/hrly/YYYYMMDD.hrly)

match_pipe($rc(wmoid), ^u[cefghijklmnpqswxy],
dcuair,
-d $gpfilter(dec_logdir)/dcuair.log m4GEMTBL,
decoders/uair/YYYYMMDD.snd)

match_pipe($rc(wmoid), (^s[im]v[igns])|(^snv[ins])|(^s[imn](w[kz]|[^vw])),
dclsfc,
-d $gpfilter(dec_logdir)/dclsfc.log m4GEMTBL,
decoders/syn/YYYYMMDD.syn)

match_pipe($rc(wmoid), ^fo(us14|ak2[5-9]),
dcnmos,
-d $gpfilter(dec_logdir)/dcnmos.log m4GEMTBL,
decoders/ngmmos/YYYYMMDDHH.nmos)

match_pipe($rc(wmoid), ^cxus[36],
dcscd, 
-d $gpfilter(dec_logdir)/dcscd.log m4GEMTBL,
decoders/scd/YYYYMMDD.scd)

match_pipe($rc(fname), ^fous6[1-6]_.[^w][^n][^o],
dcffg, 
-d $gpfilter(dec_logdir)/dcffg,
decoders/ffg/YYYYMMDD.ffg)

match_pipe($rc(wmoid),
^s[imn]v[^gins]|^s[imn]w[^kz]|^s(hv|hxx|s[^x])|^sx(vd|v.50|us(2[0-3]|08|40|82|86))|^y[ho]xx84,
dcmsfc,
-d $gpfilter(dec_logdir)/dcmsfc.log
decoders/ship/YYYYMMDDHH.ship)

match_pipe($rc(fname), kawn_(xrxx84|yixx84)|...._u[abdr],
dcacft, 
-d $gpfilter(dec_logdir)/dcacft.log m4GEMTBL, 
decoders/acft/YYYYMMDDHH.acf)

match_pipe($rc(wmoid), ^(w[fru]us01|w[fgu]us5[1-6]),
dcwarn,
-d $gpfilter(dec_logdir)/dcwarn.log m4GEMTBL,
decoders/warn/YYYYMMDDHH.warn)

match_pipe($rc(wmoid), ^(wwus(08|40)|wous20|wwus30),
dcwtch,
-d $gpfilter(dec_logdir)/dcwtch.log m4GEMTBL,
decoders/watch/YYYYMMDDHH.wtch)

match_pipe($rc(fname), kwns_wwus60,
dcwcp,
-d $gpfilter(dec_logdir)/dcwcp.log m4GEMTBL,
decoders/wcp/YYYYMMDDHH.wcp)

match_pipe($rc(fname), ((knhc|phfo)_wt(nt|pz|pa)2[1-5])|pgtw_wtpn3[1-5],
dchrcn,
-d $gpfilter(dec_logdir)/dchrcn.log m4GEMTBL,
decoders/hrcn/YYYYMMDDHH.hrcn)

match_and_pipe($rc(station), [^k]...|kkci|pgum|phfo, $rc(wmoid), ^w(s|cp[qa]3),
dcisig,
-v 4 -d $gpfilter(dec_logdir)/dcisig.log m4GEMTBL,
decoders/isig/YYYYMMDDHH.isig)

match_pipe($rc(wmoid), (^s[imn]v[^gins])|(^s[imn]w[^kz]),
dcmsfc,
-a 6 -d $gpfilter(dec_logdir)/dcmsfc_6hr.log m4GEMTBL,
decoders/ship6hr/YYYYMMDDHH.ship)

match_pipe($rc(wmoid), ^waus01,
dcairm,
-d $gpfilter(dec_logdir)/dcairm.log m4GEMTBL,
decoders/airm/YYYYMMDDHH.airm)

match_pipe($rc(wmoid), wsus01,
dcncon,
-d $gpfilter(dec_logdir)/dcncon.log m4GEMTBL,
decoders/ncon/YYYYMMDDHH.sgmt)

match_pipe($rc(wmoid), wsus4[0-2]|wcpa3[1-5],
dccsig,
-d $gpfilter(dec_logdir)/dccsig.log m4GEMTBL,
decoders/csig/YYYYMMDDHH.conv)

match_pipe($rc(wmoid), fous2[1-6]|foak3[7-9]|fopa20,
dcgmos, 
-d $gpfilter(dec_logdir)/dcgmos.log m4GEMTBL,
decoders/gfsmos/YYYYMMDDHH.gmos)

match_pipe($rc(wmoid), feus2[1-6]|feak3[7-9]|fepa20,
dcxmos, 
-d $gpfilter(dec_logdir)/dcxmos.log m4GEMTBL m4GEMPAK,
decoders/gfsxmos/YYYYMMDDHH.xmos)

match_pipe($rc(wmoid), wwus(32|6[1-6])
dcsvrl,
-d $gpfilter(dec_logdir)/dcsvrl.log m4GEMTBL,
decoders/svrl/YYYYMMDDHH.svrl)

match_pipe($rc(wmoid), ^uz,
dcuair,
-a 50 -m 24 -d $gpfilter(dec_logdir)/dcuair_drop.log m4GEMTBL, 
decoders/drops/YYYYMMDD.snd)

match_pipe($rc(wmoid), ^wwus4[1-6],
dcwstm,
-d $gpfilter(dec_logdir)/dcwstm.log m4GEMTBL,
decoders/wstm/YYYYMMDDHH.wstm)

match_pipe($rc(wmoid), ^ft,
dctaf, 
-d $gpfilter(dec_logdir)/dctaf.log m4GEMTBL,
decoders/taf/YYYYMMDD00.taf)

match_pipe($rc(wmoid), ^fous5[1-5],
dcrdf, 
-v 4 -d $gpfilter(dec_logdir)/dcrdf.log m4GEMTBL,
decoders/rdf/YYYYMMDDHH.rdf)

match_and_pipe($rc(station), kwns, $rc(wmoid), wous64,
dcwou,
-d $gpfilter(dec_logdir)/dcwou.log m4GEMTBL,
decoders/wou/YYYYMMDDHHNN.wou)

dnl
dnl Decoder for Watch Corner Points -
dnl I have seen this decoder hang forever, and in that case it
dnl has to be killed by hand.
dnl
match_and_pipe($rc(wmoid), wwus[46], $rc(awips1), wcn,
dcwcn,
-v 4 -d $gpfilter(dec_logdir)/dcwcn.log m4GEMTBL,
decoders/wcn/YYYYMMDDHH.wcn)

dnl
dnl PIREPS 
dnl Decoder for Pirep Observations
match_pipe($rc(wmoid), ^u(b|acn(01|10)), 
dcacft,
-v 2  -d $gpfilter(dec_logdir)/dcacft_pirep.log m4GEMTBL,
decoders/pireps/YYYYMMDDHH.acf)
