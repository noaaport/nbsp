#!%TCLSH%
#
# $Id$
#
# usage: nbspctl [-b] <start|stop>
#
# This script calls the installed init script

set usage {nbspctl [-b] <start|stop>};
set optlist {b};

#
source "/usr/local/etc/nbsp/filters.conf";
source "/usr/local/libexec/nbsp/nbspd.init";

# Packages from tcllib
package require cmdline;

# Nbsp packages
## The errx library. syslog enabled below if -b is given.
package require nbsp::errx;

# configuration
set nbspctl(rcfpath) "%RCFPATH%";

#
# init
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$option(b) == 1} {
    ::nbsp::syslog::usesyslog
}

if {$argc != 1} {
    ::nbsp::syslog::err $usage;
}
set stage [lindex $argv 0];

#
# main
#

set status [catch {
    exec $nbspctl(rcfpath) $stage;
} errmsg];

if {$status != 0} {
    ::nbsp::syslog::err $errmsg;
}

return $status;
