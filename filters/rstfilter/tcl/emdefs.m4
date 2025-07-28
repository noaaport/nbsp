dnl
dnl $Id$
dnl

define(match_emwin_text_only,
lappend cond {
  $rc(txtflag) && [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  break;
})

define(match_emwin_text_except,
lappend cond {
  $rc(txtflag) && ![regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  break;
})

define(match_emwin_text_only_except,
lappend cond {
  $rc(txtflag) && [regexp {$2} $1] && ![regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  break;
})

define(match_emwin_rad,
lappend cond {
  ($emwinfilter(rad_enable) == 1) && \
	[regexp {^sdus[235678]} $rc(wmoid)] && [filterlib_uwildmat $2 $1]
}
lappend action {
  set rc_status 0;
  break;
})

define(match_emwin_sat,
lappend cond {
 ($emwinfilter(sat_enable) == 1) && \
	[regexp {^ti} $rc(wmoid)] && [filterlib_uwildmat $2 $1]
}
lappend action {
  set rc_status 0;
  break;
})
