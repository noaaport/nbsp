dnl
dnl $Id$
dnl
match_sat($rc(wmoid), ^tig,
sat/gini/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt)])

match_sat_archive($rc(wmoid), ^tig,
sat/gini/[subst $dafilter(archive_sat_dirfmt)],
[subst $dafilter(archive_sat_namefmt)])

match_stop($rc(wmoid), ^tig)
