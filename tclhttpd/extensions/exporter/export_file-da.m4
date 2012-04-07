#
# $Id$
#
#
# NWX datasets 
#

# Average Monthly Weather outlook (LOCAL)
export4(Average Monthly Weather Outlook, nwx/extend, f30)

# Fire Weather bulletins
export4(Fire Weather, nwx/fire/fwd, fwddy1)
export4(Fire Weather, nwx/fire/fwd, fwddy2)

# SPC Products
# Severe Weather Outlook (Day 1-3) ACUS0[1-3]
export4(Severe Weather Outlook Day 1, nwx/spc/day1, day1)
export4(Severe Weather Outlook Day 1, nwx/spc/day2, day2)
export4(Severe Weather Outlook Day 1, nwx/spc/day3, day3)

# Convective Outlook Areal Outline (Day 1-3) WUUS0[1-3] KWNS
export4(Convective Outlook Areal Outline Day 1, nwx/spc/day1, ptsdy1)
export4(Convective Outlook Areal Outline Day 2, nwx/spc/day2, ptsdy2)
export4(Convective Outlook Areal Outline Day 3, nwx/spc/day3, ptsdy3)

# Thunderstorm/Tornado Watch Areas + Discussion
export4(Thunderstorm/Tornado Watch Discussion, nwx/spc/watch, watch)

# Thunderstorm/Tornado Watch Areas
export4(Thunderstorm/Tornado Watch Areas, nwx/spc/watch, wtch2)

# Watch outline update (WOU)
export(Watch Outline Update, nwx/spc/wou, wou)

# Public Outlook
export(Public Outlook, nwx/spc/public, public)

# Severe Weather Summary
export(Severe Weather Summary, nwx/spc/svr_summ, svr)

# Hourly Status Report
export(Hourly Status Report, nwx/spc/stahry, hry)

# Daily Status Report
export(Daily Status Report, nwx/spc/stadts, dts)

# Tornado Totals and Related Deaths (local addition)
export(Tornado Totals and Related Deaths, nwx/spc/stamts, mts)

# Mesoscale Discussion
export(Mesoscale Discussion, nwx/spc/meso, meso)

# Hazardous Weather
export(Hazardous Weather, nwx/spc/hzrd, hzrd)

# Status Report
export(Status Report, nwx/spc/status, stat)

# Watch County List
export(Watch County List, nwx/spc/sev, sev)

# Watch Summary
export(Watch Summary, nwx/spc/sevmkc, sevmkc)

# International Temp/Precip Summary
export(International Temp/Precip Summary, nwx/spc/tp_summ, tp_summ)

# Outlook Points Product
export4(Outlook Points Product 1, nwx/spc/otlkpts, ptsdy1)
export4(Outlook Points Product 2, nwx/spc/otlkpts, ptsdy2)
export4(Outlook Points Product 3, nwx/spc/otlkpts, ptsdy3)

# Earthquake and Tsunami messages/warnings
export4(Earthquake and Tsunami warnings, nwx/seismic, tsuww)
export4(Earthquake and Tsunami messages, nwx/seismic, info)

#
# NHC Products
#

# Outlooks
export(Outlooks, nwx/nhc/outlook, outlk)

# Discussions
export(Discussions, nwx/nhc/disc, disc)

# Public Forecasts
export(Public Forecasts, nwx/nhc/public, pblc)

# Marine Forecasts
export(Marine Forecasts, nwx/nhc/marine, mar)

# Model Forecasts
export(Model Forecasts, nwx/nhc/model, mdl)

# Recon Flights
export4(Recon Flights, nwx/nhc/recon, rcn)

# Tropical Discussions
export(Tropical Discussions, nwx/nhc/tdsc, tdsc)

# Strike probabilities
export(Strike probabilities, nwx/nhc/probs, probs)

#
# Tropical Pacific Products
#

# Tropical Weather Outlook & Summary
export(Tropical Weather Outlook and Summary, nwx/tropical/trpwxou, trpwxou)

# Tropical Weather Summary
export(Tropical Weather Summary, nwx/tropical/trpwxsu, trpwxsu)

# Tropical Weather Discussion
export(Tropical Weather Discussion, nwx/tropical/trpwxdi, trpwxdi)

# Marine/Aviation Tropical Cyclone Advisory
export(Marine/Aviation Tropical Cyclone Advisory,
nwx/tropical/maravnt, maravnt)

# Public Tropical Cyclone Advisory
export(Public Tropical Cyclone Advisory, nwx/tropical/pubtrpc, pubtrpc)

