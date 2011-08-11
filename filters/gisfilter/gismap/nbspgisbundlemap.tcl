#!%TCLSH%
#
# $Id$
# 
# Usage: nbspgisbundlemap [-b] [-c <conffile>] [-d <outputdir>]
#                   [-L] [-n] [-o <outputfile>] [-t] [-v] [-w] [<id_list>]
#
# -b => background mode
# -c => override the default configuration file
# -d => implies -w, but override the gisfilter configuration file output dir
# -L => output the id list
# -n => pick only the basename of the default output file
# -o => override the output file name (useful only if there is one id
#       given in the cmd line argument)
# -t => use time-stamped output file name
# -v => print the name of the output file
# -w => use the output directory specified in the gisfilter configuration file
#
# This tool reads a "bundle configuration file"
# (defined in gisfilter.{init,conf}) and then calls nbspgismap with the
# appropriate options for each configured composite bundle.

package require cmdline;

set usage {nbspgisbundlemap [-b] [-c <conffile>] [-d <outputdir>] [-L]
    [-n] [-o <outputfile>] [-t] [-v] [-w] [<id_list>]};
set optlist {b {d.arg ""} {c.arg ""} L n {o.arg ""} t v w};

# Read the init (instead of conf) because the filterlib_find_conf() function
# from filter.lib is used. This also allows templates to "require" locally
# installed packages; (e.g., gismap-bundle.conf-defaults uses radstations.tcl).
#
source "/usr/local/libexec/nbsp/filters.init";

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
    set nbspgisbundle($_k) $gisfilter($k);
}
#
set nbspgisbundle(inputdir) $nbspgisbundle(datadir);
set nbspgisbundle(outputdir) $nbspgisbundle(datadir);

# variables
set nbspgisbundle(geoclist) [list];	# initialized dynamically below
set nbspgisbundle(geocid) "";
set nbspgisbundle(geocidlist) [list];

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

    global nbspgisbundle;

    # If the argument is a full path, then it is not looked any further
    if {[file pathtype $map_tmplname] eq "absolute"} {
	return $map_tmplname;
    }

    set map_tmplfile [filterlib_find_conf $map_tmplname \
	$nbspgisbundle(mapdirs) $nbspgisbundle(mapsubdir)];

    if {[file exists $map_tmplfile] == 0} {
	log_err "$map_tmplname not found.";
    }

    return $map_tmplfile;
}

proc get_geodata_dir {} {

    global nbspgisbundle;

    if {$nbspgisbundle(geodata_dir) eq ""} {
	set nbspgisbundle(geodata_dir) [filterlib_find_conf \
	  $nbspgisbundle(geodata_dirname) $nbspgisbundle(geodata_dirs)];
    }

    if {[file isdirectory $nbspgisbundle(geodata_dir)] == 0} {
	log_err "$nbspgisbundle(geodata_dirname) not found.";
    }
}

proc get_mapfonts_dir {} {

    global nbspgisbundle;

    if {$nbspgisbundle(mapfonts_dir) eq ""} {
	set nbspgisbundle(mapfonts_dir) [filterlib_find_conf \
	 $nbspgisbundle(mapfonts_dirname) $nbspgisbundle(mapfonts_dirs)];
    }

    if {[file isdirectory $nbspgisbundle(mapfonts_dir)] == 0} {
	log_err "$nbspgisbundle(mapfonts_dirname) not found.";
    }
}

proc get_gclist {} {

    global nbspgisbundle;

    if {[llength $nbspgisbundle(geoclist)] == 0} {
	log_err "geoclist is empty.";
    }

    foreach gc $nbspgisbundle(geoclist) {
	array set a $gc;
	foreach key [array names a] {
	    set nbspgisbundle(geoclist,$a(id),$key) $a($key);
	}
	lappend nbspgisbundle(geocidlist) $a(id);
    }
    
    # foreach k [array names nbspgisbundle "geoclist,*"] {
    #     puts "$k: $nbspgisbundle($k)";
    # }
}

