#!%TCLSH%
#
# $Id$
# 
# Usage: nbspgismapl [-b] [-d <outputdir>] [-i <id>]
#
# -b => background mode
#
# This tool reads an rc script (defined in gisfilter.{init,conf} and then
# calls nbspgismap with the appropriate options for each configured
# composite bundle.

package require cmdline;

set usage {nbspgismapl [-b] [-d <outputdir>] [-i <id>]};
set optlist {b {d.arg ""} {i.arg ""}};

# Read the init (instead of conf) because the filterlib_find_conf() function
# from filter.lib is used.
set f "/usr/local/libexec/nbsp/filters.init";
if {[file exists ${f}] == 0} {
    log_err "$f not found.";
}
source $f;

# The defaults are read from the gisfilter.init, and the overrides
# from gisflter.conf.
foreach f [list "gisfilter.init"] {
    set f [file join $common(libdir) $f];
    if {[file exists ${f}] == 0} {
        log_err "$f not found.";
    }
    source $f;
}
unset f;

# Convenience definitions
foreach k [array names gisfilter gismap_*] {
    regexp {^gismap_(.+)} $k match _k;
    set nbspgismapl($_k) $gisfilter($k);
}
#
set nbspgismapl(inputdir) $gisfilter(datadir);
set nbspgismapl(outputdir) $gisfilter(datadir);

# variables
set nbspgismapl(geocid) "";
set nbspgismapl(geoclist) [list];
set nbspgismapl(geocidlist) [list];

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

proc get_rcfile {rcname} {

    global nbspgismapl;

    if {$nbspgismapl(rcfile_fpath) eq ""} {
	set nbspgismapl(rcfile_fpath) [filterlib_find_conf \
	    $nbspgismapl(rcfile) $nbspgismapl(rcdirs) $nbspgismapl(rcsubdir)];
    }

    if {[file exists $nbspgismapl(rcfile_fpath)] == 0} {
	log_err "$nbspgismapl(rcfile) not found.";
    }

    source $nbspgismapl(rcfile_fpath);
}

proc get_map_tmplfile {map_tmplname} {

    global nbspgismapl;

    set map_tmplfile [filterlib_find_conf $map_tmplname \
	$nbspgismapl(mapdirs) $nbspgismapl(mapsubdir)];

    if {[file exists $map_tmplfile] == 0} {
	log_err "$map_tmplname not found.";
    }

    return $map_tmplfile;
}

proc get_geodata_dir {} {

    global nbspgismapl;

    if {$nbspgismapl(geodata_dir) eq ""} {
	set nbspgismapl(geodata_dir) [filterlib_find_conf \
	    $nbspgismapl(geodata_dirname) $nbspgismapl(geodata_dirs)];
    }

    if {[file isdirectory $nbspgismapl(geodata_dir)] == 0} {
	log_err "$nbspgismapl(geodata_dirname) not found.";
    }
}

proc get_gclist {} {

    global nbspgismapl;

    if {[llength $nbspgismapl(geoclist)] == 0} {
	log_err "geoclist is empty.";
    }

    foreach gc $nbspgismapl(geoclist) {
	array set a $geoc;
	foreach key [array names a] {
	    set nbspgismapl(geoclist,$a(id),$key) $a($key);
	}
	lappend nbspgismapl(geocidlist) $a(id);
    }
    
    # foreach k [array names nbspgismapl "geoclist,*"] {
    #     puts "$k: $nbspgismapl($k)";
    # }
}

proc process_geoc_entry {id} {

    global option nbspgismapl;

    # get_geoclist {} fills these, for each id, and these, together
    # with the common options, are passed to nbspgismap:
    #
    # nbspgismapl(geoclist,$id,maptmpl)
    # nbspgismapl(geoclist,$id,outputfile)
    # nbspgismapl(geoclist,$id,inputpatt)
    # nbspgismapl(geoclist,$id,inputdirs)

    set map_tmplfile [get_map_tmplfile $nbspgismapl(geoclist,$id,maptmpl)];

    set cmd [list "nbspgismap"];
    if {$option(b) == 1} {
	lappend cmd "-b";
    }

    if {[file isdirectory $nbspgismapl(outputdir)] == 0} {
	log_err "$nbspgismapl(outputdir) does not exist.";
    }

    file mkdir [file join $nbspgismapl(outputdir) \
		    [file dirname $nbspgismapl(geoclist,$id,outputfile)]];
    
    set cmd [concat $cmd \
		 [list -d $nbspgismapl(outputdir) \
		      -g $nbspgismapl(geodata_dir) \
		      -m $map_tmplfile \
		      -o $nbspgismapl(geoclist,$id,outputfile) \
		      -I $nbspgismapl(inputdir) \
		      -p $nbspgismapl(geoclist,$id,inputpatt)] \
		 $nbspgismapl(geoclist,$id,inputdirs)];

    exec $cmd;
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$argc != 0} {
    set _idlist $argv;
} else {
    set _idlist $nbspgismapl(geocidlist);
}

if {$option(d) ne ""} {
    set nbspgismapl(outputdir) $option(d);
}

get_geodata_dir;
get_rcfile;
get_gclist;

foreach id $_idlist {
    process_geoc_entry $id;
}
