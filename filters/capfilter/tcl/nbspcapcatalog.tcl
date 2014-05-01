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

    # The xml files
    set catppath_global $capfilter(catppath,global);
    set catppath_state [subst $capfilter(catppathfmt,state)];

    # The body xml files
    set catbodyppath_global [file rootname $catppath_global];
    append catbodyppath_global $capfilter(catbodyfext);
    set catbodyppath_state [file rootname $catppath_state];
    append catbodyppath_state $capfilter(catbodyfext);

    # The content of the body
    set entry_global [subst $atomtxml(global,entry)];
    set entry_state [subst $atomtxml(state,entry)];
    
    # Append to the body files
    filterlib_file_append $catbodyppath_global $entry_global;
    filterlib_file_append $catbodyppath_state $entry_state;

    # Re-create the xml files
    filterlib_file_write $catppath_global [subst $atomtxml(global,header)];
    filterlib_file_append \
	$catppath_global [::fileutil::cat $catbodyppath_global];
    filterlib_file_append $catppath_global [subst $atomtxml(global,footer)];

    filterlib_file_write $catppath_state [subst $atomtxml(state,header)];
    filterlib_file_append \
	$catppath_state [::fileutil::cat $catbodyppath_state];
    filterlib_file_append $catppath_state [subst $atomtxml(state,footer)];

    foreach zone $rc(cap,zones) {
	set rc(cap,zone) $zone;
	set catppath_zone [subst $capfilter(catppathfmt,zone)];
	set catbodyppath_zone [file rootname $catppath_zone];
	append catbodyppath_zone $capfilter(catbodyfext);
	set entry_zone [subst $atomtxml(zone,entry)];
	filterlib_file_append $catbodyppath_zone $entry_zone;

	filterlib_file_write $catppath_zone [subst $atomtxml(zone,header)];
	filterlib_file_append \
	    $catppath_zone [::fileutil::cat $catbodyppath_zone];
	filterlib_file_append $catppath_zone [subst $atomtxml(zone,footer)];
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
