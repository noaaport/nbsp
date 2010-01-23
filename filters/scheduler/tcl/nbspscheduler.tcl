#!%TCLSH%
#
# $Id$
#
# This is a general purpose script that works similarly to the scheduler,
# but meant to be used from scripts.
#
# [-h] => use the hscheduler timespec syntax (default)
# [-m] => use the mscheduler (like the scheduler) syntax
# [-M] => define variables
# [-b] => use syslog
# [-f] => Use the schedule file as is. Otherwise it looks for a file with the
#         same name in localconfdirs and it uses the last one found.
# [-v] => verbose
#
set usage {usage: nbspscheduler [-b] [-h|-m] [-M <vardefines> ...] [-f] [-v]
    <schedulefile>};
set optlist [list b f h m M.marg v];

set defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaultsfile] == 0} {
    puts "[file tail $argv0]: $defaultsfile not found.";
    return 1;
}
source $defaultsfile;
unset defaultsfile;
#
package require nbsp::mscheduler;
package require nbsp::hscheduler;
package require nbsp::errx;
package require nbsp::util;

# Inherit (some) global variables for easy reference later
set g(localconfdirs) $common(localconfdirs);

proc schedule {code args} {

    global option;
    global g;

    set status [catch {
	if {$option(m) == 1} {
	    set match [::nbsp::mscheduler::match_timespec $code];
	} else {
	    set match [::nbsp::hscheduler::match_timespec $code];
	}
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
    # cmd is a list of the program and options.
    #

    set status [catch {
	eval exec $cmd;
    } errmsg];

    if {$status != 0} {
	::nbsp::syslog::warn $errmsg;
    }
}

proc configure_defines {defines} {

    foreach key $defines {
        set pair [::nbsp::util::split_first $key "="];
        set i [lindex $pair 0];
        set v [lindex $pair 1];
        ::nbsp::util::set_var $i $v;
    }
}

#
# main
#
array set option [::nbsp::util::cmdline_getoptions argv $optlist $usage];

if {$option(b) == 1} {
    ::nbsp::syslog::usesyslog;
}

if {($option(h) == 1) && ($option(m) == 1)} {
    ::nbsp::syslog::err $usage;
}

set argc [llength $argv];
if {$argc == 0} {
    ::nbsp::syslog::err $usage;
}

set rcfile [lindex $argv 0];
if {$option(f) == 0} {
    set rcfile [::nbsp::util::find_local_rcfile $rcfile $g(localconfdirs)];
}
if {[file exists $rcfile] == 0} {
    ::nbsp::syslog::err "$rcfile not found.";
}

set g(cmdlist) [list];
source_rcfile $rcfile;

configure_defines $option(M);

foreach cmd $g(cmdlist) {
    if {$option(v) == 1} {
	::nbsp::syslog::msg "Executing $cmd";
    }
    # each memmber of the cmdlist is a list or the program and options.
    exec_cmd $cmd;
}
