dnl
dnl $Id$
dnl
dnl matchstop_file_noappend($rc(wmoid), ^sdus[2-8],
dnl nexrad/NIDS/$rc(AWIPS2)/$rc(AWIPS1), $rc(AWIPS1)_$ymd_hm)
dnl
match_rad($rc(wmoid), ^sdus[2-8],
nexrad/NIDS/$rc(AWIPS2)/$rc(AWIPS1), $rc(AWIPS1)_$ymd_hm)
