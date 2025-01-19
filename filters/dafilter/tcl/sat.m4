dnl
dnl $Id$
dnl
dnl gini (old) -
dnl tig = goes
dnl tic = composite (goes + meteosat)
dnl tid, tit = microwave
dnl
dnl non-gini -
dnl tip = polarsat (5th channel - 2012)
dnl tir, tis = goes-r west and east sbn pids 107 and 108 (2016)
dnl
dnl https://vlab.noaa.gov/web/towr-s/goes-scmi: (2024)
dnl   WMO Header 	TI[RSU][A-VYZ][0|1][0-9] KNES
dnl The goesr-noaaport-user-guide-v04.2023.pdf [p.34]
dnl documents only ti[rs], but pqact.conf from
dnl   "https://github.com/Unidata/awips2/blob/unidata_20.3.2/rpms/awips2.upc/\
dnl    Installer.ldm/patch/etc/pqact.goesr"
dnl also has tiu.
dnl glm - https://vlab.noaa.gov/web/towr-s/glm
dnl That document (see "towr-s.pdf") states that the wmo header is
dnl East: tirs00 knes ddhhmm pxx
dnl West: tirt00 knes ddhhmm pzz
dnl with various BBB as follows:
dnl PAA - PCJ for tiles
dnl PZZ       for GLMFDSTTAT
dnl
dnl ixt = derived products (2024)
dnl https://vlab.noaa.gov/web/towr-s/goes-r-l2s
dnl
dnl old config
dnl match_sat_gini($rc(wmoid), ^ti[cdgt],
dnl sat/gini/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt)])
dnl
dnl match_sat_archive($rc(wmoid), ^ti[cdgt],
dnl sat/gini/[subst $dafilter(archive_sat_dirfmt)],
dnl [subst $dafilter(archive_sat_namefmt)])

dnl
dnl what used to be the gini files
dnl
match_sat_ngini($rc(wmoid), ^ti[cdgt],
sat/goesr/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt_goesr)])

match_sat_archive($rc(wmoid), ^ti[cdgt],
sat/goesr/[subst $dafilter(archive_sat_dirfmt)],
[subst $dafilter(archive_sat_namefmt_goesr)])

dnl
dnl polar
dnl
match_sat_ngini($rc(wmoid), ^tip,
sat/viirs/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt_tip)])

match_sat_archive($rc(wmoid), ^tip,
sat/viirs/[subst $dafilter(archive_sat_dirfmt)],
[subst $dafilter(archive_sat_namefmt_tip)])

dnl
dnl goesr - glm
dnl
match_sat_ngini($rc(wmoid), ^tir[st]00,
sat/glm/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt_glm)])

match_sat_archive($rc(wmoid), ^tir[st]00,
sat/glm/[subst $dafilter(archive_sat_dirfmt)],
[subst $dafilter(archive_sat_namefmt_glm)],
break;)

dnl
dnl goesr - the rest
dnl
match_sat_ngini($rc(wmoid), ^ti[rsu],
sat/goesr/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt_goesr)])

match_sat_archive($rc(wmoid), ^ti[rsu],
sat/goesr/[subst $dafilter(archive_sat_dirfmt)],
[subst $dafilter(archive_sat_namefmt_goesr)])

match_sat_ngini($rc(wmoid), ^ixt,
sat/goesr/[subst $dafilter(sat_dirfmt)], [subst $dafilter(sat_namefmt_goesr)])

match_sat_archive($rc(wmoid), ^ixt,
sat/goesr/[subst $dafilter(archive_sat_dirfmt)],
[subst $dafilter(archive_sat_namefmt_goesr)])

match_stop($rc(wmoid), ^ti)
match_stop($rc(wmoid), ^ixt)
