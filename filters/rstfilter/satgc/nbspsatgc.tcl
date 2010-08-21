#!%TCLSH%
#
# $Id$
# 
# Usage: nbspsatgc [-b] [-d <outputdir>] [-e fext]
#                  -g <geodata_dir> -m <map_tmpl_name>
#                  [-o <outname>] [-r <subdir>] [-s <shp2img>]
#                  <tif1> ... <tifn>
#
# -b => background mode
# -d => output directory
# -e => default fext
# -g => geodata directory (required)
# -m => map template      (required)
# -o => name of outputfile (otherwise the default is used)
# -r => look for the mapsereverand post  rc files in subdir within
#       the standard dirs
# -s => shp2img binary

package require cmdline;

set usage {nbspsatgc [-b] [-d <outputdir>] [-e <fext>] [-o <outname>]
    [-p <postrcname>] [-r] [-s <shp2img>] <map_template_name>};
set optlist {b {d.arg ""} {e.arg ""} {g.arg ""} {m.arg ""}
{o.arg ""} r {s.arg ""}};

## The common defaults
set defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaultsfile] == 0} {
   log_err "$defaultsfile not found.";
}
source $defaultsfile;

# Convenience definitions
set nbspsatgc(localconfdirs) $common(localconfdirs);
set nbspsatgc(rcdirs) $common(localconfdirs);

#defaults
set nbspsatgc(shp2img_bin) "shp2img";

# parameters
set nbspsatgc(map_output_fext) ".png";
set nbspsatgc(map_rc_fext) ".map";
set nbspsatgc(map_tmpl_fext) ".tmpl";

# variables
set nbspsatgc(map_tmplname) "";
set nbspsatgc(map_tmplfile) "";
set nbspsatgc(map_rcname) "";
set nbspsatgc(map_rcfile) "";
set nbspsatgc(geodata_dir) "";
set nbspsatgc(input_files_list) [list];
set nbspsatgc(outputfile) "";

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

proc run_map_rcfile {} {

    global nbspsatgc;

    # Make the rc file name
    set fname [get_tmpl_fname];
    append nbspsatgc(map_rcname) $fname $nbspsatgc(map_rc_fext);

    # The map rc file is created in the same directory as the
    # output file.
    set dir [file dirname $nbspsatgc(outputfile)];
    set nbspsatgc(map_rcfile) [file join $dir $nbspsatgc(map_rcname)];

    source $nbspsatgc(map_tmplfile);

    # Create the variables for the map script. For the same reason
    # mentioned in exec_shp2img {}, use the full paths in the map rc file.
    # The output file is not used by the map script (only by the psotscript)
    # but for uniformity we use the full path as well.

    set map(geodata) [file join [pwd] $nbspsatgc(geodata_dir)];
    set map(outputfile) [file join [pwd] $nbspsatgc(outputfile)];

    set i 1;
    foreach inputfile $nbspsatgc(input_files_list) {
	set map(inputfile,$i) [file join [pwd] $inputfile];
	incr i;
    }
	
    set status [catch {
	set F [open $nbspsatgc(map_rcfile) "w"];
	fconfigure $F -translation binary -encoding binary;
	puts $F [subst $map(script)];
    } errmsg];

    if {[info exists F]} {
	close $F;
    }

    if {$status != 0} {
	file delete $nbspsatgc(map_rcfile);
	log_err $errmsg;
    }

    exec_shp2img;

    if {[info exists map(post)]} {
	eval $map(post);
    }
}

proc exec_shp2img {} {

    global nbspsatgc;

    # Apparently shp2img make a `cd` to the directory that has the map file.
    # If nbspsatgc(outputfile) is a partial path, it will be created
    # there (or throw an error if the intermediate directories do not exist).
    # As a workaround, pass to shp2img the full path.

    set outputfile [file join [pwd] $nbspsatgc(outputfile)];

    set status [catch {
	exec $nbspsatgc(shp2img_bin) \
	    -m $nbspsatgc(map_rcfile) \
	    -o $outputfile;
    } errmsg];

    file delete $nbspsatgc(map_rcfile);

    if {[file exists $nbspsatgc(outputfile)] == 0} {
	log_err $errmsg;
    }
}

proc get_tmpl_fname {} {

    global nbspsatgc;

    if {[file extension $nbspsatgc(map_tmplname)] \
	    eq $nbspsatgc(map_tmpl_fext)} {
	set fname [file rootname [file tail $nbspsatgc(map_tmplname)]];
    } else {
	set fname [file tail $nbspsatgc(map_tmplname)];
    }

    return $fname;
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$argc < 2} {
    log_err $usage;
}
set nbspsatgc(input_files_list) $argv;

if {($option(g) eq "") || ($option(m) eq "")} {
    log_err "-g and -m are required.";
}
set nbspsatgc(geodata_dir) $option(g);
set nbspsatgc(map_tmplname) $option(m);

if {$option(e) ne ""} {
    set nbspsatgc(map_output_fext) $option(e);
}

if {$option(o) ne ""} {
    set nbspsatgc(outputfile) $option(o);
} else {
    set fname [get_tmpl_fname];
    append nbspsatgc(outputfile) $fname $nbspsatgc(map_output_fext);
}

if {$option(d) ne ""} {
    set nbspsatgc(outputfile) [file join $option(d) \
				   [file tail $nbspsatgc(outputfile)]];
}

# Find the template
if {$option(r) ne ""} {
    source $common(filterslib);
    set nbspsatgc(map_tmplfile) [filterlib_find_conf \
	$nbspsatgc(map_tmplname) $nbspsatgc(rcdirs) $option(r)];
} else {
    set nbspsatgc(map_tmplfile) $nbspsatgc(map_tmplname);
}

# The mapserver template is required.
if {[file exists $nbspsatgc(map_tmplfile)] == 0} {
    log_err "$nbspsatgc(map_tmplname) not found.";
}

run_map_rcfile;
