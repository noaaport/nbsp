dnl
dnl $Id$
dnl
dnl
dnl This is the old version before the check was introduced in the
dnl process() function itself. In addition, later (July 2010) we added
dnl the other ti(c|d|t)xxx files.
dnl
dnl match2_raw($rc(wmoid) s s1, ^tig(.), $rc(wmoid), $nntpfilter(sat_regex),
dnl sat.raw.tig${s1}, 0)
dnl
match_raw($rc(wmoid) s s1, ^ti(.{2}),
sat.raw.ti${s1}, 0)
