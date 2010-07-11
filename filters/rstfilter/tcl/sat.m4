match_sat($rc(wmoid), $rstfilter(sat_regex),
sat/img/[subst $rstfilter(sat_dirfmt)],
sat/gini/[subst $rstfilter(sat_dirfmt)],
[subst $rstfilter(sat_namefmt)])

match_satloop($rc(wmoid), $rstfilter(satloop_regex),
sat/img/[subst $rstfilter(sat_dirfmt)], *.gif,
sat/loop, $rc(wmoid).gif)

stopmatch