# Tropical Cyclone Discussion
export(Tropical Cyclone Discussion, nwx/tropical/trpcycd, trpcycd)

# Tropical Cyclone Position Estimate
export(Tropical Cyclone Position Estimate, nwx/tropical/trpcycp, trpcycp)

# Tropical Cyclone Update
export4(Tropical Cyclone Update, nwx/tropical/trpcycu, trpcycu)

# Unnumbered Depression and Suspicious Area Advisory
export4(Unnumbered Depression and Suspicious Area Advisory,
nwx/tropical/unnumdp, unnumdp)

#
# Additional Recon Flights (USAF)
#

# Plan of the Day
export4(Plan of the Day, nwx/nhc/recon, pod)

# Tropical RECO (Atlantic)
export4(Tropical RECO Atlantic, nwx/nhc/recon, areco)

# Vortex Msg (Atlantic)
export4(Vortex Msg Atlantic, nwx/nhc/recon, avortex)

# Supp Vortex Msg (Atlantic)
export4(Supp Vortex Msg Atlantic, nwx/nhc/recon, asupvort)

# Non-tropical RECO (Pacific)
export4(Non-tropical RECO Pacific, nwx/nhc/recon, pntreco)

# Tropical RECO (Pacific)
export4(Tropical RECO Pacific, nwx/nhc/recon, preco)

# Supp Vortex Msg (Pacific)
export4(Supp Vortex Msg Pacific, nwx/nhc/recon, psupvort)

# Drops (Atlantic)
export4(Drops Atlantic, nwx/nhc/recon, adrops)

# Drops (E Pacific)
export4(Drops E Pacific, nwx/nhc/recon, pdrops)

# Drops (E Pacific) (I believe it is Western Pacific)
export4(Drops W Pacicif, nwx/nhc/recon, wpdrops)

# Hold recon pact stuff (testing)
export4(Hold recon pact stuff (testing), nwx/nhc/recon, tst)

#
# Flash Flood Guidance
#

# Satellite Precipitaion Estimates
export(Satellite Precipitaion Estimates, nwx/flood/satest, satest)

# Coast Guard Reports (SXUS08, SXUS86, SXUS40)
export(Coast Guard Reports, nwx/marine/cguard, cgr)

#
# Aviation Forecasts
#

# Area Forecasts
export(Area Forecasts, nwx/old/aviation/area, area)

# Convective Sigmets
export(Convective SIGMETS, nwx/aviation/conv, conv)

# International Sigmets
export(International SIGMETS, nwx/aviation/intlsig, intl)

# SIGMETS
export4(Sigmets, nwx/aviation/sigmet, sgmt)

# AIRMETS
export(Airmets, nwx/old/aviation/airmet, airm)

# Offshore Area
export(Offshore Area, nwx/aviation/offshore, offsh)

# CWA
export(CWA, nwx/aviation/cwa, cwa)

# Meteorological Impact Statements (MIS)
export(Meteorological Impact Statements, nwx/aviation/mis, mis)

#
# MOS
#

# NGM MOS
export4(NGM MOS, nwx/mos/ngm, ngmmos) 

# NGM City Guidance -- US, Canada, and the Gulf of Mexico
export4(NGM City Guidance, nwx/mos/ngm, ngmgd)

# ETA City Guidance -- US, Canada, and the Gulf of Mexico
export(ETA City Guidance, nwx/mos/eta, etagd)

# GFS Marine
export(GFS Marine, nwx/mos/marine, marnmos)

#
# HPC Discussions
#

# Model Discussion
export4(Model Discussion, nwx/hpc/prog, disc)

# Extended Forecast Discussion
export(Extended Forecast Discussion, nwx/hpc/extend, extend)

# Hemispheric Map Discussion and 500mb Map Type Correlations
export(Hemispheric Map Discussion and 500mb Map Type Correlations,
nwx/hpc/hemi, hemi)

# NMC Prognostic Discussion (Basic Weather)
export4(NMC Prognostic Discussion (Basic Weather), nwx/hpc/prog, prog)

# Quantitative Precipitation Forecast Discussion
export4(Quantitative Precipitation Forecast Discussion, nwx/hpc/qpf, qpf)

# Quantitative Precipitation Forecast Excessive Rainfall Discussion
export4(Quantitative Precipitation Forecast Excessive Rainfall Discussion,
nwx/hpc/qpf, qpferp)

# Heavy Snowfall Discussion
export(Heavy Snowfall Discussion, nwx/hpc/hvysnow, hvysnow)

