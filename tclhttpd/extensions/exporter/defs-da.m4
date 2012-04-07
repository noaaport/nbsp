#
# $Id$
#
#
# Variables:
#
# %{ymd_hm} %{ymdh} %{awips} %{awips1} %{awips2} %{wmoid}
#
dnl define(export, `$2,$1,%{ymdh}.$3')
dnl define(export2, `$2,$1,%{ymd_hm}.$3')
dnl define(export3, `$2,$1,%{ym}.$3')

define(export, set ``export'(dirs,da/$2) "$1,digatmos/$2,%{ymdh}.$3"')dnl
define(export2, set ``export'(dirs,da/$2) "$1,digatmos/$2,%{ymd_hm}.$3"')dnl
define(export3, set ``export'(dirs,da/$2) "$1,digatmos/$2,%{ym}.$3"')dnl
define(export4, set ``export'(dirs,da/$2.$3) "$1,digatmos/$2,%{ymdh}.$3"')dnl
define(exportrad,
	set ``export'(dirs,da/$2) "$1,digatmos/$2,%{awips}_%{ymd_hm}.$3"')dnl
