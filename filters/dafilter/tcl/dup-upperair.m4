dnl
dnl $Id$
dnl

#
# Subsets of upperair that DA can import
#

# Pireps (rc(nawips) = pirep)
match_file($rc(wmoid), ^u[ab], pirep, ${ymdh}.airep)

match_file($rc(wmoid), ^ud, acars, ${ymdh}.amdar)
match_and_file($rc(wmoid), ^u[efklms], $rc(nawips), ^tt(aa|bb|cc|dd),
upperair, ${ymdh}.fm35)

dnl match_file($rc(wmoid), ^ut, acars, ${ymdh}.codar)
dnl match_file($rc(wmoid), ^u[efklms], upperair, ${ymdh}.temp)
dnl match_file($rc(wmoid), ^u[ghipqy], upperair, ${ymdh}.wind)
