dnl
dnl $Id$
dnl

##
## Decoders data
##

# dcmetr
matchstop_file($rc(wmoid), ^s[ap], surface, $ymdh.sao)

# Synoptic land reports
matchstop_file($rc(wmoid), (^s[im]v[igns])|(^snv[ins])|(^s[imn](w[kz]|[^vw])),
syn, $ymdh.syn)

# 6 Hour Ship Observations
matchstop_file($rc(wmoid), ^s[imn]v[^gins]|^s[imn]w[^kz], ship6hr, $ymdh.ship)

# Upper air reports
match_file($rc(wmoid), ^u[ab], pirep, ${ymdh}.airep)
match_file($rc(wmoid), ^ud, acars, ${ymdh}.amdar)
match_and_file($rc(wmoid), ^u[efklms], $rc(nawips), ^tt(aa|bb|cc|dd),
upperair, ${ymdh}.fm35)

##
## End of decoders data
##
