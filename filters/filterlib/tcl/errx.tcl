#
# $Id$
#
package provide nbsp::errx 1.0;
package require fileutil;

namespace eval nbsp::errx {}
namespace eval nbsp::filelog {}
namespace eval nbsp::syslog {

    variable syslog;

    set syslog(usesyslog) 0;
}

proc nbsp::errx::warn s {

    global argv0;

    set name [file tail $argv0];
    puts stderr "$name: $s";
}

proc nbsp::errx::errc s {

    warn $s;
}

proc nbsp::errx::err s {

    warn $s;
    exit 1;
}

#
# syslog
#
proc nbsp::syslog::usesyslog {{flag 1}} {

    variable syslog;

    set syslog(usesyslog) $flag;
}

proc nbsp::syslog::_log_msg s {

    global argv0;

    set name [file tail $argv0];
    exec logger -t $name -- $s;
}

proc nbsp::syslog::msg s {

    variable syslog;

    if {$syslog(usesyslog) == 1} {
	_log_msg $s;
    } else {
	puts $s;
    }
}

proc nbsp::syslog::warn s {

    variable syslog;

    if {$syslog(usesyslog) == 1} {
	_log_msg "Warning: $s";
    } else {
	::nbsp::errx::warn $s;
    }
}

proc nbsp::syslog::errc s {

    variable syslog;

    if {$syslog(usesyslog) == 1} {
	_log_msg "Error: $s";
    } else {
	::nbsp::errx::errc $s;
    }
}

proc nbsp::syslog::err s {

    variable syslog;

    if {$syslog(usesyslog) == 1} {
	_log_msg "Error: $s";
	exit 1;
    } else {
	::nbsp::errx::err $s;
    }
}

#
# Log to a file
#
proc nbsp::filelog::msg {logfile s} {

    global argv0;

    set name [file tail $argv0];
    append msg $name ": " [clock format [clock seconds]] ": " $s "\n"; 

    set status [catch {
	::fileutil::appendToFile $logfile $msg;
    } errmsg];

    if {$status != 0} {
	puts stderr $errmsg;
    }
}
