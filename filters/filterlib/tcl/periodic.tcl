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
# call ``::nbsp::syslog::usesyslog 0'' after loading this package.
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
# ::nbsp::periodic::register <cmd> <period>
#
# where <cmd> is a procedure. <period> is the period in seconds or the words
# "hourly" or "minutely".
# 
# 2) Call
#
# ::nbsp::periodic::run
#
# regularly as often as possible.
#
# 3) A filter that needs to execute the nbsp scheduler should register a
#    function, e.g., ``xxx_run_scheduler''
#
# ::nbsp::periodic::register xxx_run_scheduler "minutely"
#
# The function ``xxx_run_scheduler'' in turn calls
#
# ::nbsp::periodic::scheduler <schedule file>
#
package provide nbsp::periodic 1.0;
package require nbsp::errx;

namespace eval nbsp::periodic {} {

    variable periodic;

    set periodic(verbose) 0;
    set periodic(cmd_list) [list];
    set run_period "minutely";		# default

    set now [clock seconds];
    scan [clock format $now -format "%S"] "%d" current_second;
    set run_time [expr $now - $current_second + 60];

    set periodic(run_period) $run_period;
    set periodic(run_time) $run_time;

    ::nbsp::syslog::usesyslog;
}

proc nbsp::periodic::set_run_period {run_period} {

    variable periodic;

    set now [clock seconds];

    set periodic(run_period) $run_period;
    set periodic(run_time) \
	[::nbsp::periodic::_run_time_reset $now $run_period];
}

proc nbsp::periodic::get_run_period {} {

    variable periodic;

    return $periodic(run_period);
}

proc nbsp::periodic::register {cmd {run_period ""}} {
#
# run_period is a number in seconds, or:
#   - the keyword "hourly", in this case the command is run every hour
#     at minute 0
#   - the keyword "minutely", in which case the command is run every minute
#     at second 0.
# 
    variable periodic;

    set now [clock seconds];

    if {$run_period eq ""} {
	set run_period $periodic(run_period);
    }

    set run_time [::nbsp::periodic::_run_time_reset $now $run_period];

    lappend periodic(cmd_list) $cmd;
    set periodic($cmd,run_period) $run_period;
    set periodic($cmd,run_time) $run_time;
}

proc nbsp::periodic::run {} {

    variable periodic;

    set now [clock seconds];
    if {$now < $periodic(run_time)} {
	return;
    }

    if {$periodic(verbose) == 1} {
	::nbsp::syslog::msg "Executing ::nbsp::periodic::run";
    }

    set periodic(run_time) [::nbsp::periodic::_run_time_reset \
				$now $periodic(run_period)];

    foreach cmd $periodic(cmd_list) {
	set status [catch {
	    ::nbsp::periodic::_run_cmd $cmd;
	} errmsg];

	if {$status != 0} {
	    ::nbsp::syslog::warn $errmsg;
	}
    }
}

proc nbsp::periodic::scheduler {rcfile} {

    variable periodic;

    if {[file exists $rcfile] == 0} {
	::nbsp::syslog::warn "$rcfile not found.";
    } else {
	if {$periodic(verbose) == 1} {
	    ::nbsp::syslog::msg "Executing nbspscheduler";
	}

	set status [catch {
	    exec nbspexec nbspscheduler -b -v -m $rcfile;
	} errmsg];

	if {$status != 0} {
	    ::nbsp::syslog::warn $errmsg;
	} elseif {$periodic(verbose) == 1} {
	    ::nbsp::syslog::msg "Launched nbspscheduler -m $rcfile";
        }
    }
}

#
# private
#
proc nbsp::periodic::_run_cmd {cmd} {
#
# Checks if it is time to run a registered command and the runs it
# and reinitializes its timer.
#
    variable periodic;

    ::nbsp::periodic::_verify_cmd $cmd;

    set now [clock seconds];
    if {$now < $periodic($cmd,run_time)} {
	return;
    }

    if {$periodic(verbose) == 1} {
	::nbsp::syslog::msg "Executing $cmd in ::nbsp::periodic::_run_cmd";
    }

    set periodic($cmd,run_time) \
	[::nbsp::periodic::_run_time_reset $now $periodic($cmd,run_period)];

    eval $cmd;
}

proc nbsp::periodic::_verify_cmd {cmd} {

    variable periodic;

    if {([info exists periodic($cmd,run_period)] == 0) ||
	([info exists periodic($cmd,run_time)] == 0)} {
	    return -code error "$cmd is not registered.";
    }
}

proc nbsp::periodic::_hourly_run_time_reset {now} {
#
# This function returns the time corresponding to the next hour.
# now is the result of "clock seconds"
#
    scan [clock format $now -format "%M"] "%d" current_minute;

    return [expr $now - ($current_minute * 60) + 3600];
}

proc nbsp::periodic::_minutely_run_time_reset {now} {
#
# This function returns the time corresponding to the next minute.
# now is the result of "clock seconds"
#
    scan [clock format $now -format "%S"] "%d" current_second;

    return [expr $now - $current_second + 60];
}

proc nbsp::periodic::_run_time_reset {now run_period} {

    if {$run_period eq "minutely"} {
	set run_time [::nbsp::periodic::_minutely_run_time_reset $now];
    } elseif {$run_period eq "hourly"} {
	set run_time [::nbsp::periodic::_hourly_run_time_reset $now];
    } else {
	set run_time [expr $now + $run_period];
    }

    return $run_time;
}

proc nbsp::periodic::_verbose {{v 1}} {
#
# Mostly for debuging
#
    set periodic(verbose) $v;
}
