match_rad($rc(awips), $rstfilter(rad_regex),
rad/img/[subst $rstfilter(rad_dirfmt)], [subst $rstfilter(rad_namefmt)])

match_radloop($rc(awips), $rstfilter(radloop_regex),
rad/img/[subst $rstfilter(rad_dirfmt)], *.gif,
rad/loop/$rc(awips2), $rc(awips).gif)

stopmatch
