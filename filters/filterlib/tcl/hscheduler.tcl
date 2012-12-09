#
# $Id$
#
# This package contains the functions that should be used in the schedule
# scripts that are invoked hourly (e.g., the "nbspgribsched").
# The analogous functions for the minutely schedules (e.g., scheduler)
# are in mscheduler.tcl.
#
# The main function of the library is
#
#     proc nbsp::hscheduler::match_timespec {spec}
#
# which returns 1 if the argument passed matches the current date-time
# or 0 otherwise.
#
# The curret date-time is calculated in gmt if the <spec>
# ends with "g"; otherwise local time is used. The "spec" argument is a time
# specification in one of the following forms, optionally followed by "g"
# as mentioned above:
#
# H		=> run every hour
# H=ll|mm|nn	=> run at the hours in the list
# H/n		=> run if the remainder of the (current hour)/n is zero. 
#
# D=ddhh|d'd'h'h'|...	=> run at the days of the month and hours in the list.
#			   Here the dd is between 01-31.
# Dhh|h'h'|.../n	=> run if the remainder of (current day of month)/n
#			   is zero and the current hour is in the list.
#			
# W=dhh|d'h'h'|...	=> run at the days of the week and hours in the list.
#			   Here the d is between 0-6 with 0 = sunday.
# Whh|h'h'|.../n	=> run if the remainder of (current day of week)/n
#			   is zero and the current hour is in the list.
#
# Examples
#
# H/3  => run at 0, 3, 6, ...
# H=03|09|18|23  => run at 03, 09, 18 and 23.
# H=03|09|18|23g => same as above with times relative to gmt
# W=523 => run on fridays at 11 pm
# D=0101 => run on the first of the month at 1 am.
# D03|21/2  => run every other day at 3 am and 9 pm.
#
package provide nbsp::hscheduler 1.0;
namespace eval nbsp::hscheduler {

    variable option;

    set option(gmt) "";
}

proc nbsp::hscheduler::match_timespec {spec} {
#
# Returns 1 if the argument passed matches the current date-time
# or 0 otherwise.
#
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
        H* {set match [match_hourspec $spec]}
        D* {set match [match_dayspec $spec]}
        W* {set match [match_wdayspec $spec]}
        default {return -code error \
		     "Error: Unrecognized time specification: $spec"}
    }

    return $match;
}

proc nbsp::hscheduler::match_hourspec {hour} {

    variable option;

    if {$hour eq "H"} {
        return 1;
    }

    set seconds [clock seconds];
    set hh [eval clock format $seconds -format "%H" $option(gmt)];
    set h [eval clock format $seconds -format "%k" $option(gmt)];

    set match 0;
    switch -glob $hour {
        H/* {
            set specparts [split $hour "/"];
            set interval [lindex $specparts 1];
            if {[expr $h % $interval] == 0} {
                set match 1;
            }
        }
        H=* {
            set specparts [split $hour "="];
            set hourlist [lindex $specparts 1];
            if {[regexp $hourlist $hh]} {
                set match 1;
            }
        }           
        default {return -code error \
		     "Error: Unrecognized hour specification: $hour"}
    }

    return $match;
}

proc nbsp::hscheduler::match_dayspec {dayspec} {

    variable option;

    set seconds [clock seconds];
    set ddhh [eval clock format $seconds -format "%d%H" $option(gmt)];
    set hh [eval clock format $seconds -format "%H" $option(gmt)];
    set  Dhh "D$hh"; 
    set day [eval clock format $seconds -format "%e" $option(gmt)];

    set match 0;
    switch -glob $dayspec {
        D*/* {
            set specparts [split $dayspec "/"];
            set interval [lindex $specparts 1];
	    set Dhhspec [lindex $specparts 0];
            if {([expr $day % $interval] == 0) && \
		    [regexp $Dhhspec $Dhh]} {
                set match 1;
            }
        }
        D=* {
            set specparts [split $dayspec "="];
            set ddhhlist [lindex $specparts 1];
            if {[regexp $ddhhlist $ddhh]} {
                set match 1;
            }
        }           
        default {return -code error \
		     "Error: Unrecognized daily specification: $dayspec"}
    }

    return $match;
}

proc nbsp::hscheduler::match_wdayspec {wdayspec} {

    variable option;

    set seconds [clock seconds];
    set wdayhh [eval clock format $seconds -format "%w%H" $option(gmt)];
    set hh [eval clock format $seconds -format "%H" $option(gmt)];
    set wday [eval clock format $seconds -format "%w" $option(gmt)];
    set Whh "W$hh";

    set match 0;
    switch -glob $wdayspec {
        W*/* {
            set specparts [split $wdayspec "/"];
            set interval [lindex $specparts 1];
	    set Whhspec [lindex $specparts 0];
            if {([expr $wday % $interval] == 0) && \
		    [regexp $Whhspec $Whh]} {
                set match 1;
            }
        }
        W=* {
            set specparts [split $wdayspec "="];
            set wdayhhlist [lindex $specparts 1];
            if {[regexp $wdayhhlist $wdayhh]} {
                set match 1;
            }
        }           
        default {return -code error \
		     "Error: Unrecognized weekly specification: $wdayspec"}
    }

    return $match;
}
