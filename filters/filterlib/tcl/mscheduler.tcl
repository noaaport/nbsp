#
# $Id$
#
# This package contains the functions that should be used in the schedule
# scripts that are invoked each minute (e.g., the "scheduler").
# The analogous functions for the hourly schedules (e.g., nbspgribsched)
# are in hscheduler.tcl.
#
# The main function of the library is
#
#     proc ::mscheduler::match_timespec <spec>
#
# which returns 1 if the argument passed matches the current date-time
# or 0 otherwise.
#
# The curret date-time is calculated in gmt if the <spec>
# ends with "g"; otherwise local time is used. The "spec" argument is a time
# specification in one of the following forms, optionally followed by "g"
# as mentioned above:
#
# M		=> run <program> every minute
# M=ll|mm|nn	=> run at the minutes in the list (e.g., 00, 15, 30, 45)
# M/n		=> run if the remainder of the (current minute)/n is zero. 
#
# H=hhmm|h'h'm'm'|...	=> run at the hours and minutes in the list.
# Hmm|m'm'|.../n	=> run if the remainder of (current hour)/n is zero
#                          and if the current minute is in the list
#
# D=ddhhmm|d'd'h'h'm'm'|... => run at the days of the month
#                              and hours and minutes in the list.
#			       Here the dd is between 01-31.
# Dhhmm|h'h'm'm'|.../n	=> run if the remainder of (current day of month)/n
#			   is zero and the current hour and minute are
#			   in the list.
#			
# W=dhhmm|d'h'h'm'm'|... => run at the days of the week and hours and minutes
#                           in the list.
#			    Here the d is between 0-6 with 0 = sunday.
# Whhmm|h'h'm'm'|.../n	=> run if the remainder of (current day of week)/n
#			   is zero and the current hour and minute are
#                          in the list.
#
# Examples
#
# M      => run every minute
# M/4    => run every four minutes
# M=03|09|18|23  => run at minutes 03, 09, 18 and 23.
# H00/3  => run every three hours: 0, 3, 6, ...
# H=0300|0900|1800|2355  => run at 03, 09, 18 and 23:55
# H=0300|0900|1800|2355g => same thing but the times are relative to gmt
# W=52300 => run on fridays at 11 pm
# D=010100 => run on the first day of the month at 1 am.
# D0300|2100/2  => run every other day at 3 am and 9 pm.
#
package provide mscheduler 1.0;
namespace eval mscheduler {

    variable option;

    set option(gmt) "";
}

proc ::mscheduler::match_timespec {spec} {
#
# Returns 1 if the argument passed matches the current date-time
# or 0 otherwise.

    variable option;

    # Check first the gmt flag
    if {[regexp {(.+)g$} $spec match s1]} {
	set spec $s1;
	set option(gmt) "-gmt true";
    } else {
	set option(gmt) "";
    }

    set match 0;
    switch -glob $spec {
	M* {set match [match_minutespec $spec]}
        H* {set match [match_hourspec $spec]}
        D* {set match [match_dayspec $spec]}
        W* {set match [match_wdayspec $spec]}
        default {return -code error "Unrecognized time specification: $spec"}
    }

    return $match;
}

proc ::mscheduler::match_minutespec {code} {

    variable option;

    if {$code eq "M"} {
        return 1;
    }

    set seconds [clock seconds];
    set mm [eval clock format $seconds -format "%M" $option(gmt)];
    regsub {0(.+)} $mm {\1} m;

    set match 0;
    switch -glob $code {
        M/* {
            set specparts [split $code "/"];
            set interval [lindex $specparts 1];
            if {[expr $m % $interval] == 0} {
                set match 1;
            }
        }
        M=* {
            set specparts [split $code "="];
            set minutelist [lindex $specparts 1];
            if {[regexp $minutelist $mm]} {
                set match 1;
            }
        }           
        default {return -code error "Unrecognized hour specification: $code"}
    }

    return $match;
}

proc ::mscheduler::match_hourspec {hourspec} {

    variable option;

    set seconds [clock seconds];
    set hhmm [eval clock format $seconds -format "%H%M" $option(gmt)];
    set mm [eval clock format $seconds -format "%M" $option(gmt)];
    set  Hmm "H$mm"; 
    set hour [eval clock format $seconds -format "%k" $option(gmt)];

    set match 0;
    switch -glob $hourspec {
        H*/* {
            set specparts [split $hourspec "/"];
            set interval [lindex $specparts 1];
	    set Hmmspec [lindex $specparts 0];
            if {([expr $hour % $interval] == 0) && \
		    [regexp $Hmmspec $Hmm]} {
                set match 1;
            }
        }
        H=* {
            set specparts [split $hourspec "="];
            set hhmmlist [lindex $specparts 1];
            if {[regexp $hhmmlist $hhmm]} {
                set match 1;
            }
        }           
        default {return -code error \
		     "Unrecognized daily specification: $hourspec"}
    }

    return $match;
}

proc ::mscheduler::match_dayspec {dayspec} {

    variable option;

    set seconds [clock seconds];
    set ddhhmm [clock format $seconds -format "%d%H%M" $option(gmt)];
    set hhmm [clock format $seconds -format "%H%M" $option(gmt)];
    set  Dhhmm "D$hhmm"; 
    set day [clock format $seconds -format "%e" $option(gmt)];

    set match 0;
    switch -glob $dayspec {
        D*/* {
            set specparts [split $dayspec "/"];
            set interval [lindex $specparts 1];
	    set Dhhmmspec [lindex $specparts 0];
            if {([expr $day % $interval] == 0) && \
		    [regexp $Dhhmmspec $Dhhmm]} {
                set match 1;
            }
        }
        D=* {
            set specparts [split $dayspec "="];
            set ddhhmmlist [lindex $specparts 1];
            if {[regexp $ddhhmmlist $ddhhmm]} {
                set match 1;
            }
        }           
        default {return -code error \
		     "Unrecognized daily specification: $dayspec"}
    }

    return $match;
}

proc ::mscheduler::match_wdayspec {wdayspec} {

    variable option;

    set seconds [clock seconds];
    set wdayhhmm [clock format $seconds -format "%w%H%M" $option(gmt)];
    set hhmm [clock format $seconds -format "%H%M" $option(gmt)];
    set wday [clock format $seconds -format "%w" $option(gmt)];
    set Whhmm "W$hhmm";

    set match 0;
    switch -glob $wdayspec {
        W*/* {
            set specparts [split $wdayspec "/"];
            set interval [lindex $specparts 1];
	    set Whhmmspec [lindex $specparts 0];
            if {([expr $wday % $interval] == 0) && \
		    [regexp $Whhmmspec $Whhmm]} {
                set match 1;
            }
        }
        W=* {
            set specparts [split $wdayspec "="];
            set wdayhhmmlist [lindex $specparts 1];
            if {[regexp $wdayhhmmlist $wdayhhmm]} {
                set match 1;
            }
        }           
        default {return -code error \
		     "Unrecognized weekly specification: $wdayspec"}
    }

    return $match;
}
