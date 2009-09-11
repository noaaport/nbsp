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
dnl (e.g., grlevel warnings, hrricanes), then the "standard" ones.
dnl Then put pipes, and for those products that are processed by
dnl the decoders and also by file.m4, in pipe.m4 we use the
dnl match_file() macro instead of matchstop_file.
dnl
dnl The rc.tcl file contains some functions used by the grib rules.
dnl
include(`rad.m4')
include(`sat.m4')
include(`rc.tcl')
include(`grib.m4')
include(`dup.m4')
include(`extra.m4')
include(`pipe.m4')
include(`file.m4')

# This will store in the subdirectory "unprocessed" all the files that are
# not processed by any of the rules. Instead of saving the file under
# $rc(fname).$rc(seq) we use just the fname to keep just the newest instance
# of each such product.
if {[info exists dafilter(unprocessed_enable)] && \
	($dafilter(unprocessed_enable) == 1)} {
  lappend cond {
      ($rc_status == 1)
  }
  lappend action {
      filter_file_noappend $rc(seq) $rc(fpath) \
	  "unprocessed/$rc(station)" $rc(fname);
  }
}