# Coded Fronts Analysis
export(Coded Fronts Analysis, nwx/hpc/fronts, front)

# Coded Fronts Forecast
export(Coded Fronts Forecast, nwx/hpc/forecast, fcst)

# Extended Pacific Forecast
export(Extended Pacific Forecast, nwx/hpc/expac, expac)

# Heat Index - Mean
export4(Heat Index - Mean, nwx/hpc/heat, hmean)

# Heat Index - Max
export4(Heat Index - Max, nwx/hpc/heat, hmax)

# Heat Index - Min
export4(Heat Index - Min, nwx/hpc/heat, hmin)

# SDM Messages
export(SDM Messages, nwx/hpc/sdm, sdm)

# International Messages
export(International Messages, nwx/hpc/intl, intl)

# Storm Summaries
export(Storm Summaries, nwx/hpc/storm, storm)

#
# CPC Products
#

# 6-10 Day Forecast
export(6-10 Day Forecast, nwx/cpc/6_10_fcst, f610)

# 6-10 Day Narative
export(6-10 Day Narative, nwx/cpc/6_10_nrtv, n610)

# 30 Day Narative
export(30 Day Narative, nwx/cpc/30_nrtv, n30)

# 90 Day Narative
export(90 Day Narative, nwx/cpc/90_nrtv, n90)

# 30 & 90 Day Narative for Hawaii
export(30 and 90 Day Narative for Hawaii, nwx/cpc/hawaii, hawaii)

# Threats Assessment Discussion
export(Threats Assessment Discussion, nwx/cpc/threats, threats)

# U.S. Drought Monitor Analysis Discussion
export(U.S. Drought Monitor Analysis Discussion, nwx/cpc/drought, drought)

#
# Volcano Products
#

# Volcanic Ash Advisory Statements
export(Volcanic Ash Advisory Statements, nwx/volcano/volcano, volc)

# Volcano Warnings/SIGMETS
export(Volcano Warnings/Sigmets, nwx/volcano/volcwarn, vlcw)

# Volcano Ash Forecast/Avalanch Forecast
export(Volcano Ash Forecast/Avalanch Forecast, nwx/volcano/volcfcst, vlcf)

# Alerts and Administrative Messages
export(Alerts and Administrative Messages, nwx/admin/ada, ada)
export(Alerts and Administrative Messages, nwx/admin/adm, adm)
export(Alerts and Administrative Messages, nwx/admin/adr, adr)
export(Alerts and Administrative Messages, nwx/admin/adx, adx)
export(Alerts and Administrative Messages, nwx/admin/ini, ini)

# Agricultural products
export(Agricultural Products, nwx/ag_prod/agf, agf)
export(Agricultural Products, nwx/ag_prod/ago, ago)
export(Agricultural Products, nwx/ag_prod/fwl, fwl)
export(Agricultural Products, nwx/ag_prod/saf, saf)
export(Agricultural Products, nwx/ag_prod/wcr, wda)

# Air Quality Products
export(Air Quality Products, nwx/air_qual/apg, apg)
export(Air Quality Products, nwx/air_qual/aqi, aqi)
export(Air Quality Products, nwx/air_qual/asa, asa)

# ASOS summaries
export(ASOS summaries, nwx/asos/rr6, rr6)
export(ASOS summaries, nwx/asos/rr7, rr7)

# Avalanche
export(Avalanche, nwx/avalanche/ava, ava)
export(Avalanche, nwx/avalanche/avm, avm)
export(Avalanche, nwx/avalanche/avw, avw)
export(Avalanche, nwx/avalanche/sab, sab)
export(Avalanche, nwx/avalanche/sag, sag)
export(Avalanche, nwx/avalanche/swe, swe)
export(Avalanche, nwx/avalanche/wsw, wsw)

# Aviation
export(Aviation, nwx/aviation/aww, aww)
export(Aviation, nwx/aviation/oav, oav)
export(Aviation, nwx/aviation/rfr, rfr)
export(Aviation, nwx/aviation/sad, sad)
export(Aviation, nwx/aviation/sam, sam)
export(Aviation, nwx/aviation/sig, sig)
export(Aviation, nwx/aviation/wst, wst)
export(Aviation, nwx/aviation/wsv, wsv)
export(Aviation, nwx/aviation/area, area)
export(Aviation, nwx/aviation/airmet, airm)
export4(Aviation, nwx/aviation/sigmet, sgmt)

