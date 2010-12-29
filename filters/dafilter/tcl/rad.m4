dnl
dnl $Id$
dnl

dnl
dnl level 3
dnl
dnl 4 and 6 are status msgs
dnl
match_rad($rc(wmoid), ^sdus[2-8],
nexrad/nids/[subst $dafilter(rad_dirfmt)], [subst $dafilter(rad_namefmt)])

match_rad_archive($rc(wmoid), ^sdus[23578],
nexrad/nids/[subst $dafilter(archive_rad_dirfmt)],
[subst $dafilter(archive_rad_namefmt)])

match_stop($rc(wmoid), ^sdus[2-8])

dnl
dnl level 2 - do not add the gempak header and footer
dnl
match_rad2($rc(wmoid), ^level2,
nexrad/craft/[subst $dafilter(rad2_dirfmt)], [subst $dafilter(rad2_namefmt)])

match_rad_archive($rc(wmoid), ^level2,
nexrad/craft/[subst $dafilter(archive_rad2_dirfmt)],
[subst $dafilter(archive_rad2_namefmt)])

match_stop($rc(wmoid), ^level2)
