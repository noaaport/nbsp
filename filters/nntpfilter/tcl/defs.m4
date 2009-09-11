dnl
dnl $Id$
dnl

define(match_raw,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  set rc_subgroup $3;
  set rc_stripccb $4;
  break;
})

dnl
dnl Arg 4 can be a (uwildregex) variable; otherwise enclose it in {}.
dnl
define(match2_raw,
lappend cond {
  [regexp {$2} $1] && [filterlib_uwildmat $4 $3]
}
lappend action {
  set rc_status 0;
  set rc_subgroup $5;
  set rc_stripccb $6;
  break;
})

define(match_text_all,
lappend cond {
  $rc(txtflag)
}
lappend action {
  set rc_status 0;
  set rc_subgroup $1;
  set rc_stripccb $2;
  break;
})

define(match_text_only,
lappend cond {
  $rc(txtflag) && [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  set rc_subgroup $3;
  set rc_stripccb $4;
  break;
})

define(match_text_except,
lappend cond {
  $rc(txtflag) && ![regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  set rc_subgroup $3;
  set rc_stripccb $4;
  break;
})
