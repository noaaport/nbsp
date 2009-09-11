dnl
dnl $Id$
dnl
dnl These are rules that are not in gempak but are of general interest

# aloft (where are these in gempak?)
match_file_noappend($rc(wmoid), ^fb(us|ak|hw|oc)3(1|3|5|7|8|9),
aloft, $rc(wmotime).$rc(wmoid),
  if {$dafilter(aloft_csv_enable) == 1} {
    cd $dafilter(datadir);
    exec nbspaloftcsv -d "aloft" [file join "aloft" $rc(wmotime).$rc(wmoid)];
  }
)

# Profiler winds in bufr format. (From ``pqact.thredds'')
match_file($rc(wmoid), ^(iupt0[1-4]|iuak01), profiler, ${ymd}_prof.bufr)
match_file($rc(fname) s s1, ^kbou_iupt4([1-3]), profiler,${ymd}_prof${s1}.bufr)
match_file($rc(fname), ^kbou_isat, profiler, ${ymd}_prof4.bufr)
match_file($rc(wmoid), ^iupc0[12], profiler, ${ymd}_prof5.bufr)

# These will save the nam and gfs bufr sounding files in the same
# way as the THREDDS servers, but that does not seem to be the same format
# as the NCEP servers that modsnd5 uses.

# ncep nam and gfs model bufr soundings.
match_file($rc(fname) s s1, ^kwno_jus.(4[1-9]),
soundings/nam/$ymdh, ${ymdh}_nam${s1}.bufr)

match_file($rc(fname) s s1, ^kwbc_jus.(4[1-9]),
soundings/gfs/$ymdh, ${ymdh}_gfs${s1}.bufr)

# El Nino Southern Oscillation (ENSO) Alerts
# KWNC FXUS24 - AWIPS = PMDENS
match_file($rc(awips), ^pmdens, enso, ${ymd_hm}.enso)
