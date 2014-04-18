#!%TCLSH%
#
# $Id$
#

set usage {capcreate [-b] [-g <atomtxml_global>] [-s <atomtxml_state>]
    [-z <atomtxml_zone>] awips2};
set optlist {b {g.arg ""} {s.arg ""} {z.arg ""}};

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
set g(atomtxmlfpath,global) "";
set g(atomtxmlfpath,state) "";
set g(atomtxmlfpath,zone) "";

#
# functions
#
proc capfilter_write_catalog {rc_name} {

    upvar $rc_name rc;
    global capfilter g;

    # Until we know what to do
    # return;

    # Get the templates
    foreach type [list global state zone] {
	source $g(atomtxmlfpath,$type);
    }

    set rc(cap,catpath,global) $capfilter(catppath,global);
    set rc(cap,catppath,state) [subst $capfilter(catppathfmt,state)];
    set entry_global [subst $cap(atomtxml,global)];
    set entry_state [subst $cap(atomtxml,state)];
    filterlib_file_write $rc(cap,catpath,global) $entry_global;
    filterlib_file_write $rc(cap,catppath,state) $entry_state;

    foreach zone $rc(cap,zones) {
	set rc(cap,zone) $zone;
	set rc(cap,catppath,zone) [subst $capfilter(catppathfmt,zone)];
	set entry_zone [subst $cap(atomtxml,zone)];
	filterlib_file_write $rc(cap,catppath,zone) $entry_zone;
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

if {$argc != 1} {
        ::nbsp::syslog::err $usage;
}
set g(awips2) [lindex $argv 0];

# set the default templates
foreach type [list global state zone] {
    # Look for the template in capfilter(txmldirs) and use the last one
    set _txml_fpath [filterlib_find_conf $capfilter(atomtxml,$type) \
			 $capfilter(txmldirs) $capfilter(txmlsubdir)];
    set g(atomtxmlfpath,$type) $_txml_fpath;
}

if {$option(g) ne ""} {
    set g(atomtxmlfpath,global) $option(g);
}

if {$option(s) ne ""} {
    set g(atomtxmlfpath,state) $option(s);
}

if {$option(z) ne ""} {
    set g(atomtxmlfpath,zone) $option(z);
}

foreach type [list global state zone] {
    if {$g(atomtxmlfpath,$type) eq ""} {
	::nbsp::syslog::err "$capfilter(atomtxml,$type) not found.";
    }
}

#
# main
#
set prod_body [read stdin];
set prod_body_list [split $prod_body "\n"];

set rc(cap,identifier) [caplib_get_identifier $prod_body];
set rc(cap,expires) [caplib_get_expires $prod_body];
set rc(cap,summary) [caplib_get_summary $prod_body];
set rc(cap,zones) [caplib_get_zone_list $prod_body_list];    # a tcl list

# get the awips from the cmdline
foreach {city state} [split $capfilter(site,$g(awips2)) ","] {};
set rc(cap,city) $city;
set rc(cap,state) $state;

capfilter_write_catalog rc;
