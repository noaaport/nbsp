dnl
dnl $Id$
dnl 

dnl
dnl This was in the old pqact.conf, but not now. They are now in ship (sb).
dnl
 
match_file($rc(wmoid), sivc15|sivd1[5678]|sivd4[56],
nwx/marine/buoy, ${ymdh}.buoy)

match_and_file($rc(station), kwbc, $rc(wmoid), sivd2[0123],
nwx/marine/buoy, ${ymdh}.buoy)
