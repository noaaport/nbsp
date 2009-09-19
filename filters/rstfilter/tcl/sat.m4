match_sat($rc(wmoid), $rstfilter(sat_regex),
sat/img/$rc(wmoid), sat/gini/$rc(wmoid))

match_satloop($rc(wmoid), $rstfilter(satloop_regex),
sat/img/$rc(wmoid), *.gif, sat/loop, $rc(wmoid).gif)

stopmatch
