dnl
dnl $Id$
dnl
dnl
dnl front, forecast, day1, day2

# SPC Products
# Severe Weather Outlook (Day 1-3) ACUS0[1-3]
match_file_noappend($rc(awips) s s1, swody([0-9]), nwx/spc/day$s1, latest)
matchstop_file($rc(awips) s s1, swody([0-9]), nwx/spc/day$s1, $ymdh.day$s1)

# Coded Fronts Analysis
match_file_noappend($rc(wmoid), asus01, nwx/hpc/fronts, latest)
matchstop_file($rc(wmoid), asus01, nwx/hpc/fronts, $ymdh.front)

# Coded Fronts Forecast
match_file_noappend($rc(wmoid), fsus02, nwx/hpc/forecast, latest)
matchstop_file($rc(wmoid), fsus02, nwx/hpc/forecast, $ymdh.fcst)
