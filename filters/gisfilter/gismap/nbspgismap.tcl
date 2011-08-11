#!%TCLSH%
#
# $Id$
# 
# Usage: nbspgismap [-b] [-d <outputdir>] [-D <defines>]
#                  [-e <extent>] [-f <mapfonts_dir>] [-g <geodata_dir>]
#                  [-I <inputdir>] -m <map_template> [-n <index>]
#                  [-o <outputfile>] [-p <patt>] [-s <size>] [-t <imgtype>]
#                  <file1> ... <filenn>
#
# The <filen> arguments can be given in the command line or in stdin.
#
############################################################################
# Example
#
# !/bin/sh
#
# outputdir="sat/img/tig"
# outputfile="tig01.png"
# inputdir="/var/noaaport/data/gis/sat/shp/tig"
#
## cd /var/noaaport/data/gis
# mkdir -p $outputdir
#
# ./nbspgismap -d $outputdir -o $outputfile \
#    -f mapfonts -g geodata -m map_sat.tmpl \
#    -D extent=a;b;s;d,size=1200;800,imagetype=png \
#    -I $inputdir -p "*.shp" tigw01 tige01
############################################################################
#
# This is cmdline tool with no configuration file.
#
# -I => parent directory for the arguments to the program.
# -p => the arguments are interpreted as subdirectories of the -I parent dir.
#       Then the list of files is constructed using the glob <patt>, sorted
#       in decreasing order, and the -n <index> option is used to select
#       the file. The default is the most recent file (index = 0).
# -b => background mode
# -d => output directory
# -D => key=value,... comma separated list of map(key)=var pairs
#       (in practice, extent=...,size=...
#	The extent size values can be separated by spaces or ";", for example
#	-D size="sx sy",extent="a b c d" or -D size=sx;sy,extent=a;b;c;d
# -e => extent
# -f => map fonts directory (required)
# -g => geodata directory (required)
# -m => map template      (required)
# -o => name of outputfile (otherwise the default is used)
# -s => size
# -t => imgtype (png)

set usage {nbspgismap [-b] [-d <outputdir>] [-D <defines>]
    [-e <extent>] [-f <mapfontsdir>] [-g <geodatadir>] [-I <inputdir>]
    [-m <map_template>] [-n <index>] [-o <outputfile>] [-p <patt>]
    [-s <size>] [-t <imgtype>] <file1> ... <filen>};

set optlist {b {d.arg ""} {D.arg ""} {e.arg ""} {f.arg ""} {g.arg ""}
    {I.arg ""} {m.arg ""} {n.arg 0} {o.arg ""} {p.arg ""} {s.arg ""}
    {t.arg ""}};

package require cmdline;

# Source filters.init so that the templates can "require" locally
# installed packages (e.g., map_rad requitres gis.tcl)
#
source "/usr/local/libexec/nbsp/filters.init";

# Defaults
set map(imagetype) "png";
#
# map(extent) will be required below
# map(geodata) will be required below
# ma[(mapfonts) will be required below
# map(size) is optional (a default is set in the templates)
#

# parameters
set nbspgismap(map_rc_fext) ".map";
set nbspgismap(map_tmpl_fext) ".tmpl";

# variables
set nbspgismap(map_tmplfile) "";
set nbspgismap(map_rcfile) "";
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

    global map nbspgismap option;

    # The map rc file is created in the same directory as the output file,
    # and with the same name but with the ".map" extension.

    set nbspgismap(map_rcfile) [file rootname $nbspgismap(outputfile)];
    append nbspgismap(map_rcfile) $nbspgismap(map_rc_fext);

    # Create the variables for the map script. For the same reason
    # mentioned in exec_shp2img {}, use the full paths in the map rc file.
    # The output file is not used by the map script (only by the postscript)
    # but for uniformity we use the full path here as well.

    set map(mapfonts) [file join [pwd] $map(mapfonts)];
    set map(geodata) [file join [pwd] $map(geodata)];
    set map(outputfile) [file join [pwd] $nbspgismap(outputfile)];

    set i 1;
    foreach inputfile $nbspgismap(input_files_list) {
	set map(inputfile,$i) [file join [pwd] $inputfile];
	incr i;
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

    if {$status != 0} {
	log_warn $errmsg;
    }

    if {[info exists F]} {
	if {[catch {close $F} errmsg] != 0} {
	    set status 1;
	    log_warn $errmsg;
	}
    }

    if {$status != 0} {
	file delete $nbspgismap(map_rcfile);
	log_err "Could not create map script $nbspgismap(map_rcfile)";
    }

    exec_shp2img;

    if {[info exists map(post)]} {
	eval $map(post);
    }
}

