dnl
dnl $Id$
dnl
dnl gini
dnl tig = goes
dnl tic = composite (goes + meteosat)
dnl tid, tit = microwave
dnl non-gini
dnl tip = polarsat (5th channel - 2012)
dnl tir, tis = goes-r west and east sbn pids 107 and 108 (2016) (also tiu later)
dnl ixt = derived products (2024)
dnl
match_sat_gini($rc(wmoid), ^ti[cdgt])

match_sat_polar($rc(wmoid), ^tip,
images/sat/viirs/[string range $rc(WMOID) 0 2]/$rc(WMOID), $rc(WMOID)_${ymd_hm})

match_sat_goesr($rc(wmoid), ^ti[rsu],
images/sat/goesr/[string range $rc(WMOID) 0 2]/$rc(WMOID), $rc(WMOID)_${ymd_hm})

match_sat_goesr($rc(wmoid), ^ixt,
images/sat/goesr/[string range $rc(WMOID) 0 2]/$rc(WMOID), $rc(WMOID)_${ymd_hm})
