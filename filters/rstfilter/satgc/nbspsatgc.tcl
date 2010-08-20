#!%TCLSH%
#
# $Id$
# 
# Usage: nbspsatgc [-b] [-d <outputdir>] [-o <outname>]
#                  [-p <postrcname>] [-r <subdir>] [-s <shp2img>]
#                  <maprcname>
#
# -b => background mode
# -d => output directory
# -o => name of outputfile (otherwise the default is used)
# -p => post rc file
# -r => look for the mapsereverand post  rc files in subdir within
#       the standard dirs
# -s => shp2img binary

package require cmdline;

set usage {nbspsatgc [-b] [-d <outputdir>] [-o <outname>]
    [-p <postrcname>] [-r] [-s <shp2img>] <maprcname>};
set optlist {b {d.arg ""} {o.arg ""} {p.arg ""} r {s.arg ""}};

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

# variables
set nbspsatgc(map_rcname) "";
set nbspsatgc(map_rcfile) "";
set nbspsatgc(post_rcname) "";
set nbspsatgc(post_rcfile) "";
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

proc exec_shp2img {} {

    global nbspsatgc;

    # Apparently shp2img make a `cd` to the directory that has the map file.
    # If nbspsatgc(outputfile) is a partial path, it will be created
    # there (or throw an error if the intermdiate directories do not exist).
    # As a workaround, pass to shp2img the full path.

    set outputfile [file join [pwd] $nbspsatgc(outputfile)];
    set status [catch {
	exec $nbspsatgc(shp2img_bin) \
	    -m $nbspsatgc(map_rcfile) \
	    -o $outputfile;
    } errmsg];

    if {[file exists $nbspsatgc(outputfile)] == 0} {
	log_err $errmsg;
    }
}

proc exec_post {post_rcfile outputfile} {

    if {[file exists $post_rcfile] == 0} {
	return;
    }

    # Define the post() variables for the postscript
    set post(outputfile) $outputfile;

    source $post_rcfile;
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$argc != 1} {
    log_err $usage;
}
set nbspsatgc(map_rcname) [lindex $argv 0];

if {$option(p) ne ""} {
    set nbspsatgc(post_rcname) $option(p);
}

if {$option(o) ne ""} {
    set nbspsatgc(outputfile) $option(o);
} else {
    if {[file extension $nbspsatgc(map_rcname)] eq $nbspsatgc(map_rc_fext)} {
	set fname [file rootname [file tail $nbspsatgc(map_rcname)]];
    } else {
	set fname [file tail $nbspsatgc(map_rcname)];
    }
    append nbspsatgc(outputfile) $fname $nbspsatgc(map_output_fext);
}

if {$option(d) ne ""} {
    set nbspsatgc(outputfile) [file join $option(d) \
				   [file tail $nbspsatgc(outputfile)]];
}

# Find the rcfiles
if {$option(r) ne ""} {
    source $common(filterslib);
    set nbspsatgc(map_rcfile) [filterlib_find_conf \
	$nbspsatgc(map_rcname) $nbspsatgc(rcdirs) $option(r)];
    set nbspsatgc(post_rcfile) [filterlib_find_conf \
	$nbspsatgc(post_rcname) $nbspsatgc(rcdirs) $option(r)];
} else {
    set nbspsatgc(map_rcfile) $nbspsatgc(map_rcname);
    set nbspsatgc(post_rcfile) $nbspsatgc(post_rcname);
}

# The post_rcfile is optional but the mapserver is required.
if {[file exists $nbspsatgc(map_rcfile)] == 0} {
    log_err "$nbspsatgc(map_rcname) not found.";
}

exec_shp2img;
exec_post $nbspsatgc(post_rcfile) $nbspsatgc(outputfile);
