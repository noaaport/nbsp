#
# $Id$
#
#    nbsputil::cmdline_getoptions {argv_name optlist usage}
#    nbsputil::split_first {str substr}
#    nbsputil::set_var {varname varval}
#    nbsputil::get_var {varname}
#    nbsputil::pread {cmd}
#    nbsputil::pwrite_block {data args}
#    nbsputil::pwrite_nonblock {data args}
#    nbsputil::find_local_rcfile {rcfile localconfdirs {subdir ""}}
#
#    nbsputil::date::guess_clock_seconds {ddhhmm}
#    nbsputil::date::clock_seconds {ddhhmm}
#
package provide nbsp::util 1.0;
package require cmdline;

namespace eval nbsp::util {

    variable nbsputil;

    array set nbsputil {};
}

namespace eval nbsp::util::date {};

proc nbsp::util::cmdline_getoptions {argv_name optlist usage} {
#
# Similar to the standard one, with the following differences:
#
# (1) The options can have a ".marg" extension to indicate that they can
# appear multiple times. The corresponding result option(<key>) is a tcl list.
# (2) The optstrlist must be in the format accepted by the getopt
# function not the getoptions. For example,
#
# set optlist [list D.marg M.arg v];
# array set option [nbsputil_cmdline argv $optlist $usage];
#
    upvar 1 $argv_name argv;

    # Loop through the options specifications and do three things:
    # (1) initialize the option results
    # (2) find which options can be specified multiple times
    # (3) reformat optstr as required by the standard cmdline functions

    set std_optlist [list];
    set multiple_optlist [list];
    foreach key $optlist {
	if {[regexp {(.+)\.marg$} $key match s]} {
	    set option($s) [list];
	    lappend multiple_optlist $s;
	    lappend std_optlist ${s}.arg;
	} elseif {[regexp {(.+)\.arg$} $key match s]} {
	    set option($s) "";
	    lappend std_optlist ${s}.arg;
	} else {
	    set option($key) 0;
	    lappend std_optlist $key;
	}
    }

    # Call the standard getopt

    set c 1;
    while {$c > 0} {
	set c [::cmdline::getopt argv $std_optlist optname optval];
	if {$c > 0} {
	    if {[lsearch -exact $multiple_optlist $optname] != -1} {
		lappend option($optname) $optval;
	    } else {
		set option($optname) $optval;
	    }
	}
    }
    if {$c == -1} {
	return -code error "Invalid option $optname.\n$usage";
    }
    
    # Returns a list of key/value pairs
    return [array get option];
}

proc nbsp::util::split_first {str substr} {
#
# Given a string in str, returns the two parts that are separated
# by the first instance of substr.
#
    set i [string first $substr $str];
    set s2 [string trimleft [string range $str $i end] $substr];
    incr i -1;
    set s1 [string range $str 0 $i];

    return [list $s1 $s2];
}

proc nbsp::util::set_var {varname varval} {

    variable nbsputil;

    set nbsputil(var,$varname) $varval;
}

proc nbsp::util::get_var {varname} {

    variable nbsputil;

    if {[info exists nbsputil(var,$varname)] == 0} {
	return -code error "$varname is not defined.";
    }

    return $nbsputil(var,$varname);
}

proc nbsp::util::pread {args} {
#
# A substitute for
#
#   set content [exec <program> <options>];
#
# when the output from <program> is binary data that we don't want exec
# to modify.
#
    set s [join $args " "];

    set content "";
    set status [catch {
	set F [open "|$s" r];
	fconfigure $F -encoding binary -translation binary
	set content [read $F];
    } errmsg];

    if {[info exists F]} {
	catch {close $F};
    }

    if {$status != 0} {
	return -code error $errmsg;
    }

    return $content;
}

proc nbsp::util::pwrite_block {data args} {

    set s [join $args " "];
    set status [catch {
        set F [open "|$s" w];
        fconfigure $F -buffering none -encoding binary -translation binary;
	puts $F $data;
    } errmsg];

    if {[info exists F]} {
	catch {close $F};
    }

    if {$status != 0} {
	return -code error $errmsg;
    }
}

proc nbsp::util::pwrite_nonblock {data args} {

    set s [join $args " "];
    set status [catch {
        set F [open "|$s" w];
        fconfigure $F -buffering none -encoding binary -translation binary;
	puts $F $data;
        fconfigure $F -blocking 0;
    } errmsg];

    if {[info exists F]} {
	catch {close $F};
    }

    if {$status != 0} {
	return -code error $errmsg;
    }
}

