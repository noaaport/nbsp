#!%TCLSH%
#
# $Id$
# 
# Usage: nbspsatgc [-b] [-d <outputdir>] [-o <outname>] [-p <postrcname>]
#                  [-r] <compositename>
#
# -b => background mode
# -d => output directory
# -o => name of outputfile (otherwise the default is used)
# -p => post rc file
# -r => look for the mapserever rc file in standard dirs
#
package require cmdline;

set usage {nbspsatgc [-b] [-d <outputdir>] [-o <outname>] [-p <postrcname>]
    [-r] <compositename>};
set optlist {b {d.arg ""} {o.arg ""} {p.arg ""} r};

## The common defaults
set defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaultsfile] == 0} {
   log_err "$defaultsfile not found.";
}
source $defaultsfile;

# configuration
set nbspsatgc(localconfdirs) $common(localconfdirs);
set nbspsatgc(rcdirs) $common(localconfdirs);
set nbspsatgc(rcsubdir) [file join "gismap" "sat"];
#
set nbspsatgc(mapserver_fmt) "png";
set nbspsatgc(mapserver_rcname) "mapserver.map";
set nbspsatgc(post_rcname) "post.tcl";
#
set nbspsatgc(shp2img_bin) "shp2img";

# variables
set nbspsatgc(mapserver_rcfile) "";
set nbspsatgc(post_rcfile) "";
#
set nbspsatgc(composite_name) "";
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

    set status [catch {
	exec $nbspsatgc(shp2img_bin) \
	    -m $nbspsatgc(mapserver_rcfile) \
	    -o $nbspsatgc(outputfile);
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
set nbspsatgc(composite_name) [lindex $argv 0];

if {$option(o) ne ""} {
    set nbspsatgc(outputfile) $option(o);
} else {
    set nbspsatgc(outputfile) \
	$nbspsatgc(composite_name).$nbspsatgc(mapserver_fmt);
}

if {$option(d) ne ""} {
    set nbspsatgc(outputfile) [file join $option(d) $nbspsatgc(outputfile)];
}

if {$option(p) ne ""} {
    set nbspsatgc(post_rcname) $option(p);
}

# Find the rcfiles
if {$option(r) == 1} {
    source $common(filterslib);
    set rcsubdir [file join $nbspsatgc(rcsubdir) $nbspsatgc(composite_name)];
    set nbspsatgc(mapserver_rcfile) [filterlib_find_conf \
	$nbspsatgc(mapserver_rcname) $nbspsatgc(rcdirs) $rcsubdir];
    set nbspsatgc(post_rcfile) [filterlib_find_conf \
	$nbspsatgc(post_rcname) $nbspsatgc(rcdirs) $rcsubdir];
} else {
    set nbspsatgc(mapserver_rcfile) $nbspsatgc(mapserver_rcname);
    set nbspsatgc(post_rcfile) $nbspsatgc(post_rcname);
}

# The post_rcfile is optional but the mapserver is required.
if {[file exists $nbspsatgc(mapserver_rcfile)] == 0} {
    log_err "$nbspsatgc(mapserver_rcname) not found.";
}

exec_shp2img;
exec_post $nbspsatgc(post_rcfile) $nbspsatgc(outputfile);
