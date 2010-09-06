#!%TCLSH%
#
# $Id$
# 
# Usage: nbspgismap [-b] [-d <outputdir>] [-i <id>]
#
# -b => background mode
#
# This tool reads a "bundle configuration file"
# (defined in gisfilter.{init,conf} and then calls nbspgismap1 with the
# appropriate options for each configured composite bundle.

package require cmdline;

set usage {nbspgismap [-b] [-c <conffile>] [-d <outputdir>] [-i <id>]};
set optlist {b {d.arg ""} {c.arg ""} {i.arg ""}};

# Read the init (instead of conf) because the filterlib_find_conf() function
# from filter.lib is used.
set f "/usr/local/libexec/nbsp/filters.init";
if {[file exists ${f}] == 0} {
    puts "$f not found.";
    return 1;
}
source $f;

# The defaults are read from the gisfilter.init1, and the overrides
# from gisflter.conf.
foreach f [list "gisfilter.init1"] {
    set f [file join $common(libdir) $f];
    if {[file exists ${f}] == 0} {
        puts "$f not found.";
	return 1;
    }
    source $f;
}
unset f;

# Convenience definitions
foreach k [array names gisfilter gismap_*] {
    regexp {^gismap_(.+)} $k match _k;
    set nbspgismap($_k) $gisfilter($k);
}
#
set nbspgismap(inputdir) $nbspgismap(datadir);
set nbspgismap(outputdir) $nbspgismap(datadir);

# variables
set nbspgismap(geocid) "";
set nbspgismap(geoclist) [list];
set nbspgismap(geocidlist) [list];

# Load the configured geoclist
if {[file exists $nbspgismap(bundle_conf)] == 0} {
    log_err "$nbspgismap(bundle_conf) not found.";
} else {
    source $nbspgismap(bundle_conf);
}

proc log_warn s {

    global argv0;
    global option;

    set name [file tail $argv0];
    if {$option(b) == 0} {
        puts "$name: $s";
    } else {
        exec logger -t $name $s;
    }
}

proc log_err s {

    log_warn $s;
    exit 1;
}

proc get_map_tmplfile {map_tmplname} {

    global nbspgismap;

    set map_tmplfile [filterlib_find_conf $map_tmplname \
	$nbspgismap(mapdirs) $nbspgismap(mapsubdir)];

    if {[file exists $map_tmplfile] == 0} {
	log_err "$map_tmplname not found.";
    }

    return $map_tmplfile;
}

proc get_geodata_dir {} {

    global nbspgismap;

    if {$nbspgismap(geodata_dir) eq ""} {
	set nbspgismap(geodata_dir) [filterlib_find_conf \
	    $nbspgismap(geodata_dirname) $nbspgismap(geodata_dirs)];
    }

    if {[file isdirectory $nbspgismap(geodata_dir)] == 0} {
	log_err "$nbspgismap(geodata_dirname) not found.";
    }
}

proc get_gclist {} {

    global nbspgismap;

    if {[llength $nbspgismap(geoclist)] == 0} {
	log_err "geoclist is empty.";
    }

    foreach gc $nbspgismap(geoclist) {
	array set a $geoc;
	foreach key [array names a] {
	    set nbspgismap(geoclist,$a(id),$key) $a($key);
	}
	lappend nbspgismap(geocidlist) $a(id);
    }
    
    # foreach k [array names nbspgismap "geoclist,*"] {
    #     puts "$k: $nbspgismap($k)";
    # }
}

proc process_geoc_entry {id} {

    global option nbspgismap;

    # get_geoclist {} fills these, for each id, and these, together
    # with the common options, are passed to nbspgismap:
    #
    # nbspgismap(geoclist,$id,maptmpl)
    # nbspgismap(geoclist,$id,outputfile)
    # nbspgismap(geoclist,$id,inputpatt)
    # nbspgismap(geoclist,$id,inputdirs)

    set map_tmplfile [get_map_tmplfile $nbspgismap(geoclist,$id,maptmpl)];

    set cmd [list "nbspgismap1"];
    if {$option(b) == 1} {
	lappend cmd "-b";
    }

    if {[file isdirectory $nbspgismap(outputdir)] == 0} {
	log_err "$nbspgismap(outputdir) does not exist.";
    }

    file mkdir [file join $nbspgismap(outputdir) \
		    [file dirname $nbspgismap(geoclist,$id,outputfile)]];
    
    set cmd [concat $cmd \
		 [list -d $nbspgismap(outputdir) \
		      -g $nbspgismap(geodata_dir) \
		      -m $map_tmplfile \
		      -o $nbspgismap(geoclist,$id,outputfile) \
		      -I $nbspgismap(inputdir) \
		      -p $nbspgismap(geoclist,$id,inputpatt)] \
		 $nbspgismap(geoclist,$id,inputdirs)];

    exec $cmd;
}

#
# These two functions can be used in the rc file instead of using
# `lappend nbspgismap(geoclist)` expicitly.
#
proc geoc_bundle_add {id maptmpl outputfile inputpatt args} {
#
# The last argument "args" should contain the list of input directories.
#
    lappend nbspgismap(geoclist) [list \
	id $id \
	maptmpl $maptmpl \
	outputfile $outputfile \
	inputpatt $inputpatt \
	inputdirs $args];
}

proc geoc_bundle_clear {} {

    global nbspgismap;

    set nbspgismap(geoclist) [list];
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$argc != 0} {
    set _idlist $argv;
} else {
    set _idlist $nbspgismap(geocidlist);
}

if {$option(c) ne ""} {
    set nbspgismap(bundle_conf) $option(c);
}

if {$option(d) ne ""} {
    set nbspgismap(outputdir) $option(d);
}

get_geodata_dir;
get_gclist;

foreach id $_idlist {
    process_geoc_entry $id;
}
