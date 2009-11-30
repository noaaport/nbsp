dnl
dnl $Id$
dnl

dnl If we want a directory listing (a la ``ondas'') for just a few stations
dnl put those first:
dnl
dnl match_rad_only($rc(wmoid), ^sdus[2357],
dnl $rc(awips2), fdr|tlx|dyx|lbb|ama|fws,
dnl nexrad/nids/[subst $dafilter(rad_dirfmt)], [subst $dafilter(rad_namefmt)])
dnl
dnl followed by the rest
dnl
dnl match_rad($rc(wmoid), ^sdus[2357],
dnl nexrad/nids/[subst $dafilter(rad_dirfmt)], [subst $dafilter(rad_namefmt)])
dnl
dnl The default is a directory listing for all (but it can be disabled
dnl by setting the variable dafilter(rad_dirlist_enable) to zero in
dnl the configuration file).

# If the radar data files are saved in uncompressed form, then the "nmd"
# files should be omitted from this list because they do not have compressed
# data.
match_rad($rc(wmoid), ^sdus[2357],
nexrad/nids/[subst $dafilter(rad_dirfmt)], [subst $dafilter(rad_namefmt)])

match_rad_archive($rc(wmoid), ^sdus[2357],
nexrad/nids/[subst $dafilter(archive_rad_dirfmt)],
[subst $dafilter(archive_rad_namefmt)])

match_stop($rc(wmoid), ^sdus[2357])
