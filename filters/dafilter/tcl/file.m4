dnl
dnl $Id$
dnl
dnl
dnl front, forecast, day1, day2. Since they are not hourly, we
dnl produce a "latest" copy for da clients.

#
# NWX datasets 
#

# Average Monthly Weather outlook (LOCAL)
matchstop_file($rc(wmoid), ^feus8[0-9], nwx/extend, $ymdh.f30)

# Fire Weather bulletins
matchstop_file($rc(awips) s s1, fwddy(.), nwx/fire/fwd, $ymdh.fwddy$s1)

# SPC Products
# Severe Weather Outlook (Day 1-3) ACUS0[1-3]
match_file_noappend($rc(awips) s s1, swody([0-9]), nwx/spc/day$s1, latest)
matchstop_file($rc(awips) s s1, swody([0-9]), nwx/spc/day$s1, $ymdh.day$s1)

# Convective Outlook Areal Outline (Day 1-3) WUUS0[1-3] KWNS
matchstop_file($rc(awips) s s1, ptsdy([0-9]),
`nwx/spc/day$s1', `$ymdh.ptsdy$s1')

# Thunderstorm/Tornado Watch Areas + Discussion
matchstop_file($rc(wmoid), ^wwus20, nwx/spc/watch, $ymdh.watch)

# Thunderstorm/Tornado Watch Areas
matchstop_file($rc(wmoid), ^wwus30, nwx/spc/watch, $ymdh.wtch2)

# Watch outline update (WOU)
matchstop_file($rc(awips1), ^wou, nwx/spc/wou, $ymdh.wou)

# Public Outlook
matchstop_file($rc(wmoid), ^wous40, nwx/spc/public, $ymdh.public)

# Severe Weather Summary
match_and_file($rc(station), kwns, $rc(wmoid), nwus2[02],
nwx/spc/svr_summ, $ymdh.svr)

# Hourly Status Report
matchstop_file($rc(awips), stahry, nwx/spc/stahry, $ymdh.hry)

# Daily Status Report
matchstop_file($rc(awips), stadts, nwx/spc/stadts, $ymdh.dts)

# Tornado Totals and Related Deaths (local addition)
matchstop_file($rc(awips), stamts, nwx/spc/stamts, $ymdh.mts)

# Mesoscale Discussion
matchstop_file($rc(wmoid), ^acus11, nwx/spc/meso, $ymdh.meso)

# Hazardous Weather
matchstop_file($rc(wmoid), ^wwus44, nwx/spc/hzrd, $ymdh.hzrd)

# Status Report
matchstop_file($rc(wmoid), ^wous20, nwx/spc/status, $ymdh.stat)

# Watch County List
matchstop_file($rc(awips), ^sev[0-9], nwx/spc/sev, $ymdh.sev)

# Watch Summary
matchstop_file($rc(awips), sevspc, nwx/spc/sevmkc, $ymdh.sevmkc)

# International Temp/Precip Summary
matchstop_file($rc(wmoid), ^abus2[3-6]|abxx0[567]|abca01|fpcn60|wbcn02|abcn01,
nwx/spc/tp_summ, $ymdh.tp_summ)

# Outlook Points Product
matchstop_file($rc(wmoid) s s1, ^wwus0([1-3]),
nwx/spc/otlkpts, `$ymdh.ptsdy$s1')

# Earthquake and Tsunami messages/warnings
matchstop_file($rc(wmoid), ^we, nwx/seismic, $ymdh.tsuww)
matchstop_file($rc(wmoid), ^se, nwx/seismic, $ymdh.info)

#
# NHC Products
#

# Outlooks
match_file($rc(fname), knhc_(abnt20|abpz20),
nwx/nhc/outlook, $ymdh.outlk)
match_file($rc(fname), phfo_acpn50, nwx/nhc/outlook, $ymdh.outlk)
match_file($rc(fname), tjsj_abca33, nwx/nhc/outlook, $ymdh.outlk)
match_file($rc(fname), pgtw_abpw10, nwx/nhc/outlook, $ymdh.outlk)

# Discussions
match_file($rc(fname), knhc_(wtnt|wtpz)4[1-5], nwx/nhc/disc, $ymdh.disc)
match_file($rc(fname), phfo_wtpa4[1-5], nwx/nhc/disc, $ymdh.disc)
match_file($rc(fname), pgtw_w[dt]pn3[1-5], nwx/nhc/disc, $ymdh.disc)

# Public Forecasts
match_file($rc(fname), knhc_(wtnt|wtpz)3[1-5], nwx/nhc/public, $ymdh.pblc)
match_file($rc(fname), phfo_wtpa3[1-5], nwx/nhc/public, $ymdh.pblc)
match_file($rc(fname), tjsj_wtca40, nwx/nhc/public, $ymdh.pblc)
match_file($rc(fname), tfff_whca31, nwx/nhc/public, $ymdh.pblc)

# Marine Forecasts
match_file($rc(fname), knhc_wt(nt|pz)2[1-5], nwx/nhc/marine, $ymdh.mar)
match_file($rc(fname), phfo_wtpa2[1-5], nwx/nhc/marine, $ymdh.mar)
match_file($rc(fname), nffn_w(h|t)ps01, nwx/nhc/marine, $ymdh.mar)

# Model Forecasts
match_file($rc(fname), kmia_whxx01, nwx/nhc/model, $ymdh.mdl)
match_file($rc(fname), kwbc_whxx(0[1-4]|9[09]),
nwx/nhc/model, $ymdh.mdl)

# Recon Flights
match_file($rc(fname), (knhc_urnt12|kwbc_u[rz]nt14),
nwx/nhc/recon, $ymdh.rcn)

# Tropical Discussions
match_file($rc(fname), knhc_ax(nt|pz)20, nwx/nhc/tdsc, $ymdh.tdsc)

# Strike probabilities
match_file($rc(fname), knhc_wt(nt|pz)7[1-5], nwx/nhc/probs, $ymdh.probs)

#
# Tropical Pacific Products
#

# Tropical Weather Outlook & Summary
matchstop_file($rc(fname), phfo_acpn50, nwx/tropical/trpwxou, $ymdh.trpwxou)

# Tropical Weather Summary
matchstop_file($rc(fname), phfo_acpn60, nwx/tropical/trpwxsu, $ymdh.trpwxsu)

# Tropical Weather Discussion
matchstop_file($rc(fname), phfo_acp[aw]40,
nwx/tropical/trpwxdi, $ymdh.trpwxdi)

# Marine/Aviation Tropical Cyclone Advisory
matchstop_file($rc(fname), phfo_wtpa2[1-5],
nwx/tropical/maravnt, $ymdh.maravnt)

# Public Tropical Cyclone Advisory
matchstop_file($rc(fname), phfo_wtpa3[1-5],
nwx/tropical/pubtrpc, $ymdh.pubtrpc)

# Tropical Cyclone Discussion
matchstop_file($rc(fname), phfo_wtpa4[1-5],
nwx/tropical/trpcycd, $ymdh.trpcycd)

# Tropical Cyclone Position Estimate
matchstop_file($rc(fname), phfo_wtpa50, nwx/tropical/trpcycp, $ymdh.trpcycp)

# Tropical Cyclone Update
matchstop_file($rc(fname), phfo_wtpa60, nwx/tropical/trpcycu, $ymdh.trpcycu)

# Unnumbered Depression and Suspicious Area Advisory
matchstop_file($rc(fname), phfo_(acpa80|fzhw50),
nwx/tropical/unnumdp, $ymdh.unnumdp)

#
# Additional Recon Flights (USAF)
#

# Plan of the Day
matchstop_file($rc(fname), knhc_nous42, nwx/nhc/recon, $ymdh.pod)

# Tropical RECO (Atlantic)
matchstop_file($rc(wmoid), urnt11, nwx/nhc/recon, $ymdh.areco)

# Vortex Msg (Atlantic)
matchstop_file($rc(wmoid), urnt12, nwx/nhc/recon, $ymdh.avortex)

# Supp Vortex Msg (Atlantic)
matchstop_file($rc(wmoid), urnt14, nwx/nhc/recon, $ymdh.asupvort)

# Non-tropical RECO (Pacific)
matchstop_file($rc(wmoid), urpn10, nwx/nhc/recon, $ymdh.pntreco)

# Tropical RECO (Pacific)
matchstop_file($rc(wmoid), urpn11, nwx/nhc/recon, $ymdh.preco)

# Supp Vortex Msg (Pacific)
matchstop_file($rc(wmoid), urpn14, nwx/nhc/recon, $ymdh.psupvort)

# Drops (Atlantic)
matchstop_file($rc(wmoid), uznt13, nwx/nhc/recon, $ymdh.adrops)

# Drops (E Pacific)
matchstop_file($rc(wmoid), uzpn13, nwx/nhc/recon, $ymdh.pdrops)

# Drops (E Pacific) (I believe it is Western Pacific)
matchstop_file($rc(wmoid), uzpa13, nwx/nhc/recon, $ymdh.wpdrops)

# Hold recon pact stuff (testing)
matchstop_file($rc(fname), kbix_urnt1[13]|pgtw_(urpa11|urpa1[0-2]),
nwx/nhc/recon, $ymdh.tst)
matchstop_file($rc(wmoid), sxxx50, nwx/nhc/recon, $ymdh.tst)

#
# Flash Flood Guidance
#

# Satellite Precipitaion Estimates
matchstop_file($rc(wmoid), ^txus20, nwx/flood/satest, $ymdh.satest)

# Coast Guard Reports (SXUS08, SXUS86, SXUS40)
matchstop_file($rc(awips1), cgr, nwx/marine/cguard, $ymdh.cgr)

#
# Aviation Forecasts
#

# Area Forecasts
matchstop_file($rc(wmoid), faus0[1-5]|fahw0[1-5]|fahw31|faak0[1-5],
nwx/old/aviation/area, $ymdh.area)

# Convective SIGMETS
matchstop_file($rc(fname), kkci_wsus3[1-3]|_wsus4[012],
nwx/aviation/conv, $ymdh.conv)
matchstop_file($rc(fname), phfo_wcpa3[1-5], nwx/aviation/conv, $ymdh.conv)

# International SIGMETS
match_and_file($rc(station), ^k, $rc(wmoid), ^ws,
nwx/aviation/intlsig, $ymdh.intl)
match_and_file($rc(station), kkci|phfo|pawu, $rc(wmoid), ^w[scv](pn|nt|pa|ak),
nwx/aviation/intlsig, $ymdh.intl)

# SIGMETS
matchstop_file($rc(wmoid), ^ws[^u]|^wsuk|wsus01,
nwx/aviation/sigmet, $ymdh.sgmt)
match_and_file($rc(station), kkci, $rc(wmoid), w[scv]us0[1-6],
nwx/aviation/sigmet, $ymdh.sgmt)

# AIRMETS
matchstop_file($rc(wmoid), waus01|wahw[03]1|waak01,
nwx/old/aviation/airmet, $ymdh.airm)
match_and_file($rc(station), kkci, $rc(wmoid), waus4[1-5],
nwx/old/aviation/airmet, $ymdh.airm)

# Offshore Area
matchstop_file($rc(wmoid), fant02, nwx/aviation/offshore, $ymdh.offsh)

# CWA
matchstop_file($rc(fname), kzan_faak2[1-6], nwx/aviation/cwa, $ymdh.cwa)
matchstop_file($rc(fname), (kz..)_(faus2[1-6]|wcus2[1-6]),
nwx/aviation/cwa, $ymdh.cwa)

# Meteorological Impact Statements (MIS)
matchstop_file($rc(fname), ^(kz..|panc)_faus20, nwx/aviation/mis, $ymdh.mis)

#
# MOS
#

# NGM MOS
matchstop_file($rc(wmoid), ^fous14|foak1[34], nwx/mos/ngm, $ymdh.ngmmos) 

# NGM City Guidance -- US, Canada, and the Gulf of Mexico
matchstop_file($rc(wmoid), ^(fou[emw][6-9]|focn7|fogx),
nwx/mos/ngm, $ymdh.ngmgd)
match_file($rc(fname), kwno_fous(8[6-9]|90), nwx/mos/ngm, $ymdh.ngmgd)

# ETA City Guidance -- US, Canada, and the Gulf of Mexico
matchstop_file($rc(fname), kwno_fous67, nwx/mos/eta, $ymdh.etagd)

# GFS Marine
matchstop_file($rc(fname), kwno_fqus2[1-6], nwx/mos/marine, $ymdh.marnmos)

#
# HPC Discussions
#

# Model Discussion
matchstop_file($rc(awips), pmdhmd, nwx/hpc/prog, $ymdh.disc)

# Extended Forecast Discussion
matchstop_file($rc(wmoid), ^fxus02, nwx/hpc/extend, $ymdh.extend)

# Hemispheric Map Discussion AND 500mb Map Type Correlations
matchstop_file($rc(wmoid), fxus03, nwx/hpc/hemi, $ymdh.hemi)

# NMC Prognostic Discussion (Basic Weather)
matchstop_file($rc(wmoid), ^fx(us01|us10|ca20|sa20|hw01),
nwx/hpc/prog, $ymdh.prog)

# Quantitative Precipitation Forecast Discussion
matchstop_file($rc(wmoid), fxus04, nwx/hpc/qpf, $ymdh.qpf)

# Quantitative Precipitation Forecast Excessive Rainfall Discussion
matchstop_file($rc(wmoid), fous30, nwx/hpc/qpf, $ymdh.qpferp)

# Heavy Snowfall Discussion
matchstop_file($rc(wmoid), fous11, nwx/hpc/hvysnow, $ymdh.hvysnow)

# Coded Fronts Analysis
match_file_noappend($rc(wmoid), asus01, nwx/hpc/fronts, latest)
matchstop_file($rc(wmoid), asus01, nwx/hpc/fronts, $ymdh.front)

# Coded Fronts Forecast
match_file_noappend($rc(wmoid), fsus02, nwx/hpc/forecast, latest)
matchstop_file($rc(wmoid), fsus02, nwx/hpc/forecast, $ymdh.fcst)

# Extended Pacific Forecast
matchstop_file($rc(wmoid), fxpa00, nwx/hpc/expac, $ymdh.expac)

# Heat Index - Mean
matchstop_file($rc(awips), prb[ew]hi, nwx/hpc/heat, $ymdh.hmean)

# Heat Index - Max
matchstop_file($rc(awips), prb[ew]hh, nwx/hpc/heat, $ymdh.hmax)

# Heat Index - Min
matchstop_file($rc(awips), prb[ew]hl, nwx/hpc/heat, $ymdh.hmin)

# SDM Messages
matchstop_file($rc(fname), (kwbc|kwno)_nous42, nwx/hpc/sdm, $ymdh.sdm)

# International Messages
matchstop_file($rc(fname), (kwbc|kwno)_npxx10, nwx/hpc/intl, $ymdh.intl)

# Storm Summaries
matchstop_file($rc(wmoid), acus4[1-5], nwx/hpc/storm, $ymdh.storm)

#
# CPC Products
#

# 6-10 Day Forecast
matchstop_file($rc(fname), kwbc_feus40, nwx/cpc/6_10_fcst, $ymdh.f610)

# 6-10 Day Narative
matchstop_file($rc(fname), kwbc_fxus06, nwx/cpc/6_10_nrtv, $ymdh.n610)

# 30 Day Narative
matchstop_file($rc(fname), kwbc_fxus07, nwx/cpc/30_nrtv, $ymdh.n30)

# 90 Day Narative
matchstop_file($rc(fname), kwbc_fxus05, nwx/cpc/90_nrtv, $ymdh.n90)

# 30 & 90 Day Narative for Hawaii
matchstop_file($rc(fname), kwbc_fxhw40, nwx/cpc/hawaii, $ymdh.hawaii)

# Threats Assessment Discussion
matchstop_file($rc(fname), kwnc_fxus21, nwx/cpc/threats, $ymdh.threats)

# U.S. Drought Monitor Analysis Discussion
matchstop_file($rc(fname), kwnc_fxus25, nwx/cpc/drought, $ymdh.drought)

#
# Volcano Products
#

# Volcanic Ash Advisory Statements
match_file($rc(wmoid), (fvxx2|fvcn0|fvau0)[0-4],
nwx/volcano/volcano, $ymdh.volc)

# Volcano Warnings/SIGMETS
match_file($rc(wmoid), ^wv, nwx/volcano/volcwarn, $ymdh.vlcw)

# Volcano Ash Forecast/Avalanch Forecast
match_file($rc(wmoid), fvus2[01], nwx/volcano/volcfcst, $ymdh.vlcf)

# Alerts and Administrative Messages
matchstop_file($rc(awips1), (ad(a|m|r|x)|ini),
nwx/admin/$rc(awips1), $ymdh.$rc(awips1))

# Agricultural products
matchstop_file($rc(awips1), agf|ago|fwl|saf|wcr|wda,
nwx/ag_prod/$rc(awips1), $ymdh.$rc(awips1))

# Air Quality Products
matchstop_file($rc(awips1), apg|aqi|asa,
nwx/air_qual/$rc(awips1), $ymdh.$rc(awips1))

# ASOS summaries
matchstop_file($rc(awips1), rr[67], nwx/asos/$rc(awips1), $ymdh.$rc(awips1))

# Avalanche
matchstop_file($rc(awips1), ava|avm|avw|sab|sag|swe|wsw,
nwx/avalanche/$rc(awips1), $ymdh.$rc(awips1))

# Aviation
matchstop_file($rc(awips1), aww|oav|rfr|sad|sam|sig|wst|wsv,
nwx/aviation/$rc(awips1), $ymdh.$rc(awips1))
matchstop_file($rc(awips1), fa[0-9], nwx/aviation/area, $ymdh.area)
matchstop_file($rc(awips1), wa[0-9], nwx/aviation/airmet, $ymdh.airm)
matchstop_file($rc(awips1), w[scv][0-9], nwx/aviation/sigmet, $ymdh.sgmt)

# Civil Advisory Products
matchstop_file($rc(awips1),
awg|cae|cdw|cem|evi|hmw|lae|lew|nuw|rhw|sds|spw|sto|toe,
nwx/civil_advs/$rc(awips1), $ymdh.$rc(awips1))

# Climate
matchstop_file($rc(awips1), cf6|cli|clm|cmm,
nwx/climate/$rc(awips1), $ymdh.$rc(awips1))

# Fire
matchstop_file($rc(awips1),
fdi|frw|fwa|fwe|fwf|fwm|fwn|fwo|fws|fww|pbf|rfd|rfw|smf,
nwx/fire/$rc(awips1), $ymdh.$rc(awips1))

# Flood/Flash Flood Products
match_file($rc(awips1), esf|ffa|ffg|ffh|ffs|fln|fls|ffw|flw,
nwx/flood/$rc(awips1), $ymdh.$rc(awips1))

# Coded analysis/forecast
match_and_file($rc(station), kwbc, $rc(awips1), cod,
nwx/fronts/$rc(awips1), $ymdh.$rc(awips1))

# Hydrometeorological Messages/Discussions/Products
matchstop_file($rc(awips1), hcm|hmd|hyd|hym|hyw|rr[1-9am],
nwx/hydro/$rc(awips1), $ymdh.$rc(awips1))

# Coastal/Great Lakes/Offshore/Ice Marine Products
matchstop_file($rc(awips1), 
cfw|cwf|glf|gls|hsf|ice|iob|lsh|maw|mim|mrp|mvf|mws|mww|nsh|ocd|off|omr|osw|pls|smw|srf|tid, 
nwx/marine/$rc(awips1), $ymdh.$rc(awips1))

#
# Miscellaneous Products
#

# ESG - Extended_Streamflow_Guidance
# ESP - Extended_Streamflow_Prediction
# ESS - Water_Supply_Outlook
# GSM - General_Status_Message
# STQ - Spot_Forecast_Request
# TWB - Transcribed_Weather_Broadcast
matchstop_file($rc(awips1), esg|esp|ess|gsm|stq|twb,
nwx/misc/$rc(awips1), $ymdh.$rc(awips1))

# PMD - Prognostic_Meteorological_Discussion (Basic Weather)
matchstop_file($rc(awips), pmd(ca|hi|sa|epd|ep[3-7]|spd|thr|hmd)|preepd,
nwx/misc/$rc(awips), $ymdh.$rc(awips))

# EFP - 3_to_5_Day_Extended_Forecast
matchstop_file($rc(awips1), efp, nwx/extend/$rc(awips1), $ymdh.$rc(awips1))

# Extended Forecast Discussion
matchstop_file($rc(wmoid), fxus02, nwx/extend, $ymdh.extend)

# Hemispheric Map Discussion AND 500mb Map Type Correlations
matchstop_file($rc(wmoid), fxus03, nwx/hemi, $ymdh.hemi)

#
# Model Products
#
matchstop_file($rc(awips1),
fan|fmr|foh|frh|ftj|ftp|fwc|fzl|mav|met|mex|pfm|rdf|rdg,
nwx/model/$rc(awips1), $ymdh.$rc(awips1))

#
# Non-Weather Events Productsdnl non_weather_events
#
# EQR - Earthquake_report
# EQW - Earthquake_warning
matchstop_file($rc(awips1), eqr|eqw,
nwx/non_wx_events/$rc(awips1), $ymdh.$rc(awips1))

# Observation Data Products
matchstop_file($rc(awips1), lco|scd|taf,
nwx/obs_data/$rc(awips1), $ymdh.$rc(awips1))

# Outlook Products
matchstop_file($rc(awips1), eol|eon|hwo,
nwx/outlook/$rc(awips1), $ymdh.$rc(awips1))

# Precipitation Products
matchstop_file($rc(awips1), map, nwx/precip/$rc(awips1), $ymdh.$rc(awips1))

# Public products
matchstop_file($rc(awips1),
afd|afm|afp|aws|awu|ccf|lfp|lsr|mis|now|opu|pns|rec|rer|rtp|rws|rzf|scc|scs|sfp|sft|sls|sps|stp|swr|tav|tpt|tvl|wcn|wvm|zfp,
nwx/pub_prod/$rc(awips1), $ymdh.$rc(awips1))

#
# Quantitative Precipitation Forecast Products
#
# QPF - QPF - the PTR, RSA and STR products
matchstop_file($rc(awips), qpf(ptr|rsa|str), nwx/qpf/qpf, $ymdh.qpf)

# QPFERD - QPF - Excessive Rainfall
# QPFHSD - QPF - Heavy Snowfall
# QPFPFD - QPF - Quantitative precipitation forecast discussion
matchstop_file($rc(awips), qpferd|qpfhsd|qpfpfd,
nwx/qpf/$rc(awips), $ymdh.$rc(awips))

# QPS - Quantitavive_Precipitation_Statement
matchstop_file($rc(awips1), qps, nwx/qpf/$rc(awips1), $ymdh.$rc(awips1))

# Radar Products
# FTM - WSR88_Radar_Outage_Notification/Free_Text_Message
# MRM - Missing_Radar_Message_(WSR_88D)
matchstop_file($rc(awips1), ftm|mrm,
nwx/radar/$rc(awips1), $ymdh.$rc(awips1))

# River Products
matchstop_file($rc(awips1), rva|rvd|rvf|rvi|rvm|rvr|rvs, 
nwx/river/$rc(awips1), $ymdh.$rc(awips1))

# Satellite Products
matchstop_file($rc(awips1), scp|scv|sim,
nwx/satellite/$rc(awips1), $ymdh.$rc(awips1))

# Space Products
matchstop_file($rc(awips1), mon, nwx/space/$rc(awips1), $ymdh.$rc(awips1))

# Statistical Products
matchstop_file($rc(awips1), mef|par|pvm|sta|ver,
nwx/stats/$rc(awips1), $ymdh.$rc(awips1))

# Tropical Products
match_file($rc(awips1),
chg|dsa|hls|psh|tca|tcd|tce|tcm|tcp|tcs|tcu|tma|tsm|tsu|twd|two|tws,
nwx/tropical/$rc(awips1), $ymdh.$rc(awips1))

# Ultraviolet Index Products
matchstop_file($rc(awips1), uvi, nwx/uvi/$rc(awips1), $ymdh.$rc(awips1))

# Volcano Products
matchstop_file($rc(awips1), vaa|vow,
nwx/volcano/$rc(awips1), $ymdh.$rc(awips1))

# Watch and Warning Products
match_file($rc(awips1), npw|svr|svs|tor,
nwx/watch_warn/$rc(awips1), $ymdh.$rc(awips1))

# WSW - Winter_Weather_Warnings, watches, advisories
# FFW, FLW - Flash flood and flood warning
match_file($rc(awips1), wsw|ffw|flw,
nwx/watch_warn/$rc(awips1), $ymdh.$rc(awips1))
