#!%TCLSH%
#
# $Id$
#
# Copyright (c) 2008 Jose F. Nieves <nieves@ltp.uprrp.edu>
#
# See LICENSE
#
# Usage: fm35dc [-v] [-a] [-c] [-d] [-n <na_str>] [-s <parts_sep>] \
#               [-l <levels_sep>] <file>};
#
# The file can be a .upa file or a raw file, with or without the ccb.
# If the <parts_sep> is not given, only the data portion is written, not
# the entire record. If <file> is not given, then it reads from stdin.
#
# Examples
#
# dcfm35 -d -s "," -l "|" -n NA < 2009022115.upa
# dcfm35 -s "," -l "|" -n NA < 2009022115.upa
#
# Without options, the outputs are lines like
#
# ttaa 71125 84629 99001 22023 00000 00060 21417
#
# and with the -s option,
#
# e.g., 
#
# dcfm35 -s "," < 2009022115.upa
#
# the output is
#
# spim,211200,84629,2112,ttaa 71125 84629 99001 22023
#
# and with dcfm35 -s " " < 2009022115.upa
#
# the output is
#
# spim 211200 84629 2112 ttaa 71125 84629 99001 22023
#
# where the elements are
#
# <wmostation> <wmotime> <obstation> <obtime> <obdata>
#
# with <obtime> = hhmm, and <obstation> = station numeric id.
#
# With the -d option the data portion of the output is the decoded data.
# instead of the <obdata>. The element corresponding
# to each level is a comma-separated list of five or three numbers, depending
# on the level, in the same order emited by the fm35 library,
# with each of those multiplets preceeded by the level name: surface,
# tropopause, windmax, 1000, ...:
#
# surface,<p_mb>,<temp_c>,<dewp_c>,<wspeed_kt>,<wdir>
# tropopause,<p_mb>,<temp_c>,<dewp_c>,<wspeed_kt>,<wdir>
# windmax,<p_mb>,<wspeed_kt>,<wdir>                         
# 1000,<height_m>,<temp_c>,<dewp_c>,<wspeed_kt>,<wdir>
# ...
#
# The data is output all in one line.
#
# Example:
#
# dcfm35.tcl -d -s "," <2009022115.upa
# dcfm35.tcl -d -s "," -l "|" < 2009022115.upa
#
# give
#
# spim,211200,84629,2112 surface,1001,22.0,19.7,0,0 1000,60,21.4,19.7,2,345 ...
# spim,211200,84629,2112|surface,1001,22.0,19.7,0,0|1000,60,21.4,19.7,2,345|...
#
# respectively. If -a is added in this mode, then decoded data will
# include the stnm,time prior to the levels data.
#
# With the "-c" (which implies "-d") the input file is assumed to have been
# cleaned instead of being the raw "upa" file. A cleanedup file has all
# the data record in one line:
#
# spim 211200 84629 2112 ttaa 71125 84629 99001 22023 ...
# spim,211200,84629,2112,ttaa 71125 84629 99001 22023 ...
# ttaa 71125 84629 99001 22023 ...

package require cmdline;
package require textutil::split;
lappend auto_path "%TCLUPPERAIR_INSTALLDIR%";
package require upperair::fm35;

set usage {nbspfm35dc [-v] [-a] [-c] [-d] [-l <levels_sep>]
    [-n <na_str>] [-s <data_sep>] <file>};

set optlist {v a c d {l.arg ""} {n.arg ""} {s.arg ""}};

# The wmo header is searched anywhere in the line, not necessarily
# at the start, in order to be usable also with the files saved with the ccb.
# For the report start marks (ttxx) we have both, at the start (for raw files)
# and anywhere (for cleanup files).
#
set g(wmoheader_regexp) {u[[:alnum:]]{5} [[:alnum:]]{4} \d{6}};
set g(report_start_regexp) {^tt(aa|bb|cc|dd) \d{5} \d{5}};
set g(report_end_regexp) {=$};
set g(report_continuation_regexp) {[\d/]{5}};	# digits or /
set g(report_start_regexp_any) {tt(aa|bb|cc|dd) \d{5} \d{5}};

