dnl
dnl $Id$
dnl
dnl gini -
dnl tig = goes
dnl tic = composite (goes + meteosat)
dnl tid, tit = microwave
dnl non-gini -
dnl tip = polarsat (5th channel) (2012)
dnl tir, tis = goes-r west and east sbn pids 107 and 108 (2016)
dnl
match_sat_gini($rc(wmoid), ^ti[cdgt],
sat/gini/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt)])

match_sat_archive($rc(wmoid), ^ti[cdgt],
sat/gini/[subst $dafilter(archive_sat_dirfmt)],
[subst $dafilter(archive_sat_namefmt)])

match_sat_ngini($rc(wmoid), ^tip,
sat/viirs/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt_tip)])

match_sat_archive($rc(wmoid), ^tip,
sat/viirs/[subst $dafilter(archive_sat_dirfmt)],
[subst $dafilter(archive_sat_namefmt_tip)])

match_sat_ngini($rc(wmoid), ^ti[rs],
sat/goesr/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt_goesr)])

match_sat_archive($rc(wmoid), ^ti[rs],
sat/goesr/[subst $dafilter(archive_sat_dirfmt)],
[subst $dafilter(archive_sat_namefmt_goesr)])

match_stop($rc(wmoid), ^ti)
