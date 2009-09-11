dnl
dnl $Id$
dnl

#
# Duplicate of nhc hurricane-related products.
#
match_file($rc(awips1), tcm, hurricane/track, $yyyy.$rc(awips))
match_file_noappend($rc(awips1), tcm|tcp|tcd|pws,
hurricane/forecast, $rc(awips).txt)

match_file($rc(awips1), chg, hurricane/model, $yyyy.$rc(awips))
match_file_noappend($rc(awips1), chg, hurricane/model, $rc(awips).txt)