# Civil Advisory Products
export(Civil Advisory Products, nwx/civil_advs/awg, awg)
export(Civil Advisory Products, nwx/civil_advs/cae, cae)
export(Civil Advisory Products, nwx/civil_advs/cdw, cdw)
export(Civil Advisory Products, nwx/civil_advs/cem, cem)
export(Civil Advisory Products, nwx/civil_advs/evi, evi)
export(Civil Advisory Products, nwx/civil_advs/hmw, hmw)
export(Civil Advisory Products, nwx/civil_advs/lae, lae)
export(Civil Advisory Products, nwx/civil_advs/lew, lew)
export(Civil Advisory Products, nwx/civil_advs/nuw, nuw)
export(Civil Advisory Products, nwx/civil_advs/rhw, rhw)
export(Civil Advisory Products, nwx/civil_advs/sds, sds)
export(Civil Advisory Products, nwx/civil_advs/spw, spw)
export(Civil Advisory Products, nwx/civil_advs/sto, sto)
export(Civil Advisory Products, nwx/civil_advs/toe, toe)

# Climate
export(Climate, nwx/climate/cf6, cf6)
export(Climate, nwx/climate/cli, cli)
export(Climate, nwx/climate/clm, clm)
export(Climate, nwx/climate/cmm, cmm)

# Fire
export(Fire, nwx/fire/fdi, fdi)
export(Fire, nwx/fire/frw, frw)
export(Fire, nwx/fire/fwa, fwa)
export(Fire, nwx/fire/fwe, fwe)
export(Fire, nwx/fire/fwf, fwf)
export(Fire, nwx/fire/fwm, fwm)
export(Fire, nwx/fire/fwn, fwn)
export(Fire, nwx/fire/fwo, fwo)
export(Fire, nwx/fire/fws, fws)
export(Fire, nwx/fire/fww, fww)
export(Fire, nwx/fire/pbf, pbf)
export(Fire, nwx/fire/rfd, rfd)
export(Fire, nwx/fire/rfw, rfw)
export(Fire, nwx/fire/smf, smf)

# Flood/Flash Flood Products
export(Flood/Flash Flood Products, nwx/flood/esf, esf)
export(Flood/Flash Flood Products, nwx/flood/ffa, ffa)
export(Flood/Flash Flood Products, nwx/flood/ffg, ffg)
export(Flood/Flash Flood Products, nwx/flood/ffh, ffh)
export(Flood/Flash Flood Products, nwx/flood/ffs, ffs)
export(Flood/Flash Flood Products, nwx/flood/fln, fln)
export(Flood/Flash Flood Products, nwx/flood/fls, fls)
export(Flood/Flash Flood Products, nwx/flood/ffw, ffw)
export(Flood/Flash Flood Products, nwx/flood/flw, flw)

# Coded analysis/forecast
export(Coded analysis/forecast, nwx/fronts/cod, cod)

# Hydrometeorological Messages/Discussions/Products
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/hcm, hcm)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/hmd, hmd)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/hyd, hyd)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/hym, hym)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/rr1, rr1)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/rr2, rr2)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/rr3, rr3)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/rr4, rr4)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/rr5, rr5)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/rr6, rr6)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/rr7, rr7)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/rr8, rr8)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/rr9, rr9)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/rra, rra)
export(Hydrometeorological Messages/Discussions/Products, nwx/hydro/rrm, rrm)

# Coastal/Great Lakes/Offshore/Ice Marine Products
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/cfw, cfw)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/cwf, cwf)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/glf, glf)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/gls, gls)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/hsf, hsf)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/ice, ice)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/iob, iob)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/lsh, lsh)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/maw, maw)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/mim, mim)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/mrp, mrp)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/mvf, mvf)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/mws, mws)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/nsh, nsh)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/ocd, ocd)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/off, off)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/omr, omr)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/osw, osw)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/pls, pls)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/smw, smw)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/srf, srf)
export(Coastal/Great Lakes/Offshore/Ice Marine Products, nwx/marine/tid, tid)

#
# Miscellaneous Products
#
export(Extended Streamflow Guidance, nwx/misc/esg, esg)
export(Extended Streamflow Prediction, nwx/misc/esp, esp)
export(Water Supply Outlook, nwx/misc/ess, ess)
export(General Status Message, nwx/misc/gsm, gsm)
export(Spot Forecast Request, nwx/misc/stq, stq)
export(Transcribed Weather Broadcast, nwx/misc/twb, twb)

