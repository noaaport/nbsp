dnl
dnl $Id$
dnl
dnl
dnl tig = goes
dnl tic = composite (goes + meteosat)
dnl tid, tit = microwave
dnl tip = polarsat
dnl
match_sat($rc(wmoid), ^ti[^p],
sat/gini/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt)])

match_sat_archive($rc(wmoid), ^ti[^p],
sat/gini/[subst $dafilter(archive_sat_dirfmt)],
[subst $dafilter(archive_sat_namefmt)])

match_psat($rc(wmoid), ^tip,
sat/viirs/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt_tip)])

match_sat_archive($rc(wmoid), ^tip,
sat/viirs/[subst $dafilter(archive_sat_dirfmt)],
[subst $dafilter(archive_sat_namefmt_tip)])

match_stop($rc(wmoid), ^ti)
