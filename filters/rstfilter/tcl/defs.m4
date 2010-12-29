dnl
dnl $Id$
dnl

define(match_text_all,
lappend cond {
  $rc(txtflag) && ($rstfilter(txt_enable) == 1) && \
    [filterlib_uwildmat $rstfilter(txt_regex) $1]
}
lappend action {
  set rc_status 0;
  set rc_output [filter_text $rc(fpath) $1];
})

define(match_text_only,
lappend cond {
  $rc(txtflag) && ($rstfilter(txt_enable) == 1) && [regexp {$2} $1] && \
    [filterlib_uwildmat $rstfilter(txt_regex) $3]
}
lappend action {
  set rc_status 0;
  set rc_output [filter_text $rc(fpath) $3];
})

define(match_text_except,
lappend cond {
  $rc(txtflag) && ($rstfilter(txt_enable) == 1) && ![regexp {$2} $1] && \
    [filterlib_uwildmat $rstfilter(txt_regex) $3]
}
lappend action {
  set rc_status 0;
  set rc_output [filter_text $rc(fpath) $3];
})

define(match_text_only_and,
lappend cond {
  $rc(txtflag) && ($rstfilter(txt_enable) == 1) && \
    [regexp {$2} $1] && [regexp {$4} $3] && \
    [filterlib_uwildmat $rstfilter(txt_regex) $5]
}
lappend action {
  set rc_status 0;
  set rc_output [filter_text $rc(fpath) $5];
})

define(match_text_only_except,
lappend cond {
  $rc(txtflag) && ($rstfilter(txt_enable) == 1) && \
    [regexp {$2} $1] && ![regexp {$4} $3] && \
    [filterlib_uwildmat $rstfilter(txt_regex) $5]
}
lappend action {
  set rc_status 0;
  set rc_output [filter_text $rc(fpath) $5];
})

define(match_rad,
lappend cond {
  ($rstfilter(rad_enable) == 1) && \
	[regexp {^sdus[23578]} $rc(wmoid)] && [filterlib_uwildmat $2 $1]
}
lappend action {
  set rc_status 0;
  set rc_output [filter_rad $rc(awips) $rc(fpath) $3 $4];
})

define(match_radloop,
lappend cond {
  ($rstfilter(radloop_enable) == 1) && \
	[regexp {^sdus[23578]} $rc(wmoid)] && [filterlib_uwildmat $2 $1]
}
lappend action {
  filter_radloop $3 $4 $5 $6;
})

define(match_sat,
lappend cond {
  ($rstfilter(sat_enable) == 1) && \
	[regexp {^ti} $rc(wmoid)] && [filterlib_uwildmat $2 $1]
}
lappend action {
  set rc_status 0;
  set rc_output [filter_sat $rc(wmoid) $rc(fpath) $3 $4 $5];
})

define(match_satloop,
lappend cond {
  ($rstfilter(satloop_enable) == 1) && \
	[regexp {^ti} $rc(wmoid)] && [filterlib_uwildmat $2 $1]
}
lappend action {
  filter_satloop $3 $4 $5 $6;
})

define(stopmatch,
lappend cond {
  ($rc_status == 0)
}
lappend action {
  break;
})
