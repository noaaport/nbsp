#
# $Id$
#
# Variables:
#
# %{ymdh} %{m} %{g} %{fh}
#

define(m4grbdir,grib/data/grb)dnl

define(export,
set ``export'(dirs,grib/$2_$3) \
  "$1,m4grbdir/$2/%{ymdh},$2_$3_%{ymdh}_%{fh}.grb"')dnl

define(export2,
set ``export'(dirs,grib/$2_$3) \
  "$1,m4grbdir/$2/%{ymdh},$2_$3_%{ymdh}_%{fh}.grb2"')dnl
