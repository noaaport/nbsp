#!%TCLSH%
#
# $Id$
#

set usage {capcreate [-b] [-t <atomtxml>] awips2};
set optlist {b {t.arg ""}};

# To find the nbsp packages and load the filter library
source "/usr/local/etc/nbsp/filters.conf";
source $common(filterslib);

# Load the capfilter variables, functions and definitions
source [file join $common(libdir) "capfilter.init"];
source $capfilter(def);

# tcllib packages
package require cmdline;

# Nbsp packages - syslog enabled below if -b is given.
package require nbsp::errx;

# variables
set g(atomtxmlfpath) "";

#
# functions
#
proc capfilter_write_catalog {rc_name} {

    upvar $rc_name rc;
    global g;

    source $g(atomtxmlfpath);
    set entry [subst $cap(atomtxml)];

    # Until we know what to do
    puts $rc(cap,zone)

    return;

    filterlib_file_write $rc(cap,catpath,global) $entry;
    filterlib_file_write $rc(cap,catppath,bystate) $entry;
    filterlib_file_write $rc(cap,catppath,byzone) $entry;
}

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
set g(awips2) [lindex $argv 0];

if {$option(t) ne ""} {
    set g(atomtxmlfpath) $option(t);
} else {
    # Look for the template in capfilter(txmldirs) and use the last one
    set _txml_fpath [filterlib_find_conf \
	$capfilter(atomtxml) $capfilter(txmldirs) $capfilter(txmlsubdir)];

    if {$_txml_fpath eq ""} {
	::nbsp::syslog::err "$capfilter(atomtxml) not found.";
    }

    set g(atomtxmlfpath) $_txml_fpath;
}

#
# main
#
set prod_body [read stdin];

set rc(cap,identifier) [caplib_get_identifier $prod_body];
set rc(cap,zone) [caplib_get_zone $prod_body];
set rc(cap,expires) [caplib_get_expires $prod_body];
set rc(cap,summary) [caplib_get_summary $prod_body];

# get the awips from the cmdline
foreach {city state} [split $capfilter(site,$g(awips2)) ","] {};
set rc(cap,city) $city;
set rc(cap,state) $state;

set rc(cap,catpath,global) $capfilter(catppath,global);
set rc(cap,catppath,bystate) [subst $capfilter(catppathfmt,bystate)];
set rc(cap,catppath,byzone) [subst $capfilter(catppathfmt,byzone)];

capfilter_write_catalog rc;
