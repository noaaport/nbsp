dnl
dnl $Id$
dnl

##
## Decoders data
##

# dcmetr
match_file($rc(wmoid), ^s[ap], surface, $ymdh.sao)

# Synoptic land reports - dclsfc
match_file($rc(wmoid), (^s[im]v[igns])|(^snv[ins])|(^s[imn](w[kz]|[^vw])),
syn, $ymdh.syn)

# ship, buoy and CMAN - dcmsfc
match_file($rc(wmoid),
(^s(hv|hxx|s[^x]))|(^sx(vd|v.50|us(2[0-3]|08|40|82|86)))|(^y[ho]xx84),
ship, $ymdh.sb)

# 6 Hour Ship Observations
dnl match_file($rc(wmoid), ^s[imn]v[^gins]|^s[imn]w[^kz], ship, $ymdh.sb)
match_file($rc(wmoid), ^s[imn]v[^gins]|^s[imn]w[^kz], ship6hr, $ymdh.ship)

# Wave Observations (from the ncep pqact.conf)
match_file($rc(wmoid), ^sxvx[46], wavobs, $ymdh.wvob)

# upper air reports and dropsonde reports - dcuair
match_file($rc(wmoid), ^u[abcdefghijklmnpqrstwxy], upperair, $ymdh.upa)
match_file($rc(wmoid), ^uz, drops, $ymdh.drop)

# ngm Mos - dcnmos
match_file($rc(wmoid), ^fo(us14|ak1[34]|ak2[5-9]), mos, $ymdh.nmos)

# Decoder for GFS MOS - dcgmos
match_file($rc(wmoid), fous2[1-6]|foak3[7-9]|fopa20, mos, $ymdh.gmos)

# Decoder for GFSX MOS
match_file($rc(wmoid), feus2[1-6]|feak3[7-9]|fepa20, mos, $ymdh.xmos)

# Decoder for global sea-ice drift bulletins - dcidft
match_and_file($rc(wmoid), fzxx41, $rc(awips1), idm, idft, $ymdh.idft)

# Alaska sea ice bulletins - dcidft
match_and_file($rc(wmoid), fzak41, $rc(awips1), idm, idft, $ymdh.idak)

# SPC storm reports - dcstorm [NWUS20 (daily), NWUS22 (Hourly)]
match_file($rc(fname), kwns_nwus2[02], storm/sels, $ymdh.sels)

# Watch box coordinates - dcwatch
match_file($rc(fname), (kmkc_wwus40|kwns_wwus30), storm/watches, $ym.watches)

# dcwarn
match_file($rc(awips1), tor|svr|ffw, storm/warn, $ymdh.warn)

# dcwtch
match_file($rc(wmoid), ^wwus(40|08)|wous20|wwus30, storm/wtch, $ymdh.wtch)

# dcwcp
match_and_file($rc(wmoid), wwus60, $rc(station), kwns, storm/wcp, $ymdh.wcp)

# Watchbox outlines /pSLSxx (WWUS32 and WWUS6[1-5])
match_and_file($rc(wmoid), ^wwus(6[1-6]|32), $rc(awips1), sls,
storm/svrl, $ymdh.svrl)

# Hurricane/tropical storm positions and forecasts - dctrop
match_file($rc(wmoid), ^wtpz4, storm/tropic/epacific, $ym.$rc(wmoid))
match_file($rc(wmoid), ^wtpn3, storm/tropic/wpacific, $ym.$rc(wmoid))
match_file($rc(wmoid), ^wtnt4, storm/tropic/atlantic, $ym.$rc(wmoid))
match_file($rc(wmoid), ^wtpa4, storm/tropic/cpacific, $ym.$rc(wmoid))

# dchrcn
match_file($rc(wmoid), ^wt(nt|pz|pa)2[1-5], storm/hrcn, $ymdh.hrcn)

# Winter Warnings, Watches and Advisories - dcwstm
match_file($rc(wmoid), ^wwus4[1-6], storm/wstm, $ymdh.wstm)

# flash flood watches - dcffa
match_file($rc(awips1), ffa, storm/ffa, $ymdh.ffa)

# flash flood guidance - dcffg
match_file($rc(awips1), ffg|ffh, storm/ffg, $ymd.ffg)

# Supplemental Climatological Data (SCD) - dcscd
match_file($rc(awips1), scd, scd, $ymd.scd)

# Aircraft Observations - dcacft
match_file($rc(fname), kawn_(xrxx84|yixx84)|_u[abdr], acft, $ymdh.acf)

# Airmets - dcairm
match_file($rc(fname), kkci_waus4[1-5]|_waus01, airm, $ymdh.airm)

# International Sigmets - dcisig
match_and_file($rc(station), kkci|phfo|pawu|pgum|panc,
$rc(wmoid), ^w[scv](pn|nt|pa|ak),
isig, $ymdh.isig)

# Non-convective Sigmets - dcncon
match_and_file($rc(station), kkci, $rc(wmoid), wsus01|w[scv]us0[1-6],
ncon, $ymdh.sgmt)

# Convective Sigmets - dccsig
match_and_file($rc(station), kkci,
$rc(wmoid), wsus4[0-2]|wcpa3[1-5]|wsus3[1-3],
csig, $ymdh.conv)

# TAFs - dctaf
match_file($rc(wmoid), ^ft, taf, $ymdh.taf)

# Regional Digital Forecasts/Point Forecast Matrices - dcrdf
match_file($rc(wmoid), ^fous5[1-5], rdf, $ymdh.rdf)

# watch outline updates - dcwou
match_file($rc(awips1), wou, storm/wou, ${ymd_hm}.wou)
match_file($rc(awips1), wcn, storm/wcn, ${ymd_hm}.wcn)

##
## End of decoders data
##