# PMD - Prognostic_Meteorological_Discussion (Basic Weather)
export(Prognostic Meteorological Discussion, nwx/misc/pmdca, pmdca)
export(Prognostic Meteorological Discussion, nwx/misc/pmdhi, pmdhi)
export(Prognostic Meteorological Discussion, nwx/misc/pmdsa, pmdsa)
export(Prognostic Meteorological Discussion, nwx/misc/pmdepd, pmdepd)
export(Prognostic Meteorological Discussion, nwx/misc/pmdep3, pmdep3)
export(Prognostic Meteorological Discussion, nwx/misc/pmdep4, pmdep4)
export(Prognostic Meteorological Discussion, nwx/misc/pmdep5, pmdep5)
export(Prognostic Meteorological Discussion, nwx/misc/pmdep6, pmdep6)
export(Prognostic Meteorological Discussion, nwx/misc/pmdep7, pmdep7)
export(Prognostic Meteorological Discussion, nwx/misc/pmdspd, pmdspd)
export(Prognostic Meteorological Discussion, nwx/misc/pmdthr, pmdthr)
export(Prognostic Meteorological Discussion, nwx/misc/pmdhmd, pmdhmd)
export(Prognostic Meteorological Discussion, nwx/misc/preepd, preepd)

# EFP - 3_to_5_Day_Extended_Forecast
export(3 to 5 Day Extended Forecast, nwx/extend/efp, efp)

# Extended Forecast Discussion
export4(Extended Forecast Discussion, nwx/extend, extend)

# Hemispheric Map Discussion AND 500mb Map Type Correlations
export(Hemispheric Map Discussion AND 500mb Map Type Correlations,
nwx/hemi, hemi)

#
# Model Products
#
export(Model Products, nwx/model/fan, fan)
export(Model Products, nwx/model/fmr, fmr)
export(Model Products, nwx/model/foh, foh)
export(Model Products, nwx/model/frh, frh)
export(Model Products, nwx/model/ftj, ftj)
export(Model Products, nwx/model/ftp, ftp)
export(Model Products, nwx/model/fwc, fwc)
export(Model Products, nwx/model/fzl, fzl)
export(Model Products, nwx/model/mav, mav)
export(Model Products, nwx/model/met, met)
export(Model Products, nwx/model/mex, mex)
export(Model Products, nwx/model/pfm, pfm)
export(Model Products, nwx/model/rdf, rdf)
export(Model Products, nwx/model/rdg, rdg)

#
# Non-Weather Events Productsdnl non_weather_events
#
# EQR - Earthquake_report
# EQW - Earthquake_warning
export(Earthquake Report, nwx/non_wx_events/eqr, eqr)
export(Earthquake Warning, nwx/non_wx_events/eqw, eqw)

# Observation Data Products
export(Observation Data Products, nwx/obs_data/lco, lco)
export(Observation Data Products, nwx/obs_data/scd, scd)
export(Observation Data Products, nwx/obs_data/taf, taf)

# Outlook Products
export(Outlook Products, nwx/outlook/eol, eol)
export(Outlook Products, nwx/outlook/eon, eon)
export(Outlook Products, nwx/outlook/hwo, hwo)

# Precipitation Products
export(Precipitation Products, nwx/precip/map, map)

# Public products
export(Public Products, nwx/pub_prod/afd, afd)
export(Public Products, nwx/pub_prod/afm, afm)
export(Public Products, nwx/pub_prod/afp, afp)
export(Public Products, nwx/pub_prod/aws, aws)
export(Public Products, nwx/pub_prod/awu, awu)
export(Public Products, nwx/pub_prod/ccf, ccf)
export(Public Products, nwx/pub_prod/lfp, lfp)
export(Public Products, nwx/pub_prod/lsr, lsr)
export(Public Products, nwx/pub_prod/mis, mis)
export(Public Products, nwx/pub_prod/now, now)
export(Public Products, nwx/pub_prod/opu, opu)
export(Public Products, nwx/pub_prod/pns, pns)
export(Public Products, nwx/pub_prod/rec, rec)
export(Public Products, nwx/pub_prod/rer, rer)
export(Public Products, nwx/pub_prod/rtp, rtp)
export(Public Products, nwx/pub_prod/rws, rws)
export(Public Products, nwx/pub_prod/rzf, rzf)
export(Public Products, nwx/pub_prod/scc, scc)
export(Public Products, nwx/pub_prod/scs, scs)
export(Public Products, nwx/pub_prod/sfp, sfp)
export(Public Products, nwx/pub_prod/sft, sft)
export(Public Products, nwx/pub_prod/sls, sls)
export(Public Products, nwx/pub_prod/sps, sps)
export(Public Products, nwx/pub_prod/stp, stp)
export(Public Products, nwx/pub_prod/swr, swr)
export(Public Products, nwx/pub_prod/tav, tav)
export(Public Products, nwx/pub_prod/tpt, tpt)
export(Public Products, nwx/pub_prod/tvl, tvl)
export(Public Products, nwx/pub_prod/wcn, wcn)
export(Public Products, nwx/pub_prod/wvm, wvm)
export(Public Products, nwx/pub_prod/zfp, zfp)

