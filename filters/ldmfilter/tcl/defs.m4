dnl
dnl $Id$
dnl

define(match_ldmfeed,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  set rc_ldmfeed "$3";
  append rc_ldmprodid $4;
  break;
})

define(match_ldmfeed_not,
lappend cond {
  ([regexp {$2} $1] == 0)
}
lappend action {
  set rc_status 0;
  set rc_ldmfeed "$3";
  append rc_ldmprodid $4;
  break;
})

define(match_ldmfeed_and,
lappend cond {
  [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  set rc_ldmfeed "$5";
  append rc_ldmprodid $6;
  break;
})

define(match_ldmfeed_andnot,
lappend cond {
  [regexp {$2} $1] && ([regexp {$4} $3] == 0)
}
lappend action {
  set rc_status 0;
  set rc_ldmfeed "$5";
  append rc_ldmprodid $6;
  break;
})
