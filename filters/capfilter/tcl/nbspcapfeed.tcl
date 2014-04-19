#!%TCLSH%
#
# $Id$
#

set usage {capfeed [-b] [-g <atomtxml_global>] [-s <atomtxml_state>]
    [-z <atomtxml_zone>]};
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
    set entry_global [subst $atomtxml(global)];
    set entry_state [subst $atomtxml(state)];
    filterlib_file_append $rc(cap,catpath,global) $entry_global;
    filterlib_file_append $rc(cap,catppath,state) $entry_state;

    foreach zone $rc(cap,zones) {
	set rc(cap,zone) $zone;
	set rc(cap,catppath,zone) [subst $capfilter(catppathfmt,zone)];
	set entry_zone [subst $atomtxml(zone)];
	filterlib_file_append $rc(cap,catppath,zone) $entry_zone;
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

if {$argc != 0} {
        ::nbsp::syslog::err $usage;
}

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

set rc(cap,key,pil) [caplib_get_pil $prod_body];
set rc(cap,key,awips) [string range $rc(cap,key,pil) 3 end];
set rc(cap,key,awips1) [string range $rc(cap,key,awips) 0 2];
set rc(cap,key,awips2) [string range $rc(cap,key,awips) 3 end];

# Get the city/state from capfilter(site,<awips2>) 
foreach {city state} [split $capfilter(site,$rc(cap,key,awips2)) ","] {};
set rc(cap,city) $city;
set rc(cap,state) $state;

foreach key $capfilter(capkeylist) {
    set r [caplib_get_key $key $prod_body];
    set rc(cap,key,$key) $r;
}

set rc(cap,zones) [caplib_get_zone_list $prod_body_list];    # a tcl list

# split expires into date-time
foreach {d t} [split $rc(cap,key,expires) "T"] {};
set rc(cap,key,expires,date) $d;
set rc(cap,key,expires,time) $t;

capfilter_write_catalog rc;
