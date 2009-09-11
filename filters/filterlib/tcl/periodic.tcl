#
# $Id$
#

# This package contains a set of functions to be used by the
# filters. The package uses the package "errx"
#
# and the programs
#
# nbspexec
# nbspscheduler
#
# It uses syslog. If that is not desired, then the caller must
# call ``::syslog::usesyslog 0'' after loading this package.
#
# The external functions are:
#
#   periodic::set_run_period {run_period} (default is "minutely")
#   periodic::get_run_period
#   periodic::register {cmd run_period}
#   periodic::run
#   periodic::scheduler {rcfile}
#
# Usage -
#
# 1) Register a function by
#
# ::periodic::register <cmd> <period>
#
# where <cmd> is a procedure. <period> is the period in seconds or the words
# "hourly" or "minutely".
# 
# 2) Call
#
# ::periodic::run
#
# regularly as often as possible.
#
# 3) A filter that needs to execute the nbsp scheduler should register a
#    function, e.g., ``xxx_run_scheduler''
#
# ::periodic::register xxx_run_scheduler "minutely"
#
# The function ``xxx_run_scheduler'' in turn calls
#
# ::periodic::scheduler <schedule file>
#
package provide periodic 1.0;
package require errx;

namespace eval periodic {} {

    variable periodic;

    set periodic(verbose) 0;
    set periodic(cmd_list) [list];
    set run_period "minutely";		# default

    set now [clock seconds];
    scan [clock format $now -format "%S"] "%d" current_second;
    set run_time [expr $now - $current_second + 60];

    set periodic(run_period) $run_period;
    set periodic(run_time) $run_time;

    ::syslog::usesyslog;
}

proc ::periodic::set_run_period {run_period} {

    variable periodic;

    set now [clock seconds];

    set periodic(run_period) $run_period;
    set periodic(run_time) [::periodic::_run_time_reset $now $run_period];
}

proc ::periodic::get_run_period {} {

    variable periodic;

    return $periodic(run_period);
}

proc ::periodic::register {cmd run_period} {
#
# run_period is a number in seconds, or the keyword "hourly". In this case
# the command is run on the hour every hour.
# 
    variable periodic;

    set now [clock seconds];

    set run_time [::periodic::_run_time_reset $now $run_period];

    lappend periodic(cmd_list) $cmd;
    set periodic($cmd,run_period) $run_period;
    set periodic($cmd,run_time) $run_time;
}

proc ::periodic::run {} {

    variable periodic;

    set now [clock seconds];
    if {$now < $periodic(run_time)} {
	return;
    }

    if {$periodic(verbose) == 1} {
	::syslog::msg "Executing ::periodic::run";
    }

    set periodic(run_time) [::periodic::_run_time_reset \
				$now $periodic(run_period)];
    set msg "";
    foreach cmd $periodic(cmd_list) {
	set status [catch {
	    ::periodic::_run_cmd $cmd;
	} errmsg];

	if {$status != 0} {
	    ::syslog::warn $errmsg;
	}
    }
}

proc ::periodic::scheduler {rcfile} {

    variable periodic;

    if {[file exists $rcfile] == 0} {
	log_msg "$rcfile not found.";
    } else {
	if {$periodic(verbose) == 1} {
	    log_msg "Executing nbspscheduler";
	}

	set status [catch {
	    exec nbspexec nbspscheduler -b -v -m $rcfile;
	} errmsg];

	if {$status != 0} {
	    ::syslog::warn $errmsg;
	} elseif {$periodic(verbose) == 1} {
	    ::syslog::msg "Launched nbspscheduler -m $rcfile";
        }
    }
}

#
# private
#
proc ::periodic::_run_cmd {cmd} {
#
# Checks if it is time to run a registered command and the runs it
# and reinitializes its timer.
#
    variable periodic;

    ::periodic::_verify_cmd $cmd;

    set now [clock seconds];
    if {$now < $periodic($cmd,run_time)} {
	return;
    }

    if {$periodic(verbose) == 1} {
	::syslog::msg "Executing $cmd in ::periodic::_run_cmd";
    }

    set periodic($cmd,run_time) \
	[::periodic::_run_time_reset $now $periodic($cmd,run_period)];

    eval $cmd;
}

proc ::periodic::_verify_cmd {cmd} {

    variable periodic;

    if {([info exists periodic($cmd,run_period)] == 0) ||
	([info exists periodic($cmd,run_time)] == 0)} {
	    return -code error "$cmd is not registered.";
    }
}

proc ::periodic::_hourly_run_time_reset {now} {
#
# This function returns the time corresponding to the next hour.
# now is the result of "clock seconds"
#
    scan [clock format $now -format "%M"] "%d" current_minute;

    return [expr $now - ($current_minute * 60) + 3600];
}

proc ::periodic::_minutely_run_time_reset {now} {
#
# This function returns the time corresponding to the next minute.
# now is the result of "clock seconds"
#
    scan [clock format $now -format "%S"] "%d" current_second;

    return [expr $now - $current_second + 60];
}

proc ::periodic::_run_time_reset {now run_period} {

    if {$run_period eq "minutely"} {
	set run_time [::periodic::_minutely_run_time_reset $now];
    } elseif {$run_period eq "hourly"} {
	set run_time [::periodic::_hourly_run_time_reset $now];
    } else {
	set run_time [expr $now + $run_period];
    }

    return $run_time;
}

proc ::periodic::_verbose {{v 1}} {
#
# Mostly for debuging
#
    set periodic(verbose) $v;
}
