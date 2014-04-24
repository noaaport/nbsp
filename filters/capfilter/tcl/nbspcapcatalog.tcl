#!%TCLSH%
#
# $Id$
#

set usage {capcatalog [-b] [-g <atomtxml_global>] [-s <atomtxml_state>]
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
caplib_get_rcvars rc $prod_body;
capfilter_write_catalog rc;
