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
  $5
})

define(match_and_file,
lappend cond {
  [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$5" "$6";
  $7
})

define(match_or_file,
lappend cond {
  [regexp {$2} $1] || [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$5" "$6";
  $7
})


define(matchstop_file,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$3" "$4";	
  $5
  break;
})

define(matchstop_and_file,
lappend cond {
  [regexp {$2} $1] && [regexp {$4} $3]
} 
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$5" "$6";
  $7
  break;
})

define(matchstop_or_file,
lappend cond {
  [regexp {$2} $1] || [regexp {$4} $3]
} 
lappend action {
  set rc_status 0;
  filter_file $rc(seq) $rc(fpath) "$5" "$6";
  $7
  break;
})

define(match_file_compress,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_file_compress $rc(seq) $rc(fpath) "$3" "$4";
  $5
})

define(matchstop_file_compress,
lappend cond {
  [regexp {$2} $1]
} 
lappend action {
  set rc_status 0;
  filter_file_compress $rc(seq) $rc(fpath) "$3" "$4";
  $5
  break;
})

define(match_file_noappend,
lappend cond {
  [regexp {$2} $1]
} 
lappend action {
  set rc_status 0;
  filter_file_noappend $rc(seq) $rc(fpath) "$3" "$4";
  $5
})

define(matchstop_file_noappend,
lappend cond {
  [regexp {$2} $1]
} 
lappend action {
  set rc_status 0;
  filter_file_noappend $rc(seq) $rc(fpath) "$3" "$4";
  $5
  break;
})

define(match_rad,
lappend cond {
  [regexp {$2} $1]
} 
lappend action {
 set rc_status 0;
  if {($dafilter(raduncompress) != 0) && \
    ([regexp {^nmd} $rc(awips)] == 0)} {
	filter_rad_unz $rc(seq) $rc(fpath) "$3" "$4";
  } else {
	filter_rad $rc(seq) $rc(fpath) "$3" "$4";
  }
  $5
  break;
})

define(match_rad_only,
lappend cond {
  [regexp {$2} $1] && [regexp {$4} $3]
} 
lapend action {
  set rc_status 0;
  if {($dafilter(raduncompress) != 0) && \
    ([regexp {^nmd} $rc(awips)] == 0)} {
      filter_rad_unz $rc(seq) $rc(fpath) "$5" "$6";
  } else {
      filter_rad $rc(seq) $rc(fpath) "$5" "$6";
  }
  $7
  break;
})

define(match_rad_except,
lappend {
  [regexp {$2} $1] && ([regexp {$4} $3] == 0)
}
lappend action {
  set rc_status 0;
  if {($dafilter(raduncompress) != 0) && ([regexp {^nmd} $rc(awips)] == 0)} {
    filter_rad_unz $rc(seq) $rc(fpath) "$5" "$6";
  } else {
    filter_rad $rc(seq) $rc(fpath) "$5" "$6";
  }
  $7
  break;
})

define(match_sat,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  filter_sat $rc(fpath) "$3" "$4";
  $5
  break;	
})

define(match_and_grib,
lappend cond  {
  ($rc(nawips) eq "grib") && \
  ($rc(gribstatus) == 0) && ($rc(gribedition) == 1) && \
  [regexp {$2} $1] && [regexp {$4} $3]
} 
lappend action {
  set rc_status 0;
  filter_grib $rc(seq) $rc(fpath) "$5" "$6";
  $7
})

define(matchstop_and_grib,
lappend cond {
  ($rc(nawips) eq "grib") && \
  ($rc(gribstatus) == 0) && ($rc(gribedition) == 1) && \
  [regexp {$2} $1] && [regexp {$4} $3]
} 
lappend action {
  set rc_status 0;
  filter_grib $rc(seq) $rc(fpath) "$5" "$6";
  $7
  break;
})

define(match_stop,
lappend cond {
  [regexp {$2} $1]
} 
lappend action {
  break;
})