dnl
dnl ##R
dnl
match_sat($rc(wmoid), $rstfilter(sat_regex),
sat/img/[subst $rstfilter(sat_dirfmt)],
sat/gini/[subst $rstfilter(sat_dirfmt)],
[subst $rstfilter(sat_namefmt)])

dnl
dnl ##R
dnl
match_satloop($rc(wmoid), $rstfilter(satloop_regex),
sat/img/[subst $rstfilter(sat_dirfmt)], *.gif,
sat/loop, $rc(wmoid).gif)

match_sat_goesr($rc(wmoid)$rc(nawips), $rstfilter(sat_goesr_regex),
sat/img/[subst $rstfilter(sat_goesr_dirfmt)],
sat/nc/[subst $rstfilter(sat_goesr_dirfmt)],
[subst $rstfilter(sat_goesr_namefmt)])

stopmatch
