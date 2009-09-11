#
# $Id$
#
dnl 
dnl Definitions and macros (required)
dnl
divert(-1)
include(`defs.m4')
divert(0)
dnl
dnl Filter rules
dnl
dnl In the text files, put first the duplicates for special applications
dnl (e.g., grlevel warnings), then the "standard" ones.
dnl Then put pipes, and for those products that are processed by
dnl the decoders and also by file.m4, in pipe.m4 we use the
dnl match_file() macro instead of matchstop_file.
dnl
include(`rad.m4')
dnl include(`sat.m4')
include(`grib.m4')
include(`dup-warnings.m4')
include(`dup-hurricane-lite.m4')
include(`pipe-lite.m4')
include(`file-lite.m4')
