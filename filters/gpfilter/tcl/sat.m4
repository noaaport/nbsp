dnl
dnl $Id$
dnl
dnl tig = goes
dnl tic = composite (goes + meteosat)
dnl tid, tit = microwave
dnl tip = polarsat
dnl
match_sat($rc(wmoid), ^ti[^p])

match_psat($rc(wmoid), ^tip)
