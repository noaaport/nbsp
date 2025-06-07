dnl
dnl $Id$
dnl

define(match_text_all,
if {$rstfilter(txt_enable) == 1} {
  lappend cond {
    $rc(txtflag) && [filterlib_uwildmat $rstfilter(txt_regex) $1]
  }
  lappend action {
    set rc_status 0;
    set rc_output [filter_text $rc(fpath) $1 $2];
  }
})

define(match_text_only,
if {$rstfilter(txt_enable) == 1} {
  lappend cond {
    $rc(txtflag) && [regexp {$2} $1] && \
      [filterlib_uwildmat $rstfilter(txt_regex) $3]
  }
  lappend action {
    set rc_status 0;
    set rc_output [filter_text $rc(fpath) $3 $4];
  }
})

define(match_text_except,
if {$rstfilter(txt_enable) == 1} {
  lappend cond {
    $rc(txtflag) && ![regexp {$2} $1] && \
      [filterlib_uwildmat $rstfilter(txt_regex) $3]
  }
  lappend action {
    set rc_status 0;
    set rc_output [filter_text $rc(fpath) $3 $4];
  }
})

define(match_text_only_and,
if {$rstfilter(txt_enable) == 1} {
  lappend cond {
    $rc(txtflag) && [regexp {$2} $1] && [regexp {$4} $3] && \
      [filterlib_uwildmat $rstfilter(txt_regex) $5]
  }
  lappend action {
    set rc_status 0;
    set rc_output [filter_text $rc(fpath) $5 $6];
  }
})

define(match_text_only_except,
if {$rstfilter(txt_enable) == 1} {
  lappend cond {
    $rc(txtflag) && [regexp {$2} $1] && ![regexp {$4} $3] && \
      [filterlib_uwildmat $rstfilter(txt_regex) $5]
  }
  lappend action {
    set rc_status 0;
    set rc_output [filter_text $rc(fpath) $5 $6];
  }
})

define(match_rad,
if {$rstfilter(rad_enable) == 1} {
  lappend cond {
    [regexp {^sdus[23578]} $rc(wmoid)] && [filterlib_uwildmat $2 $1]
  }
  lappend action {
    set rc_status 0;
    set rc_output [filter_rad $rc(awips) $rc(fpath) $3 $4];
  }
})

define(match_radloop,
if {$rstfilter(radloop_enable) == 1} {
  lappend cond {
    [regexp {^sdus[23578]} $rc(wmoid)] && [filterlib_uwildmat $2 $1]
  }
  lappend action {
    filter_radloop $3 $4 $5 $6;
  }
})

define(match_sat,
if {$rstfilter(sat_enable) == 1} {
  lappend cond {
    [regexp {^ti} $rc(wmoid)] && [filterlib_uwildmat $2 $1]
  }
  lappend action {
    set rc_status 0;
    set rc_output [filter_sat $rc(wmoid) $rc(fpath) $3 $4 $5];
  }
})

define(match_satloop,
if {$rstfilter(satloop_enable) == 1} {
  lappend cond {
    [regexp {^ti} $rc(wmoid)] && [filterlib_uwildmat $2 $1]
  }
  lappend action {
    filter_satloop $3 $4 $5 $6;
  }
})

define(match_sat_goesr,
if {$rstfilter(sat_goesr_enable) == 1} {
  lappend cond {
    [regexp {^ti} $rc(wmoid)] && [filterlib_uwildmat $2 $1]
  }
  lappend action {
    set rc_status 0;
    set rc_output [filter_sat_goesr $rc(fpath) $3 $4 $5];
  }
})

define(stopmatch,
lappend cond {
  ($rc_status == 0)
}
lappend action {
  break;
})
