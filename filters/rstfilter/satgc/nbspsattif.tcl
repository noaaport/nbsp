#!%TCLSH%
#
# $Id$
# 
# Usage: nbspsattif [-b] [-d outputdir] [-K] [-o outputname] 
#                   [-p <postrcname>] [-r <subdir>] [-w <wct_bin>]
#                   <inputfile> <wctconffile>;
#
# -b => background mode
# -d => output dir
# -K => delete the .wct-cache dir
# -o => prefix name of the output file (e.g., tige04)
# -p => postrc filename
# -r => look for wct config file and post rc file in <subdir> within the
#       standard dirs
# -w => path to wct-export
#
package require cmdline;

set usage {nbspsattif [-b] [-d <outputdir>] [-K] [-o <outname>]
    [-p <postrc> [-r <subdir>] [-w <wctbin>] <inputfile> [<wctconffile>]};
    set optlist {b {d.arg ""} K {o.arg ""} {p.arg ""} {r.arg ""} {w.arg ""}};

## The common defaults
set defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaultsfile] == 0} {
   log_err "$defaultsfile not found.";
}
source $defaultsfile;

# Convenience definitions
set nbspsattif(localconfdirs) $common(localconfdirs);
set nbspsattif(rcdirs) $common(localconfdirs);

# defaults
set nbspsattif(wct_bin) "wct-export";

# parameters
set nbspsattif(wct_fmt) "tif";
set nbspsattif(wct_fext) ".tif";
set nbspsattif(wct_fmeta) "-var-1-8bit";
set nbspsattif(wct_cachedir) [file join $env(HOME) ".wct-cache"];

# variables
set nbspsattif(wct_rcname) "";
set nbspsattif(wct_rcfile) "";
set nbspsattif(post_rcname) "";
set nbspsattif(post_rcfile) "";
#
set nbspsattif(inputfile) "";
set nbspsattif(outputfile) "";

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

proc exec_wct {} {

    global nbspsattif;

    set status [catch {
		exec $nbspsattif(wct_bin) \
		    $nbspsattif(inputfile) \
		    $nbspsattif(outputfile) \
		    $nbspsattif(wct_fmt) \
		    $nbspsattif(wct_rcfile); 
    } errmsg];

    # This is the actual name that wct uses in the output file
    append wct_name [file rootname $nbspsattif(outputfile)] \
	$nbspsattif(wct_fmeta) $nbspsattif(wct_fext);

    if {[file exists $wct_name] == 0} {
	log_err $errmsg;
    } else {
	file rename -force $wct_name $nbspsattif(outputfile);
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

if {$argc != 2} {
    log_err $usage;
}
set nbspsattif(inputfile) [lindex $argv 0];
set nbspsattif(wct_rcname) [lindex $argv 1];

if {$option(p) ne ""} {
    set nbspsattif(post_rcname) $option(p);
}

if {$option(w) ne ""} {
    set nbspsattif(wct_bin) $option(w);
}

if {$option(o) ne ""} {
    set nbspsattif(outputfile) $option(o);
} else {
    set nbspsattif(outputfile) [file rootname $nbspsattif(inputfile)];
}

if {[file extension $nbspsattif(outputfile)] ne $nbspsattif(wct_fext)} {
    append nbspsattif(outputfile) $nbspsattif(wct_fext);
}

if {$option(d) ne ""} {
    set nbspsattif(outputfile) [file join $option(d) \
				    [file tail $nbspsattif(outputfile)]];
}

# Find the wct and post rcfile

if {$option(r) ne ""} {
    source $common(filterslib);

    set nbspsattif(wct_rcfile) [filterlib_find_conf \
	$nbspsattif(wct_rcname) $nbspsattif(rcdirs) $option(r)];

    set nbspsattif(post_rcfile) [filterlib_find_conf \
	$nbspsattif(post_rcname) $nbspsattif(rcdirs) $option(r)];
} else {
    set nbspsattif(wct_rcfile) $nbspsattif(wct_rcname);
    set nbspsattif(post_rcfile) $nbspsattif(post_rcname);
}

# The post_rcfile is optional but the wct rcfile is required.
if {[file exists $nbspsattif(wct_rcfile)] == 0} {
    log_err "$nbspsattif(wct_rcname) not found.";
}

exec_wct;
exec_post $nbspsattif(post_rcfile) $nbspsattif(outputfile);

if {($option(K) == 1) && [file isdirectory $nbspsattif(wct_cachedir)]} {
    file delete -force $nbspsattif(wct_cachedir);
}