#
# Quantitative Precipitation Forecast Products
#
# QPF - QPF - the PTR, RSA and STR products
export(Quantitative Precipitation Forecast Products, nwx/qpf/qpf, qpf)

# QPFERD - QPF - Excessive Rainfall
# QPFHSD - QPF - Heavy Snowfall
# QPFPFD - QPF - Quantitative precipitation forecast discussion
export(Excessive Rainfall, nwx/qpf/qpferd, qpferd)
export(Heavy Snowfall, nwx/qpf/qpfhsd, qpfhsd)
export(Quantitative Precipitation Forecast Discussion, nwx/qpf/qpfpfd, qpfpfd)

# QPS - Quantitavive_Precipitation_Statement
export(Quantitavive_Precipitation_Statement, nwx/qpf/qps, qps)

# Radar Products
# FTM - WSR88_Radar_Outage_Notification/Free_Text_Message
# MRM - Missing_Radar_Message_(WSR_88D)
export(WSR88 Radar Outage Notification, nwx/radar/ftm, ftm)
export(Missing Radar Message, nwx/radar/mrm, mrm)

# River Products
export(River Products, nwx/river/rva, rva)
export(River Products, nwx/river/rvd, rvd)
export(River Products, nwx/river/rvf, rvf)
export(River Products, nwx/river/rvi, rvi)
export(River Products, nwx/river/rvm, rvm)
export(River Products, nwx/river/rvr, rvr)
export(River Products, nwx/river/rvs, rvs)

# Satellite Products
export(Satellite Products, nwx/satellite/scp, scp)
export(Satellite Products, nwx/satellite/scv, scv)
export(Satellite Products, nwx/satellite/sim, sim)

# Space Products
export(Space Products, nwx/space/mon, mon)

# Statistical Products
export(Statistical Products, nwx/stats/mef, mef)
export(Statistical Products, nwx/stats/par, par)
export(Statistical Products, nwx/stats/pvm, pvm)
export(Statistical Products, nwx/stats/sta, sta)
export(Statistical Products, nwx/stats/ver, ver)

# Tropical Products
export(Tropical Products, nwx/tropical/chg, chg)
export(Tropical Products, nwx/tropical/dsa, dsa)
export(Tropical Products, nwx/tropical/hls, hls)
export(Tropical Products, nwx/tropical/psh, psh)
export(Tropical Products, nwx/tropical/tca, tca)
export(Tropical Products, nwx/tropical/tcd, tcd)
export(Tropical Products, nwx/tropical/tce, tce)
export(Tropical Products, nwx/tropical/tcm, tcm)
export(Tropical Products, nwx/tropical/tcp, tcp)
export(Tropical Products, nwx/tropical/tcs, tcs)
export(Tropical Products, nwx/tropical/tcu, tcu)
export(Tropical Products, nwx/tropical/tma, tma)
export(Tropical Products, nwx/tropical/tsm, tsm)
export(Tropical Products, nwx/tropical/tsu, tsu)
export(Tropical Products, nwx/tropical/twd, twd)
export(Tropical Products, nwx/tropical/two, two)
export(Tropical Products, nwx/tropical/tws, tws)

# Ultraviolet Index Products
export(Ultraviolet Index Products, uvi, nwx/uvi/uvi, uvi)

# Volcano Products
export(Volcano Products, nwx/volcano/vva, vva)
export(Volcano Products, nwx/volcano/vow, vow)

# Watch and Warning Products
export(Watch and Warning Products, nwx/watch_warn/npw, npw)
export(Watch and Warning Products, nwx/watch_warn/svr, svr)
export(Watch and Warning Products, nwx/watch_warn/svs, svs)
export(Watch and Warning Products, nwx/watch_warn/tor, tor)
export(Watch and Warning Products, nwx/watch_warn/wsw, wsw)
export(Watch and Warning Products, nwx/watch_warn/ffw, ffw)
export(Watch and Warning Products, nwx/watch_warn/flw, flw)