proc process_geoc_entry {id} {

    global option nbspgisbundle;

    # get_geoclist {} fills these, for each id, and these, together
    # with the common options, are passed to nbspgismap:
    #
    # nbspgisbundle(geoclist,$id,maptmpl)
    # nbspgisbundle(geoclist,$id,options)
    # nbspgisbundle(geoclist,$id,outputfile)
    # nbspgisbundle(geoclist,$id,outputfilet)
    # nbspgisbundle(geoclist,$id,inputpatt)
    # nbspgisbundle(geoclist,$id,inputdirs)

    set map_tmplfile [get_map_tmplfile $nbspgisbundle(geoclist,$id,maptmpl)];

    # Construct the definitions list
    set defs_list [list];
    foreach {k v} $nbspgisbundle(geoclist,$id,options) {
	lappend defs_list "$k=$v";
    }
    set defs [join $defs_list ","];

    set cmd [list "nbspgismap"];
    if {$option(b) == 1} {
	lappend cmd "-b";
    }

    if {$option(o) ne ""} {
	set nbspgisbundle(geoclist,$id,outputfile) $option(o);
    }

    if {$option(n) != 0} {
	set nbspgisbundle(geoclist,$id,outputfile) \
	    [file tail $nbspgisbundle(geoclist,$id,outputfile)];
	set nbspgisbundle(geoclist,$id,outputfilet) \
	    [file tail $nbspgisbundle(geoclist,$id,outputfilet)];
    }

    if {$option(w) != 0} {
	if {[file isdirectory $nbspgisbundle(outputdir)] == 0} {
	    log_err "$nbspgisbundle(outputdir) does not exist.";
	}
    }

    set outputfile $nbspgisbundle(geoclist,$id,outputfile);
    if {$option(w) != 0} {
	set outputpath [file join $nbspgisbundle(outputdir) $outputfile];
    } else {
	set outputpath $outputfile;
    }
    file mkdir [file dirname $outputpath];

    # Delete the default output file in case it is a link.
    file delete $outputpath;

    # Check if "-t " was specified and set the output name accordingly
    if {$option(t) != 0} {
	set fmt $nbspgisbundle(geoclist,$id,outputfilet);
	set outputfilet [clock format [clock seconds] -format $fmt];
	if {$option(w) != 0} {
	    set outputpatht [file join $nbspgisbundle(outputdir) $outputfilet];
	} else {
	    set outputpatht $outputfilet;
	}
	file mkdir [file dirname $outputpatht];
    }

    if {$option(w) != 0} {
	lappend cmd "-d" $nbspgisbundle(outputdir);
    }

    if {$option(t) != 0} {
	lappend cmd -o $outputfilet;
    } else {
	lappend cmd -o $outputfile;
    }

    set cmd [concat $cmd \
		 [list -D $defs \
		      -f $nbspgisbundle(mapfonts_dir) \
		      -g $nbspgisbundle(geodata_dir) \
		      -m $map_tmplfile \
		      -I $nbspgisbundle(inputdir) \
		      -p $nbspgisbundle(geoclist,$id,inputpatt)] \
		 $nbspgisbundle(geoclist,$id,inputdirs)];

    set status [catch {
	eval exec $cmd;
    } errmsg];

    if {$status != 0} {
	# Don't exit so that other bundles can be processed
	log_warn $errmsg;
    } else {
	if {$option(t) != 0} {
	    exec ln -s [file join [pwd] $outputpatht] $outputpath;
	}

	if {$option(v) != 0} {
	    puts $outputpath;
	}
    }
}

#
# These two functions can be used in the bundle conf file instead of using
# `lappend nbspgisbundle(geoclist)` expicitly.
#
proc geoc_sat_bundle_add {id maptmpl extent size outputfile inputpatt args} {
#
# The last argument "args" should contain the list of input directories.
# Each argument can be a tcl list of such directories.
#
    set options [list extent $extent size $size];
    lappend nbspgisbundle(geoclist) [list \
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
    lappend nbspgisbundle(geoclist) [list \
				      id $id \
				      maptmpl $maptmpl \
				      options $options \
				      outputfile $outputfile \
				      inputpatt $inputpatt \
				      inputdirs [eval concat $args]];
}

proc geoc_bundle_clear {} {

    global nbspgisbundle;

    set nbspgisbundle(geoclist) [list];
}

#
# Dynamic initialization
#

# Load the configured geoclist
if {[file exists $nbspgisbundle(bundle_conf)] == 0} {
    log_err "$nbspgisbundle(bundle_conf) not found.";
} else {
    source $nbspgisbundle(bundle_conf);
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
    set nbspgisbundle(bundle_conf) $option(c);
}

if {$option(d) ne ""} {
    set nbspgisbundle(outputdir) $option(d);
    set option(w) 1;
}

get_mapfonts_dir;
get_geodata_dir;
get_gclist;

# If no id's were given in the cmd line, then do all of them.
if {[llength $_idlist] == 0} {
   set _idlist $nbspgisbundle(geocidlist);
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
