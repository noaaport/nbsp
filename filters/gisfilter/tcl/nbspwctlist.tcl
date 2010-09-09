#!%TCLSH%
#
# $Id$
#
# Usage: nbspwctlist [-b] [-k] [-K] [-l <latestname>] [-V] [-w <wctbin>]
#                    <listfile> <fmt>
#
# -k => delete the listfile at the end
# -K => delete the cache dir
#
# This is a cmdline tool to process a "listfile". A "listfile" is
# essentially a "wct listfile" with comments that contain additional
# post-processing instructions that this scripts uses (e.g., for renaming
# the output files).
#
package require cmdline;

set usage {nbspwctlist [-b] [-k] [-K] [-l <latestname>] [-V]
    [-w <wctbin>] <listfile> <fmt>};
set optlist {b k K {l.arg ""} V {w.arg ""}};

# defaults
set nbspwctlist(wct_bin) "wct-export";

# parameters
set nbspwctlist(wct_cachedir) [file join ~/ ".wct-cache"];

proc log_warn s {

    global argv0;
    global option;

    set name [file tail $argv0];
    if {$option(b) == 0} {
        puts "$name: $s";
    } else {
        exec logger -t $name $s;
    }
}

proc log_err s {

    log_warn $s;
    exit 1;
}

proc log_wct_err s {

    global option;
    global errorInfo;

    if {$option(V) == 1} {
	log_warn $s;
	log_err $errorInfo;
    } else {
	log_err $s;
    }
}    

proc process_wct_listfile {wct_listfile fmt} {

    global nbspwctlist option;
    global errorInfo;

    set status [catch {
	exec $nbspwctlist(wct_bin) $wct_listfile $fmt;
    } errmsg];

    if {[regexp {BATCH PROCESSING ERROR} $errmsg]} {
	log_wct_err $errmsg;
    }
}

proc clean_wct_cache_dir {} {

    global nbspwctlist option;

    if {($option(K) == 1) && [file isdirectory $nbspwctlist(wct_cachedir)]} {
	file delete -force $nbspwctlist(wct_cachedir);
    }
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$argc != 2} {
    log_err $usage;
}
set wct_listfile [lindex $argv 0];
set fmt [lindex $argv 1];

if {$option(w) ne ""} {
    set nbspwctlist(wct_bin) $option(w);
}

# Process the list file
clean_wct_cache_dir;
process_wct_listfile $wct_listfile $fmt;
clean_wct_cache_dir;

# Rename
set flist [exec cat $wct_listfile];

foreach entry $flist {

    if {[regexp {^\#} $entry] == 0} {
	continue;
    }
	 
    set entry_parts [split $entry ","];
    set type [lindex $entry_parts 1];
    set inputfile [lindex $entry_parts 2];
    set outputfile [lindex $entry_parts 3];

    set cmd [list nbspwct -f $fmt -n -o $outputfile -t $type];
    if {$option(l) ne ""} {
	lappend cmd "-l" $option(l);
    }

    if {$option(V) == 1} {
	lappend cmd "-V";
    }

    if {$option(w) ne ""} {
	lappend cmd "-w" $option(w);
    }

    lappend cmd $inputfile;

    set status [catch {
	eval exec $cmd;
    } errmsg];

    if {$status != 0} {
	log_warn $errmsg;
	continue;
    }

    if {$option(k) == 1} {
	file delete $wct_listfile;
    }
}
