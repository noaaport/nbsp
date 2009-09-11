dnl
dnl $Id$
dnl

define(match_pipe,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_pipe $rc(fpath) "$3";
  break;
})

define(matchmore_pipe,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_pipe $rc(fpath) "$3";
})

define(match_exec,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_exec $rc(fpath) "$3";
  break;
})

define(matchmore_exec,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_exec $rc(fpath) "$3";
})

define(match_mail,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_sendmail "$3" "$4" "$5" $rc(fpath);
  break;
})

define(matchmore_mail,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_sendmail "$3" "$4" "$5" $rc(fpath);
})

define(match,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  $3;
})
