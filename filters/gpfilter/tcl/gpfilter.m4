#
# $Id$
#
dnl 
dnl Definitions and macros (required)
dnl
divert(-1)
include(`defs.m4')
include(`macros.m4')
divert(0)

dnl
dnl Filters
dnl
dnl Put pipes first, and for those products that are processed by
dnl the decoders and also by file.m4, in pipe.m4 we use the
dnl match_pipe() macro (instead of matchstop_pipe).
dnl
include(`rad.m4')
include(`sat.m4')
include(`grib.m4')
include(`pipe.m4')
include(`file.m4')
include(`snd.m4')
