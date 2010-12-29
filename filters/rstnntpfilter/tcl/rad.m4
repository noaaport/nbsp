dnl
dnl $Id$
dnl
dnl This was the old version before we introduced the check in
dnl the process() function itself.
dnl
dnl match_and_binary($rc(wmoid), ^sdus[23578],
dnl $rc(awips), $rstnntpfilter(rad_regex),
dnl rad.img.$rc(awips2), $rc(awips))
dnl
match_binary($rc(wmoid), ^sdus[23578],
rad.img.$rc(awips2), $rc(awips))

stopmatch