proc exec_shp2img {} {

    global nbspgismap;

    # The partial file names in the mapserver script are interpreted
    # relative to the location of the map script and the data
    # files are relative to the SHAPEPATH variable.
    # I am not ure, but apparently shp2img make a `cd` to
    # the directory that has the map file. If nbspgismap(outputfile)
    # is a partial path, it will be created there (or throw an error
    # if the intermediate directories do not exist).
    # As a workaround to this mess, we pass to shp2img the full path,
    # and always use full paths in the map scripts.

    set outputfile [file join [pwd] $nbspgismap(outputfile)];
    set outputlock ${outputfile}.lock.[pid];

    # Delete any older version
    file delete $outputfile;

    set status [catch {
	exec shp2img \
	    -m $nbspgismap(map_rcfile) \
	    -o $outputlock;
    } errmsg];

    file delete $nbspgismap(map_rcfile);

    if {[file exists $outputlock]} {
	file rename -force $outputlock $outputfile;
    }

    if {[file exists $nbspgismap(outputfile)] == 0} {
	log_err $errmsg;
    }
}

proc get_tmpl_fname {} {
#
# If the output file name is not specified, then the name is derived from
# the template name.
#
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
	if {[file isdirectory $dir] == 0} {
	    log_warn "Skiping $dir; not found.";
	    continue;
	}
	# Exclude the latest links in the glob
	set flist [list];
	foreach f [lsort -decreasing \
		       [glob -nocomplain -directory $dir $option(p)]] {
	    if {[file type $f] ne "link"} {
		lappend flist $f;
	    }
	}

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

if {$argc == 0} {
    set argv [split [string trim [read stdin]]];
}
get_input_files_list $argv;

if {$option(e) ne ""} {
    set map(extent) $option(e);
}

if {$option(f) ne ""} {
    set map(mapfonts) $option(f);
}

if {$option(g) ne ""} {
    set map(geodata) $option(g);
}

if {$option(m) eq ""} {
    log_err "-m is required.";
} else {
    set nbspgismap(map_tmplfile) $option(m);
}

if {$option(s) ne ""} {
    set map(size) $option(s);
}

if {$option(t) ne ""} {
    set map(imagetype) $option(t);
}

# The defines
if {$option(D) ne ""} {
    set Dlist [split $option(D) ","];
    foreach pair $Dlist {
	set p [split $pair "="];
	set var [lindex $p 0];
	set val [lindex $p 1];
	set map($var) "$val";    # extent and size can contain spaces 
    }
}

foreach k [list extent geodata mapfonts] {
    if {[info exists map($k)] == 0} {
	log_err "No $k specified.";
    }
}

if {$option(o) ne ""} {
    set nbspgismap(outputfile) $option(o);
} else {
    if {[llength $nbspgismap(input_files_list)] == 1} {
	set rootname [file rootname \
		     [file tail [lindex $nbspgismap(input_files_list) 0]]];
    } else {
	set rootname [get_tmpl_fname];
    }
    set nbspgismap(outputfile) ${rootname}.$map(imagetype);
}

if {$option(d) ne ""} {
    set nbspgismap(outputfile) [file join $option(d) $nbspgismap(outputfile)];
}

# The mapserver template is required.
if {[file exists $nbspgismap(map_tmplfile)] == 0} {
    log_err "$nbspgismap(map_tmplfile) not found.";
}

run_map_rcfile;
