#!%TCLSH%
#
# $Id$
#
# Usage: nbspstatmonitor [-f <fmt>]
#
# The <fmt> can be: std (default), yaml, xml, csv, csvk
#
# This script retrieves the "stats" counters using the script nbspstatcounters
# and makes some tests to determine the health of the server.
# The output is formatted similarly to the output from nbspstatcounter,
# but with the following variables:
#
# monitor_code - 0 or an error code
# poll_time    - time at which the script is called
# stats_time   - time at which the server logged the stats
# chstats_time - time at which the inventory filter logged the last minute stats
# stats_timediff - difference between poll and stats times
# chstats_timediff - difference between poll and chstats times
# total_files  - total number of files received by the inventory filter
# total_bytes  - total number of bytes received by the inventory filter
#
# The code returned in monitor_code is 0 (no error) or
#
# err_stats_time 1   - if the difference between poll and stats times > 60
# err_chstats_time 2 - if the difference between poll and chstats times > 60
# err_chstats_data 3 - if total_files or total_bytes are zero

package require cmdline;

set usage {nbspstatmonitor [-f <fmt>]};
set optlist {{f.arg "std"}};

proc monitor {rawoutput} {
    #
    # rawoutput is meant to be the output of ``nbspstatcounters -f csvk''
    #
    set keys [list monitor_code \
		  poll_time \
		  stats_time \
		  chstats_time \
		  stats_timediff \
		  chstats_timediff \
		  total_files \
		  total_bytes];

    # error codes
    set err_stats_time 1;
    set err_chstats_time 2;
    set err_chstats_data 3;

    set total_files 0;
    set total_bytes 0;

    set output [split $rawoutput ","];

    foreach entry $output {
	if {[regexp {chstats_files} $entry]} {
	    set val [string trim [lindex [split $entry "="] 1]];
	    incr total_files $val;
	}

	if {[regexp {chstats_bytes} $entry]} {
	    set val [string trim [lindex [split $entry "="] 1]];
	    incr total_bytes $val;
	}

	if {[regexp {chstats_time} $entry]} {
	    set chstats_time [string trim [lindex [split $entry "="] 1]];
	}

	if {[regexp {^stats_time} $entry]} {	             
	    set stats_time [string trim [lindex [split $entry "="] 1]];
	}
    }

    set seconds [clock seconds];

    set v(monitor_code) 0;
    set v(poll_time) $seconds;
    set v(stats_time) $stats_time;
    set v(chstats_time) $chstats_time;
    set v(stats_timediff) [expr $seconds - $stats_time];
    set v(chstats_timediff) [expr $seconds - $chstats_time];
    set v(total_files) $total_files;
    set v(total_bytes) $total_bytes;
    
    # make the tests

    if {[expr $v(stats_timediff) > 60]} {
	set v(monitor_code) $err_stats_time;
    }

    if {[expr $v(chstats_timediff) > 60]} {
	set v(monitor_code) $err_chstats_time;
    }

    if {($total_files == 0) || ($total_bytes == 0)} {
	set v(monitor_code) $err_chstats_data;
    }

    set rlist [list];
    foreach k $keys {
	lappend rlist $k $v($k);
    }
    
    return $rlist;
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

# defaults
set fmt $option(f);

set status [catch {
    set rawoutput [exec nbspstatcounters -f "csvk"];
} errmsg];

if {$status != 0} {
    puts $errmsg;
    return 1;
}

set rlist [monitor $rawoutput];
if {$fmt eq "yaml" } {
    foreach {k v} $rlist {
	puts "${k}:${v}";
    }
} elseif {$fmt eq "std"} {
    foreach {k v} $rlist {
	puts "${k}=${v}";
    }
} elseif {$fmt eq "csv"} {
    set r "";
    foreach {k v} $rlist {
	append r "$v,";
    }
    puts [string trim $r ","];
} elseif {$fmt eq "csvk"} {
    set r "";
    foreach {k v} $rlist {
	append r "$k=$v,";
    }
    puts [string trim $r ","];
} elseif {$fmt eq "xml"} {
    foreach {k v} $rlist {
	puts "<$k>$v</$k>";
    }
} else {
    puts "Invalid format: $fmt";
    return 1;
}
