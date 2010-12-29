dnl
dnl $Id$
dnl
dnl This is the old version before the check was introduced in the
dnl process() function itself.
dnl
dnl match2_raw($rc(wmoid), ^sdus[2357], $rc(awips), $nntpfilter(rad_regex),
dnl rad.raw.$rc(awips2), 0)
dnl
match_raw($rc(wmoid), ^sdus[2-8],
rad.raw.$rc(awips2), 0)
