#
# $Id$
#
##
## Decoders data
##

# dcmetr
export(Surface, surface, sao)

# Synoptic land reports - dclsfc
export(Synoptic land reports, syn, syn)

# ship, buoy and CMAN - dcmsfc
export(Ship Buoy and CMAN, ship, sb)

# 6 Hour Ship Observations
export(6 Hour Ship Observations, ship6hr, ship)

# Wave Observations (from the ncep pqact.conf)
export(Wave Observations, wavobs, wvob)

# upper air reports and dropsonde reports - dcuair
export(Upper Air Reports, upperair, upa)
export(Dropsonde Reports, drops, drop)

# ngm Mos - dcnmos
export4(NGM Mos, mos, nmos)

# Decoder for GFS MOS - dcgmos
export4(GFS MOS, mos, gmos)

# Decoder for GFSX MOS
export4(GFSX MOS, mos, xmos)

# Decoder for global sea-ice drift bulletins - dcidft
export4(Sea-Ice drift, idft, idft)

# Alaska sea ice bulletins - dcidft
export4(Alaska Sea Ice, idft, idak)

# SPC storm reports - dcstorm [NWUS20 (daily), NWUS22 (Hourly)]
export(SPC Storm Reports, storm/sels, sels)

# Watch box coordinates - dcwatch
export(Watch box coordinates, storm/watches, watches)

# dcwarn
export(Warnings, storm/warn, warn)

# dcwtch
export(Watches, storm/wtch, wtch)

# dcwcp
export(WCP, storm/wcp, wcp)

# Watchbox outlines /pSLSxx (WWUS32 and WWUS6[1-5])
export(Watchbox Outlines, storm/svrl, svrl)

# Hurricane/tropical storm positions and forecasts - dctrop
export3(Hurricane/tropical storms, storm/tropic/epacific, %{wmoid})
export3(Hurricane/tropical storms, storm/tropic/wpacific, %{wmoid})
export3(Hurricane/tropical storms, storm/tropic/cpacific, %{wmoid})
export3(Hurricane/tropical storms, storm/tropic/atlantic, %{wmoid})

# dchrcn XXXX
export(HCRN, storm/hrcn, hrcn)

# Winter Warnings, Watches and Advisories - dcwstm
export(Winter Warnings/Watches and Advisories, storm/wstm, wstm)

# flash flood watches - dcffa
export(Flash Flood Watches, storm/ffa, ffa)

# flash flood guidance - dcffg
export(Flash Flood Guidance, storm/ffg, ffg)

# Supplemental Climatological Data (SCD) - dcscd
export(Supplemental Climatological Data, scd, scd)

# Aircraft Observations - dcacft
export(Aircraft Observations, acft, acf)

# Airmets - dcairm
export(Airmets, airm, airm)

# International Sigmets - dcisig
export(International Sigmets, isig, isig)

# Non-convective Sigmets - dcncon
export(Non-convective Sigmets, ncon, sgmt)

# Convective Sigmets - dccsig
export(Convective Sigmets, csig, conv)

# TAFs - dctaf
export(TAFs, taf, taf)

# Regional Digital Forecasts/Point Forecast Matrices - dcrdf
export(Regional Digital Forecasts/Point Forecast Matrices, rdf, rdf)

# watch outline updates - dcwou
export2(Watch Outline Updates, storm/wou, wou)
export2(Watch Outline Updates, storm/wcn, wcn)

##
## End of decoders data
##
