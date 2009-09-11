#
# $Id$
#
package provide errx 1.0;

namespace eval errx {}
namespace eval syslog {

    variable syslog;

    set syslog(usesyslog) 0;
}

proc ::errx::warn s {

    global argv0;

    set name [file tail $argv0];
    puts stderr "$name: $s";
}

proc ::errx::err s {

    warn $s;
    exit 1;
}

#
# syslog
#
proc ::syslog::usesyslog {{flag 1}} {

    variable syslog;

    set syslog(usesyslog) $flag;
}

proc ::syslog::_log_msg s {

    global argv0;

    set name [file tail $argv0];
    exec logger -t $name -- $s;
}

proc ::syslog::msg s {

    variable syslog;

    if {$syslog(usesyslog) == 1} {
	_log_msg $s;
    } else {
	puts $s;
    }
}

proc ::syslog::warn s {

    variable syslog;

    if {$syslog(usesyslog) == 1} {
	_log_msg $s;
    } else {
	::errx::warn $s;
    }
}

proc ::syslog::err s {

    variable syslog;

    if {$syslog(usesyslog) == 1} {
	_log_msg $s;
	exit 1;
    } else {
	::errx::err $s;
    }
}
