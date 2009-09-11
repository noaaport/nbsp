dnl
dnl $Id$
dnl
dnl
dnl This is the old version before the check was introduced in the
dnl process() function itself.
dnl
dnl match2_raw($rc(wmoid) s s1, ^tig(.), $rc(wmoid), $nntpfilter(sat_regex),
dnl sat.raw.tig${s1}, 0)
dnl
match_raw($rc(wmoid) s s1, ^tig(.),
sat.raw.tig${s1}, 0)
