#!%TCLSH%
#
# $Id$
#

# This is the "system scheduler", called by the nbspd daemon. It is not
# meant to be called by filters or scripts.  For that, use the nbspscheduler
# instead.

## The common defaults
source "/usr/local/etc/nbsp/filters.conf";

## The error library
package require nbsp::syslog;

## The scheduler library (load the library for minutely run schedules)
package require nbsp::mscheduler;

#
# Default schedule
#
set scheduler(rc) "scheduler.conf";

# Location (use the last one found)
set scheduler(confdirs) [concat $common(confdir) $common(localconfdirs)];

proc schedule {code args} {

    global g;
    
    set status [catch {
	set match [::nbsp::mscheduler::match_timespec $code];
    } errmsg];

    if {$status != 0} {
	::nbsp::syslog::warn $errmsg;
    } elseif {$match == 1} {
	lappend g(cmdlist) $args;
    }
}

proc source_rcfile {rcfile} {

    source $rcfile;
}

proc exec_cmd {cmd} {
    # 
    # Each memmber of the cmdlist is a list or the program and options.
    #

    set status [catch {
	eval exec $cmd;
    } errmsg];

    if {$status != 0} {
	::nbsp::syslog::warn $errmsg;
    }
}

#
# main
#

set rcfile "";
foreach _d $scheduler(confdirs) {
    set _f [file join ${_d} $scheduler(rc)]; 
    if {[file exists ${_f}]} {
	set rcfile ${_f};
    }
}
if {$rcfile eq ""} {
    ::nbsp::syslog::warn "$scheduler(rc) not found.";
    return 1;
}

set g(cmdlist) [list];
source_rcfile $rcfile;

foreach cmd $g(cmdlist) {
    exec_cmd $cmd;
}
