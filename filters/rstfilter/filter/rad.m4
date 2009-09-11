match_rad($rc(awips), $rstfilter(rad_regex),
rad/img/$rc(awips2)/$rc(awips1), $rc(awips)_${ymd_hm}.gif)

match_radloop($rc(awips), $rstfilter(radloop_regex),
rad/img/$rc(awips2)/$rc(awips1), $rc(awips)*.gif,
rad/loop/$rc(awips2), $rc(awips).gif)

stopmatch
