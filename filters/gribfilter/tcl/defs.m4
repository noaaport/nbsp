dnl
dnl $Id$
dnl

define(match_file,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) \
    "$3" \
    "$4";
  $5
})

define(match_and_file,
lappend cond {
  [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) \
    "$5" \
    "$6";
  $7
})

define(match_or_file,
lappend cond {
  [regexp {$2} $1] || [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) \
    "$5" \
    "$6";
  $7
})

define(matchstop_file,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) \
    "$3" \
    "$4";	
  $5
  break;
})

define(matchstop_and_file,
lappend cond {
  [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) \
   "$5" \
   "$6";
  $7
  break;
})

define(matchstop_or_file,
lappend cond {
  [regexp {$2} $1] || [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) \
    "$5" \
    "$6";
  $7
  break;
})

dnl
dnl Note that there are no brackets around $2 here (so that we can pass tcl
dnl variables in the argument $2.
dnl
define(match_file_varregexp,
lappend cond {
  [regexp $2 $1]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) \
    "$3" \
    "$4";
  $5
})

define(match_stop,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  break;
})
