dnl
dnl $Id$
dnl

define(match_file,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$3" "$4";
})

define(match_and_file,
lappend cond {
  [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$5" "$6";
})

define(matchstop_file,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$3" "$4";	
  break;
})

define(matchstop_and_file,
lappend cond {
  [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$5" "$6";
  break;
})

define(match_file_noappend,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_file_noappend $rc(seq) $rc(fpath) "$3" "$4";
})

define(matchstop_file_noappend,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_file_noappend $rc(seq) $rc(fpath) "$3" "$4";
  break;
})

define(match_pipe,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_pipe $rc(seq) $rc(fpath) "$3" "$4" "$5";
})

define(match_and_pipe,
lappend cond {
  [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  filter_pipe $rc(seq) $rc(fpath) "$5" "$6" "$7";
})

define(matchstop_pipe,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_pipe $rc(seq) $rc(fpath) "$3" "$4" "$5";
  break;
})

define(matchstop_and_pipe,
lappend cond {
  [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  filter_pipe $rc(seq) $rc(fpath) "$5" "$6" "$7";
  break;
})

dnl
dnl This is the rule using filter_rad to insert the data files in the data
dnl inventory.
dnl
define(match_rad,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_rad $rc(seq) $rc(fpath) "$3" "$4";
  break;
})

define(match_sat,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_sat $rc(seq) $rc(fpath) "$3" "$4";
  break;
})

define(match_stop,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  break;
})

define(stopmatch,
lappend cond {
  ($rc_status == 0)
}
lappend action {
  break;
})
