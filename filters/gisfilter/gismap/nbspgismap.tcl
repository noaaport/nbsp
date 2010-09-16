#!%TCLSH%
#
# $Id$
# 
# Usage: nbspgismap [-b] [-c conffile] [-d <outputdir>] [-L] [<id_list>]
#
# -b => background mode
# -L => output the id list
#
# This tool reads a "bundle configuration file"
# (defined in gisfilter.{init,conf} and then calls nbspgismap1 with the
# appropriate options for each configured composite bundle.

package require cmdline;

set usage {nbspgismap [-b] [-c <conffile>] [-d <outputdir>] [-L] [<id_list>]};
set optlist {b {d.arg ""} {c.arg ""} L};

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
set nbspgismap(geoclist) [list];	# initialized dynamically below
set nbspgismap(geocid) "";
set nbspgismap(geocidlist) [list];

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
	array set a $gc;
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
    # nbspgismap(geoclist,$id,options)
    # nbspgismap(geoclist,$id,outputfile)
    # nbspgismap(geoclist,$id,inputpatt)
    # nbspgismap(geoclist,$id,inputdirs)

    set map_tmplfile [get_map_tmplfile $nbspgismap(geoclist,$id,maptmpl)];

    # Construct the definitions list
    set defs_list [list];
    foreach {k v} $nbspgismap(geoclist,$id,options) {
	lappend defs_list "$k=$v";
    }
    set defs [join $defs_list ","];

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
		      -D $defs \
		      -g $nbspgismap(geodata_dir) \
		      -m $map_tmplfile \
		      -o $nbspgismap(geoclist,$id,outputfile) \
		      -I $nbspgismap(inputdir) \
		      -p $nbspgismap(geoclist,$id,inputpatt)] \
		 $nbspgismap(geoclist,$id,inputdirs)];

    eval exec $cmd;
}

#
# These two functions can be used in the bundle conf file instead of using
# `lappend nbspgismap(geoclist)` expicitly.
#
proc geoc_sat_bundle_add {id maptmpl extent size outputfile inputpatt args} {
#
# The last argument "args" should contain the list of input directories.
# Each argument can be a tcl list of such directories.
#
    set options [list extent $extent size $size];
    lappend nbspgismap(geoclist) [list \
				      id $id \
				      maptmpl $maptmpl \
				      options $options \
				      outputfile $outputfile \
				      inputpatt $inputpatt \
				      inputdirs [eval concat $args]];
}

proc geoc_rad_bundle_add {id maptmpl extent size awips1 \
			      outputfile inputpatt args} {
#
# The last argument "args" should contain the list of input directories.
# Each argument can be a tcl list of such directories.
#
    set options [list extent $extent size $size awips1 $awips1];
    lappend nbspgismap(geoclist) [list \
				      id $id \
				      maptmpl $maptmpl \
				      options $options \
				      outputfile $outputfile \
				      inputpatt $inputpatt \
				      inputdirs [eval concat $args]];
}

proc geoc_bundle_clear {} {

    global nbspgismap;

    set nbspgismap(geoclist) [list];
}

#
# Dynamic initialization
#

# Load the configured geoclist
if {[file exists $nbspgismap(bundle_conf)] == 0} {
    log_err "$nbspgismap(bundle_conf) not found.";
} else {
    source $nbspgismap(bundle_conf);
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$argc != 0} {
    set _idlist $argv;
} else {
    set _idlist [list];
}

if {$option(c) ne ""} {
    set nbspgismap(bundle_conf) $option(c);
}

if {$option(d) ne ""} {
    set nbspgismap(outputdir) $option(d);
}

get_geodata_dir;
get_gclist;

# If no id's were given in the cmd line, then do all of them.
if {[llength $_idlist] == 0} {
   set _idlist $nbspgismap(geocidlist);
}

if {$option(L) == 1} {
    foreach id $_idlist {
	puts $id;
    }
    return;
}

foreach id $_idlist {
    process_geoc_entry $id;
}