proc nbsp::util::find_local_rcfile {rcfile localconfdirs {subdir ""}} {
#
# Looks for a file with the same name as rcfile, in the local directories. If
# it is found, the path is returned, otherwise the original rcfile is returned.
# The <subdir> can specify a subdir of the localconfdirs to look.
# (The function has the same functionality as filterlib_find_conf
# in filters.lib, except that here it always returns the original rcfile
# if a local version is not found.
#
    set rcname [file tail $rcfile];    
    set r $rcfile;

    foreach d $localconfdirs {
        if {$subdir ne ""} {
            set f [file join $d $subdir $rcname];
        } else {
            set f [file join $d $rcname];
        }
        if {[file exists $f]} {
            set r $f;
        }
    }

    return $r;
}

#
# nbsputil::date
#

proc nbsp::util::date::guess_clock_seconds {ddhhmm} {
#
# The algorith will be like this. First we calculate "now" as
# ``clock seconds''. From that we determine three points, in seconds:
#
# (1) the start of the current month
# (2) the start of the previous month
# (3) the start of the next month
#
# Then we convert the  ddhhmm to seconds, and calculate three candidates
# by adding the result to the start times (1-3) above. The winner will
# be the one that is the closest to ``now''.
#
# In addition, we exclude from the above candidates the case in which
# dd is not valid  for the given month.
#
# Of course this is an undetermined problem, so that we will be our best guess.

    if {[regexp {(\d{2})(\d{2})(\d{2})} $ddhhmm match dd HH MM] == 0} {
	return -code error "Invalid date.";
    }

    scan $dd "%d" d;
    scan $HH "%d" H;
    scan $MM "%d" M;

    # The number of seconds in ddhhmm
    set ddhhmm_seconds [expr $M * 60 + $H * 3600 + ($d - 1) * 24 * 3600];

    set now [clock seconds];

    scan [clock format $now -format "%M" -gmt true] "%d" current_minute;
    scan [clock format $now -format "%H" -gmt true] "%d" current_hour;
    scan [clock format $now -format "%S" -gmt true] "%d" current_second;
    scan [clock format $now -format "%d" -gmt true] "%d" current_day;
    scan [clock format $now -format "%m" -gmt true] "%d" current_month;
    scan [clock format $now -format "%Y" -gmt true] "%d" current_year;

    # The number of seconds at the start of the current month
    set current_month_seconds [expr $now \
				   - $current_second \
				   - $current_minute * 60 \
				   - $current_hour * 3600 \
				   - ($current_day - 1) * 24 * 3600];

    set start_candidates [list];

    if {$d <= [_days_in_month $current_year $current_month]} {
	lappend start_candidates $current_month_seconds;
    }

    # Find the start of the previous month
    set month $current_month;
    incr month -1;
    set year $current_year
    if {$month == 0} {
	incr year -1;
	set month 12;
    }
    set days_in_month [_days_in_month $year $month];
    set prev_month_seconds \
	[expr $current_month_seconds - $days_in_month * 24 * 3600];

    if {$d <= $days_in_month} {
	lappend start_candidates $prev_month_seconds; 
    }

    # Same for next month
    set days_in_month [_days_in_month $current_year $current_month];
    set next_month_seconds \
	[expr $current_month_seconds + $days_in_month * 24 * 3600];

    set month $current_month;
    set year $current_year;
    incr month;
    if {$month == 13} {
	set month 1;
	incr year;
    }
    if {$d <= [_days_in_month $year $month]} {
	lappend start_candidates $next_month_seconds;
    }

    # The comparison
    set r "";
    set diff $now;
    foreach base $start_candidates {
	set _r [expr $base + $ddhhmm_seconds];
	set _diff [expr abs($_r - $now)];
	if {$_diff < $diff} {
	    set diff $_diff;
	    set r $_r;
	}
    }

    return $r;
}

proc nbsp::util::date::clock_seconds {ddhhmm} {

    set s "";

    if {$ddhhmm ne ""} {
	set status [catch {
	    set s [guess_clock_seconds $ddhhmm];
	} errmsg];
    }
    if {$s eq ""} {
	set s [clock seconds];
    }

    return $s;
}

proc nbsp::util::date::_is_leap_year {yyyy} {
#
# KR p.41
#
    if {(([expr $yyyy % 4] == 0) && ([expr $yyyy % 100] != 0)) || \
	     ([expr $yyyy % 400] == 0)} {
	return 1;
    }

    return 0;
}

proc nbsp::util::date::_days_in_month {year month} {

    set days(1) 31;
    set days(2) 28;
    set days(3) 31;
    set days(4) 30;
    set days(5) 31;
    set days(6) 30;
    set days(7) 31;
    set days(8) 31;
    set days(9) 30;
    set days(10) 31;
    set days(11) 30;
    set days(12) 31;

    if {[_is_leap_year $year]} {
	set days(2) 29;
    }

    return $days($month);
}

proc nbsp::util::date::_debug {s} {

    puts $s;
}
