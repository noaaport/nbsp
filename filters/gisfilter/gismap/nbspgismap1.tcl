#!%TCLSH%
#
# $Id$
# 
# Usage: nbspgismap1 [-b] [-d <outputdir>] [-D <defines>]
#                  [-e fext] -g <geodata_dir>
#                  [-I <inputdir>] -m <map_template> [-n <index>]
#                  [-o <outputfile>] [-p <patt>] [-s <shp2img>]
#                  <tif1> ... <tifn>
#
############################################################################
# Example
#
# !/bin/sh
#
# outputdir="sat/img/tig"
# outputfile="tig01.png"
# inputdir="/var/noaaport/data/gis/sat/tif/tig"
#
## cd /var/noaaport/data/gis
# mkdir -p $outputdir
#
# ./nbspgismap -d $outputdir -o $outputfile -g geodata -m map_sat_conus.tmpl \
#    -D extent=a;b;s;d,size=1200;800 \
#    -I $inputdir -p "*.tif" tigw01 tige01
############################################################################
#
# This is cmdline tool with no configuration file.
#
# -I => parent directory for the arguments <tif1> ...
# -p => the <tif1> arguments are interpreted as directories. Then the list
#       of files is constructed using the glob <patt>, and the -n <index>
#       option is used to select the file. The defailt is the "latest" file.
#       (index = end).
# -b => background mode
# -d => output directory
# -D => key=value,... comma separated list of map(key)=var pairs
#       (in practice, extent=...,size=...
#	The extent size values can be separated by spaces or ";", for example
#	-D size="sx sy",extent="a b c d" or -D size=sx;sy,extent=a;b;c;d
# -e => default fext
# -g => geodata directory (required)
# -m => map template      (required)
# -o => name of outputfile (otherwise the default is used)
# -s => shp2img binary

set usage {nbspgismap1 [-b] [-d <outputdir>] [-D <defines>]
    [-e <fext>] [-I <inputdir>] [-n <index>] [-o <outputfile>]
    [-p <patt>] [-s <shp2img>] <map_template>};
set optlist {b {d.arg ""} {D.arg ""} {e.arg ""} {g.arg ""} {I.arg ""}
    {m.arg ""} {n.arg "end"} {o.arg ""} {p.arg ""} {s.arg ""}};

package require cmdline;

# Source filters.init so that the templates can "require" locally
# installed packages (e.g., map_rad requitres gis.tcl)
#
source "/usr/local/libexec/nbsp/filters.init";

# defaults
set nbspgismap(shp2img_bin) "shp2img";

# parameters
set nbspgismap(map_output_fext) ".png";
set nbspgismap(map_rc_fext) ".map";
set nbspgismap(map_tmpl_fext) ".tmpl";

# variables
set nbspgismap(map_tmplfile) "";
set nbspgismap(map_rcfile) "";
set nbspgismap(geodata_dir) "";
set nbspgismap(input_files_list) [list];
set nbspgismap(outputfile) "";

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

proc source_map_tmpl {map_array_name} {

    global nbspgismap;
    upvar $map_array_name map;

    source $nbspgismap(map_tmplfile);
}

proc run_map_rcfile {} {

    global nbspgismap option;

    # Make the rc file name
    set fname [get_tmpl_fname];
    append map_rcname $fname $nbspgismap(map_rc_fext);

    # The map rc file is created in the same directory as the output file.
    set dir [file dirname $nbspgismap(outputfile)];
    set nbspgismap(map_rcfile) [file join $dir $map_rcname];

    # Create the variables for the map script. For the same reason
    # mentioned in exec_shp2img {}, use the full paths in the map rc file.
    # The output file is not used by the map script (only by the postscript)
    # but for uniformity we use the full path here as well.
    set map(geodata) [file join [pwd] $nbspgismap(geodata_dir)];
    set map(outputfile) [file join [pwd] $nbspgismap(outputfile)];

    set i 1;
    foreach inputfile $nbspgismap(input_files_list) {
	set map(inputfile,$i) [file join [pwd] $inputfile];
	incr i;
    }

    # The defines
    if {$option(D) ne ""} {
	set Dlist [split $option(D) ","];
	foreach pair $Dlist {
	    set p [split $pair "="];
	    set var [lindex $p 0];
	    set val [lindex $p 1];
	    set map($var) "$val";    # extent and size contain spaces 
	}
    }

    # Source the template inside a function to avoid clashes with
    # local variables.
    source_map_tmpl map;
	
    set status [catch {
	set F [open $nbspgismap(map_rcfile) "w"];
	fconfigure $F -translation binary -encoding binary;
	if {[info exists map(script)]} {
	    puts $F [subst $map(script)];
	} else {
	    puts $F $map(scriptstr);
	}
    } errmsg];

    if {[info exists F]} {
	close $F;
    }

    if {$status != 0} {
	file delete $nbspgismap(map_rcfile);
	log_err $errmsg;
    }

    exec_shp2img;

    if {[info exists map(post)]} {
	eval $map(post);
    }
}

proc exec_shp2img {} {

    global nbspgismap;

    # Apparently shp2img make a `cd` to the directory that has the map file.
    # If nbspgismap(outputfile) is a partial path, it will be created
    # there (or throw an error if the intermediate directories do not exist).
    # As a workaround, pass to shp2img the full path.

    set outputfile [file join [pwd] $nbspgismap(outputfile)];

    set status [catch {
	exec $nbspgismap(shp2img_bin) \
	    -m $nbspgismap(map_rcfile) \
	    -o $outputfile;
    } errmsg];

    file delete $nbspgismap(map_rcfile);

    if {[file exists $nbspgismap(outputfile)] == 0} {
	log_err $errmsg;
    }
}

proc get_tmpl_fname {} {

    global nbspgismap;

    if {[file extension $nbspgismap(map_tmplfile)] \
	    eq $nbspgismap(map_tmpl_fext)} {
	set fname [file rootname [file tail $nbspgismap(map_tmplfile)]];
    } else {
	set fname [file tail $nbspgismap(map_tmplfile)];
    }

    return $fname;
}

proc get_input_files_list {argv} {

    global nbspgismap option;

    set nbspgismap(input_files_list) [list];

    set flist [list];
    if {$option(I) ne ""} {
	foreach f $argv {
	    lappend flist [file join $option(I) $f];
	}
    } else {
	set flist $argv;
    }
    
    if {$option(p) eq ""} {
	set nbspgismap(input_files_list) $flist;
	return;
    }

    set dirlist $flist;
    foreach dir $dirlist {
	set flist [glob -nocomplain -directory $dir $option(p)];
	set f [lindex $flist $option(n)];
	if {$f ne ""} {
	    lappend nbspgismap(input_files_list) $f;
	}
    }
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$argc < 2} {
    log_err $usage;
}
get_input_files_list $argv;

if {($option(g) eq "") || ($option(m) eq "")} {
    log_err "-g and -m are required.";
}
set nbspgismap(geodata_dir) $option(g);
set nbspgismap(map_tmplfile) $option(m);

if {$option(e) ne ""} {
    set nbspgismap(map_output_fext) $option(e);
}

if {$option(o) ne ""} {
    set nbspgismap(outputfile) $option(o);
} else {
    set fname [get_tmpl_fname];
    append nbspgismap(outputfile) $fname $nbspgismap(map_output_fext);
}

if {$option(d) ne ""} {
    set nbspgismap(outputfile) [file join $option(d) $nbspgismap(outputfile)];
}

# The mapserver template is required.
if {[file exists $nbspgismap(map_tmplfile)] == 0} {
    log_err "$nbspgismap(map_tmplfile) not found.";
}

run_map_rcfile;