set g(data_sep) ",";		# -s
set g(levels_sep) " ";		# -l
set g(na_str) "";		# -n

set record(wmoid) "";
set record(wmostation) "";
set record(wmotime) "";
#
set record(obpart) "";		# ttaa, ...
set record(obtime) "";		# from the record (1st after the ttaa)
set record(obstation) "";	# from the record (2nd after the ttaa)
set record(obdata) "";		# raw data record
set record(_open) 0;

# Variables
set g(F) stdin;
set g(fpath) "";

proc match_report_start {str} {

    global g;

    if {[regexp $g(report_start_regexp) $str]} {
	return 1;
    }

    return 0;
}

proc match_report_end {str} {

    global g;

    if {[regexp $g(report_end_regexp) $str]} {
	return 1;
    }

    return 0;
}

proc match_report_continuation {str} {

    global g;

    if {[regexp $g(report_continuation_regexp) $str]} {
	return 1;
    }

    return 0;
}

proc obdata_init {data} {

    global record;

    set record(obdata) $data;
    set record(_open) 1;

    # Split and get the report time and station id from the
    # first and second elements (after the ttaa).
    set elements [::textutil::split::splitx $data];
    set ddhhi [lindex $elements 1];
    set record(obstation) [lindex $elements 2];
    
    if {[regexp {(\d{2})(\d{2})(\d)} $ddhhi match dd hh i]} {
	if {$dd > 50} {
	    set dd [expr $dd - 50];
	}
	set record(obtime) ${dd}${hh};
    } else {
	set record(obtime) $ddhhi;
    }
}

proc obdata_append {data} {

    global record;

    append record(obdata) " " $data;

    if {[match_report_end $data]} {
	obdata_end;
    }
}

proc obdata_end {} {

    global g;
    global record;
    global option;

    set record(_open) 0;

    set record(obdata) [string trimright $record(obdata) "="];

    if {$option(d) == 0} {
	# Write out the raw data.
	if {$option(s) eq ""} {
	    puts $record(obdata);
	} else {
	    set r [join [list $record(wmostation) $record(wmotime) \
			     $record(obstation) $record(obtime) \
			     $record(obdata)] $g(data_sep)];
	    puts $r;
	}
	return;
    }

    set record(decoded_data) [decode_report $record(obdata)];

    if {[regexp {W|E} $record(decoded_data)] != 0} {
	if {$option(v) != 0} {
	    puts $record(decoded_data);
	}
	return;
    }

    set header [join [list $record(wmostation) $record(wmotime) \
		 $record(obstation) $record(obtime)] $g(data_sep)];

    set r [join [list $header $record(decoded_data)] $g(levels_sep)];

    puts $r;
}

proc obdata_start {data} {

    global record;

    # Close the current record if it is open
    if {$record(_open) == 1} {
	obdata_end;
    }

    obdata_init $data;

    if {[regexp {=$} $data]} {
	obdata_end;
    }
}

proc decode_report {report} {

    global g;
    global option;

    # Only ttaa or ttcc (this is not seen if the parser rejects the other parts
    # as soon as they are read).
    if {[regexp {tt(aa|cc)} $report] == 0} {
	return "W: No ttaa or ttcc.";
    }

    set status [catch {
	::upperair::fm35::decode $report;
    } errmsg];
    if {$status != 0} {
	return "E: $errmsg";
    }

    set r [list];

    if {$option(a) == 1} {
	set h "";
	append h [::upperair::fm35::get_siteid] $g(data_sep) \
	    [::upperair::fm35::get_time];
	lappend r $h;
    }

    foreach level [::upperair::fm35::get_levels] {
	set l "";
	append l $level $g(data_sep);
	append l [join [::upperair::fm35::get_data $level] $g(data_sep)];
	lappend r $l;
    }

    return [join $r $g(levels_sep)];
}

