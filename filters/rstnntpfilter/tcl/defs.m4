dnl
dnl $Id$
dnl

dnl
dnl All the macros here are of the type "matchmore" (i.e., no break statement)
dnl because some products are duplicated to various groups.
dnl

define(match_txt_only,
lappend cond {
  $rc(txtflag) && [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  set rc_subgroup $3;
  set rc_subject $4;

  set rc_newsgroups "$rstnntpfilter(groupprefix).$rc_subgroup";
  filter_sendnntp_plain $rc_newsgroups $rc_subject $rc(fpath);
})

define(match_txt_except,
lappend cond {
  $rc(txtflag) && ![regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  set rc_subgroup $3;
  set rc_subject $4;

  set rc_newsgroups "$rstnntpfilter(groupprefix).$rc_subgroup";
  filter_sendnntp_plain $rc_newsgroups $rc_subject $rc(fpath);
})

define(match_txt_only_and,
lappend cond {
  $rc(txtflag) && [regexp {$2} $1] && [regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  set rc_subgroup $5;
  set rc_subject $6;

  set rc_newsgroups "$rstnntpfilter(groupprefix).$rc_subgroup";
  filter_sendnntp_plain $rc_newsgroups $rc_subject $rc(fpath);
})

define(match_txt_only_or,
lappend cond {
  $rc(txtflag) && ([regexp {$2} $1] || [regexp {$4} $3])
}
lappend action {
  set rc_status 0;
  set rc_subgroup $5;
  set rc_subject $6;

  set rc_newsgroups "$rstnntpfilter(groupprefix).$rc_subgroup";
  filter_sendnntp_plain $rc_newsgroups $rc_subject $rc(fpath);
})

define(match_txt_only_except,
lappend cond {
  $rc(txtflag) && [regexp {$2} $1] && ![regexp {$4} $3]
}
lappend action {
  set rc_status 0;
  set rc_subgroup $5;
  set rc_subject $6;

  set rc_newsgroups "$rstnntpfilter(groupprefix).$rc_subgroup";
  filter_sendnntp_plain $rc_newsgroups $rc_subject $rc(fpath);
})

define(match_binary,
lappend cond {
  [regexp {$2} $1]
}
lappend action {
  set rc_status 0;
  set rc_subgroup $3;
  set rc_subject $4;

  set rc_newsgroups "$rstnntpfilter(groupprefix).$rc_subgroup";
  filter_sendnntp_encode $rc_newsgroups $rc_subject $rc(fpath);
})

dnl
dnl Arg $4 can be a (uwildregex) variable. If not, must be enclosed in {}.
dnl
define(match_and_binary,
lappend cond {
  [regexp {$2} $1] && [filterlib_uwildmat $4 $3]
}
lappend action {
  set rc_status 0;
  set rc_subgroup $5;
  set rc_subject $6;

  set rc_newsgroups "$rstnntpfilter(groupprefix).$rc_subgroup";
  filter_sendnntp_encode $rc_newsgroups $rc_subject $rc(fpath);
})

define(stopmatch,
lappend cond {
  ($rc_status == 0)
}
lappend action {
  break;
})
