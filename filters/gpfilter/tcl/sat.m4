dnl
dnl $Id$
dnl
dnl non-gini -
dnl tip = polarsat (5th channel - 2012)
dnl tir, tis = goes-r west and east sbn pids 107 and 108 (2016)
dnl
dnl https://vlab.noaa.gov/web/towr-s/goes-scmi: (2024)
dnl   WMO Header 	TI[RSU][A-VYZ][0|1][0-9] KNES
dnl The goesr-noaaport-user-guide-v04.2023.pdf [p.34]
dnl documents only ti[rs], but pqact.conf from
dnl   "https://github.com/Unidata/awips2/blob/unidata_20.3.2/rpms/awips2.upc/\
dnl    Installer.ldm/patch/etc/pqact.goesr"
dnl also has tiu.
dnl glm - https://vlab.noaa.gov/web/towr-s/glm
dnl That document (see "towr-s.pdf") states that the wmo header is
dnl East: tirs00 knes ddhhmm pxx
dnl West: tirt00 knes ddhhmm pzz
dnl with various BBB as follows:
dnl PAA - PCJ for tiles
dnl PZZ       for GLMFDSTTAT
dnl
dnl ixt = derived products (2024)
dnl https://vlab.noaa.gov/web/towr-s/goes-r-l2s
dnl
dnl old config
dnl match_sat_gini($rc(wmoid), ^ti[cdgt])

dnl what used to be the gini files
match_sat_goesr($rc(wmoid), ^ti[cdgt],
images/sat/goesr/[string range $rc(WMOID) 0 2]/$rc(WMOID), $rc(WMOID)_${ymd_hm})

dnl polar
match_sat_polar($rc(wmoid), ^tip,
images/sat/viirs/[string range $rc(WMOID) 0 2]/$rc(WMOID), $rc(WMOID)_${ymd_hm})

goesr - glm
match_sat_goesr($rc(wmoid), ^tir[st]00,
images/sat/glm/[string range $rc(WMOID) 0 2]/$rc(WMOID), $rc(WMOID)_${ymd_hm})

goesr - the rest
match_sat_goesr($rc(wmoid), ^ti[rsu],
images/sat/goesr/[string range $rc(WMOID) 0 2]/$rc(WMOID), $rc(WMOID)_${ymd_hm})

match_sat_goesr($rc(wmoid), ^ixt,
images/sat/goesr/[string range $rc(WMOID) 0 2]/$rc(WMOID), $rc(WMOID)_${ymd_hm})