proc process_raw_file {} {

    global g;
    global record;

    while {[gets $g(F) line] >= 0 } {

	set line [string tolower [string trim $line]];

	# Ignore empty lines that we get due to the ending \r\r\n sequence.
	if {$line eq ""} {
	    continue;
	}

	# Looking for the wmo header this way catches it even in the case
	# in which it is embedded in a header with a ccb (so it does 
	# not start a line).

	if {[regexp $g(wmoheader_regexp) $line match]} {
	    # split and get the wmo_time and wmo_station
	    set wmo [::textutil::split::splitx $match];
	    set record(wmoid) [lindex $wmo 0];
	    set record(wmostation) [lindex $wmo 1];
	    set record(wmotime) [lindex $wmo 2];

	    continue;
	}

	# Each line is handled according to how it starts, and whether
	# a record has not been closed (because it did not end with = so
	# we could not determine if it ended, until we read the next line).
	if {[match_report_start $line]} {
	    # Start a new the record.
	    # If obdata_start sees the '=' sign it closes it.
	    obdata_start $line;
	} elseif {[match_report_continuation $line] && ($record(_open) == 1)} {
	    # A continuation line is anything that is digits and / and
	    # which we get when a record is still open.
	    obdata_append $line;
	} elseif {$record(_open) == 1} {
	    obdata_end;
	}
    }

    if {$record(_open) == 1} {
	obdata_end;
    }
}

proc process_clean_file {} {
#
# Lines are assumed to be like
#
# <header><some separator>ttxx....
#
# where the <header>could be missing. The <header> is then output intact,
# followed by the <level_sep>, followed by the decoded data. If "-a" was
# given, the decoded data includes the stnm and time.

    global g;
    global option;

    while {[gets $g(F) line] >= 0 } {

	set line [string tolower [string trim $line]];

	# Ignore possible empty lines that we get due to the
	# ending \r\r\n sequence.
	if {$line eq ""} {
	    continue;
	}

	if {[regexp $g(report_start_regexp_any) $line match s1]} {
	    set part "tt";
	    append part $s1;	# s1 contains the aa, bb, ...
	    set start [string first $part $line];
	    set obdata [string range $line $start end];
	    if {$start == 0} {
		set header "";
	    } else {
		# The header ends before the ttxx
		incr start -1;
		set header [string range $line 0 $start];
	    }
	    if {[string length $header] > 1} {
		# trim the separator between the header ane the ttxx
		set header [string range $header 0 end-1];
	    }
	} else {
	    continue;
	}

	set decoded_data [decode_report $obdata];

	if {[regexp {W|E} $decoded_data] == 1} {
	    if {$option(v) == 1} {
		puts $decoded_data;
		return;
	    }
	    continue;
	}

	
	set r "";
	if {$header ne ""} {
	    append r $header $g(levels_sep);
	}
	
	append r $decoded_data;
	puts $r;
    }
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$option(c) == 1} {
    set option(d) 1;
}

if {$option(l) ne ""} {
    set g(levels_sep) $option(l);
}

if {$option(n) ne ""} {
    ::upperair::fm35::set_na $option(n);
}

if {$option(s) ne ""} {
    set g(data_sep) $option(s);
}

if {$argc != 0} {
    set g(fpath) [lindex $argv 0];
    set status [catch {set g(F) [open $g(fpath) "r"]} errmsg];
    if {$status != 0} {
	puts $errmsg;
	exit 1;
    }
}

fconfigure $g(F) -encoding binary -translation binary;
fconfigure stdout -encoding binary -translation binary;

if {$option(c) == 0} {
    process_raw_file;
} else {
    process_clean_file;
}

if {$g(fpath) ne ""} {
    close $g(F);
}
