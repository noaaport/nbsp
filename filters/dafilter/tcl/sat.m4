dnl
dnl $Id$
dnl
dnl
dnl tig = goes
dnl tic = composite (goes + meteosat)
dnl tid, tit = microwave
dnl
match_sat($rc(wmoid), ^ti,
sat/gini/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt)])

match_sat_archive($rc(wmoid), ^ti,
sat/gini/[subst $dafilter(archive_sat_dirfmt)],
[subst $dafilter(archive_sat_namefmt)])

match_stop($rc(wmoid), ^ti)
