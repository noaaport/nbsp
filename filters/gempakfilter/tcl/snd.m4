dnl
dnl $Id$
dnl

# These will save the nam and gfs bufr sounding files in the same
# way as the THREDDS servers, but that does not seem to be the same format
# as the NCEP servers that modsnd5 uses.

# ncep nam model bufr sounding
matchstop_file($rc(fname) s s1, ^kwno_jus.(4[1-9]),
soundings/nam/$ymdh,
${ymdh}_nam${s1}.bufr)

# ncep gfs model bufr sounding
matchstop_file($rc(fname) s s1, ^kwbc_jus.(4[1-9]),
soundings/gfs/$ymdh,
${ymdh}_gfs${s1}.bufr)
