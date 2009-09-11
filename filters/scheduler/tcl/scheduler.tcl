#!%TCLSH%
#
# $Id$
#

# This is the "system scheduler", called by the nbspd daemon. It is not
# meant to be called by filters or scripts.  For that, use the nbspscheduler
# instead.

## The common defaults
set defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaultsfile] == 0} {
    puts "$argv0: $defaultsfile not found.";
    return 1;
}
source $defaultsfile;
unset defaultsfile;

## The filter error function library
if {[file exists $common(filterserrlib)] == 0} {
    puts "$argv0: $common(filterserrlib) not found.";
    return 1;
}
source $common(filterserrlib);

## The scheduler library (load the library for minutely run schedules)
package require mscheduler;

#
# Default schedule
#
set scheduler(rc) "scheduler.conf";

# Location (use the last one found)
set scheduler(confdirs) [concat $common(confdir) $common(localconfdirs)];

proc schedule {code args} {

    global g;
    
    set status [catch {
	set match [::mscheduler::match_timespec $code];
    } errmsg];

    if {$status != 0} {
	log_msg $errmsg;
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
	log_msg $errmsg;
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
    log_msg "$scheduler(rc) not found.";
    return 1;
}

set g(cmdlist) [list];
source_rcfile $rcfile;

foreach cmd $g(cmdlist) {
    exec_cmd $cmd;
}
