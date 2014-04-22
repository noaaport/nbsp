#!%TCLSH%
#
# $Id$
#

set usage {capbuild [-b] [-c <catdir>] [-i <invdir>]};
set optlist {b {c.arg ""} {i.arg ""}};

# To find the nbsp packages and load the filter library
source "/usr/local/etc/nbsp/filters.conf";
source $common(filterslib);

# Load the capfilter variables
source [file join $common(libdir) "capfilter.init"];

# tcllib packages
package require cmdline;

# Nbsp packages - syslog enabled below if -b is given.
package require nbsp::errx;
package require nbsp::util;

# variables
set g(catdir) $capfilter(catdir);
set g(invdir) $capfilter(invdir);

#
# init
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$option(b) == 1} {
    ::nbsp::syslog::usesyslog
}

if {$argc > 0} {
        ::nbsp::syslog::err $usage;
}

if {$option(c) ne ""} {
    set g(catdir) $option(c);
}

if {$option(i) ne ""} {
    set g(invdir) $option(i);
}

#
# main
#
set invfilelist [lsort [glob -directory $g(invdir) -nocomplain -tails "*"]];
if {[llength $invfilelist] == 0} {
    return;
}

file delete -force $g(catdir);

foreach invfile $invfilelist {
    set invfpath [file join $g(invdir) $invfile];
    foreach capfpath [split [filterlib_file_cat $invfpath] "\n"] {
	set prod_body [filterlib_file_cat $capfpath];

	set status [catch {
	    ::nbsp::util::pwrite_block $prod_body $capfilter(feedbin);
	} errmsg];

	if {$status != 0} {
	    ::nbsp::syslog::errc $errmsg;
	}
    }
}
