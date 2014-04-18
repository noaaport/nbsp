#!%TCLSH%
#
# $Id$
#

set usage {capcleanup [-f namefmt] [<invdir>]};
set optlist {b {f.arg "%Y-%m-%d.log"}};

# To find the nbsp packages
source "/usr/local/etc/nbsp/filters.conf";

# Load the capfilter variables
source [file join $common(libdir) "capfilter.init"];

# tcllib packages
package require cmdline;

# Nbsp packages - syslog enabled below if -b is given.
package require nbsp::errx;

# variables
set g(invdir) $capfilter(invdir);

#
# functions
#
proc nbspcapcleanup_delete_file {fpath} {

    set status [catch {
	file delete $fpath;
    } errmsg];

    if {$status != 0} {
	::nbsp::syslog::err $errmsg;
    }
}

#
# init
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$option(b) == 1} {
    ::nbsp::syslog::usesyslog
}

if {$argc > 1} {
        ::nbsp::syslog::err $usage;
} elseif {$argc == 1} {
    set g(invdir) [lindex $argv 0];   
}

#
# main
#
set seconds [expr [clock seconds] - 24*3600];
set yesterday [clock format $seconds -gmt true -format $option(f)];

set invfilelist [glob -directory $g(invdir) -nocomplain -tails "*"];
if {[llength $invfilelist] == 0} {
    return;
}

foreach invfile $invfilelist {
    if {[string compare $invfile $yesterday] == 1} {
	continue;
    }

    invfpath [file join  $g(invdir) $invfile];
    foreach capfpath [split [exec cat $invfpath] "\n"] {
	nbspcapcleanup_delete_file $capfpath;
    }
    nbspcapcleanup_delete_file $invfpath;
}
