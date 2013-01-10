#!%TCLSH%
#
# $Id$
#
# Copyright (c) 2008 Jose F. Nieves <nieves@ltp.uprrp.edu>
#
# See LICENSE
#
# Usage: uatocsv [-h] [-i] [-l <levels_sep>] [-n <na_str>] [-s <datasep>]
#                [<file>]
#
# The input data should be in the format such as
#
# spim,211200,84629,2112 surface,1001,22.0,19.7,0,0 1000,60,21.4,19.7,2,345 ...
# spim,211200,84629,2112|surface,1001,22.0,19.7,0,0|1000,60,21.4,19.7,2,345|...
#
# as returned by the fm35dc decoder, where the <levels_sep> (" " or "|")
# separates the different levels. So, an example usage is
#
#      fm35dc -c 78526_20110714.upperair | uatocsv
#
# The data portion of such records (which starts after the first <separator>)
#
# surface,<p_mb>,<temp_c>,<dewp_c>,<wspeed_kt>,<wdir>
# tropopause,<p_mb>,<temp_c>,<dewp_c>,<wspeed_kt>,<wdir>
# windmax,<p_mb>,<wspeed_kt>,<wdir>                         
# 1000,<height_m>,<temp_c>,<dewp_c>,<wspeed_kt>,<wdir>
#
# is then converted to a set of records of the form
#
# <level_name>,<height>,<p_mb>,<temp_c>,<dewp_c>,<wspeed_kt>,<wdir>
#
# that is,
#
# surface,0,<p_mb>,<temp_c>,<dewp_c>,<wspeed_kt>,<wdir>        (surface)
# <level>,<height_m>,1000,<temp_c>,<dewp_c>,<wspeed_kt>,<wdir> (1000, 950, ...)
# tropopause,NA,<p_mb>,<temp_c>,<dewp_c>,<wspeed_kt>,<wdir>    (tropopause)
# windmax,NA,<p_mb>,NA,NA,<wspeed_kt>,<wdir>                   (windmax)
#
# and each such converted record is output preceeded by the two elements
#
# <obstation>,<obtime>
#
# So, for each input record, the complete output is a set of records
# of the form
#
# <obstation>,<obtime>,\
#	<level_name>,<height>,<p_mb>,<temp_c>,<dewp_c>,<wspeed_kt>,<wdir>
#
# If the [-h] option is given, the first line of the output is a header
# with the column names.
#
# If the [-i] option is used, then the non-option argument is taken
# to be a data record instead of a file name.

package require cmdline;
package require fileutil;

set usage {uatocsv [-h] [-i] [-l <levels_sep>] [-n <na_str>] [-s <input_sep>]
    [<file>]};
set optlist {h i {l.arg ""} {n.arg ""} {s.arg ""}};

set g(output_sep) ",";	# not configurable
set g(levels_sep) "";	# -l
set g(na_str) "";	# -n
set g(data_sep) ",";    # -s

# Variables
set g(F) stdin;
set g(fpath) "";
set g(input_str) "";
set g(header) 0;     # add columns header

#
# Functions
#
proc write_header {} {

    global g;

    puts -nonewline "# ";
    puts [join [list station time \
		    level height_m p_mb temp_c dewp_c wspeed_kt wdir] \
	      $g(output_sep)];
}

proc process_file {} {

    global g;

    if {$g(header) == 1} {
	write_header;
    }

    while {[gets $g(F) data] >= 0} {
	if {[regexp {^\s+$} $data]} {
	    continue;
	}
	set output_records [process_line $data];
	if {[llength $output_records] != 0} {
	    puts [join $output_records "\n"];
	}
    }
}

proc process_input {} {

    global g;

    set output_records [process_line $g(input_str)];
    if {[llength $output_records] == 0} {
	return;
    }

    if {$g(header) == 1} {
	write_header;
    }

    puts [join $output_records "\n"];
}

proc process_line {line} {

    global g;

    if {$g(levels_sep) ne ""} {
	set data [split $line $g(levels_sep)];
    } else {
	set data [split $line];
    }

    # info = [<wmostation>,<wmotime>,]<obstation>,<obtime>
    set info [split [lindex $data 0] $g(data_sep)];
    set obstation [lindex $info end-1];
    set obtime [lindex $info end];
	
    set level_records [lrange $data 1 end];
    set output_records [list];

    foreach record $level_records {
	set record_values [split $record $g(data_sep)];
	set level_name [lindex $record_values 0];
	
	if {$level_name eq "surface"} {
	    # surface,<p_mb>,<temp_c>,<dewp_c>,<wspd_kt>,<wdir>
	    # surface,0,<p_mb>,<temp_c>,<dewp_c>,<wspd_kt>,<wdir>
	    set output [linsert $record_values 1 "0"];
	} elseif {$level_name eq "tropopause"} {
	    # tropopause,<p_mb>,<temp_c>,<dewp_c>,<wspd_kt>,<wdir>
	    # tropopause,NA,<p_mb>,<temp_c>,<dewp_c>,<wspd_kt>,<wdir>
	    set output [linsert $record_values 1 $g(na_str)];
	} elseif {$level_name eq "windmax"} {
	    # windmax,<p_mb>,<wspeed_kt>,<wdir>
	    # windmax,NA,<p_mb>,NA,NA,<wspd_kt>,<wdir>
	    set output [linsert $record_values 2 $g(na_str) $g(na_str)];
	    set output [linsert $output 1 $g(na_str)];
	} else {
	    # <level>,<height_m>,<temp_c>,<dewp_c>,<wspd_kt>,<wdir>
	    # <level>,<height_m>,<level>,<temp_c>,<dewp_c>,<wspd_kt>,<wdir>
	    set output [linsert $record_values 2 $level_name];
	}
	set output [linsert $output 0 $obstation $obtime];
	lappend output_records [join $output $g(output_sep)];
    }

    return $output_records;
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

set g(header) $option(h);

if {$option(l) ne ""} {
    set g(levels_sep) $option(l);
}

if {$option(n) ne ""} {
    set g(na_str) $option(n);
}

if {$option(s) ne ""} {
    set g(data_sep) $option(s);
}

if {$argc != 0} {
    if {$option(i) == 0} {
	set g(fpath) [lindex $argv 0];
	set status [catch {set g(F) [open $g(fpath) "r"]} errmsg];
	fconfigure $g(F) -encoding binary -translation binary;
	if {$status != 0} {
	    puts $errmsg;
	    exit 1;
	}
    } else {
	set g(input_str) [lindex $argv 0];
    }
}

fconfigure stdout -encoding binary -translation binary;

if {$option(i) == 0} {
    set status [catch {process_file} errmsg];
    if {$g(fpath) ne ""} {
	close $g(F);
    }

    if {$status != 0} {
        puts $errmsg;
        exit 1;	
    }
} else {
    process_input;
}
