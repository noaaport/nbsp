#!%TCLSH%
#
# $Id$
# 
# Usage: nbspwct [-b] [-d <outputdir>] [-e <fext>] [-f <fmt>] [-K]
#                   [-o <outputfile>] [-p <postrcfile>]
#                   [-w <wct_bin>] -x <wctconffile> <inputfile>
#
# This is a cmdline tool (really a wrapper for WCT) with no configuration file.
#
# -b => background mode
# -d => output dir
# -e => extension of the output file
# -f => format of the output file
# -K => delete the .wct-cache dir
# -o => outpufile (otherwise default is constructed)
# -p => postrc filename
# -w => path to wct-export
# -x => the wct xml config file (required)

package require cmdline;

set usage {nbspwct [-b] [-d <outputdir>] [-e <fext>] [-f <fmt>]
    [-K] [-o <outputfile>] [-p <postrc>] [-w <wctbin>]
    -x <wctconfigfile> <inputfile>};
set optlist {b {d.arg ""} {e.arg ""} {f.arg ""}
    K {o.arg ""} {p.arg ""} {w.arg ""} {x.arg ""}};

# defaults
set nbspwct(wct_bin) "wct-export";

# parameters
set nbspwct(wct_fmt) "tif";
set nbspwct(wct_fext) ".tif";
set nbspwct(wct_fmeta) "-var-1-8bit";
set nbspwct(wct_cachedir) [file join $env(HOME) ".wct-cache"];

# variables
set nbspwct(wct_rcfile) "";
set nbspwct(post_rcfile) "";
#
set nbspwct(inputfile) "";
set nbspwct(outputfile) "";

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

    global nbspwct;

    set status [catch {
		exec $nbspwct(wct_bin) \
		    $nbspwct(inputfile) \
		    $nbspwct(outputfile) \
		    $nbspwct(wct_fmt) \
		    $nbspwct(wct_rcfile); 
    } errmsg];

    # This is the actual name that wct uses in the output file
    append wct_name [file rootname $nbspwct(outputfile)] \
	$nbspwct(wct_fmeta) $nbspwct(wct_fext);

    if {[file exists $wct_name] == 0} {
	log_err $errmsg;
    } else {
	file rename -force $wct_name $nbspwct(outputfile);
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
set nbspwct(inputfile) [lindex $argv 0];

if {$option(x) eq ""} {
    log_err "-x is required.";
}
set nbspwct(wct_rcfile) $option(x);

if {$option(e) ne ""} {
    set nbspwct(wct_fext) $option(e);
}

if {$option(f) ne ""} {
    set nbspwct(wct_fmt) $option(f);
}

if {$option(p) ne ""} {
    set nbspwct(post_rcfile) $option(p);
}

if {$option(w) ne ""} {
    set nbspwct(wct_bin) $option(w);
}

if {$option(o) ne ""} {
    set nbspwct(outputfile) $option(o);
} else {
    set nbspwct(outputfile) [file rootname $nbspwct(inputfile)];
    append nbspwct(outputfile) $nbspwct(wct_fext);
}

if {$option(d) ne ""} {
    set nbspwct(outputfile) [file join $option(d) \
				    [file tail $nbspwct(outputfile)]];
}

# The post_rcfile is optional but the wct rcfile is required.
if {[file exists $nbspwct(wct_rcfile)] == 0} {
    log_err "$nbspwct(wct_rcfile) not found.";
}

exec_wct;
exec_post $nbspwct(post_rcfile) $nbspwct(outputfile);

if {($option(K) == 1) && [file isdirectory $nbspwct(wct_cachedir)]} {
    file delete -force $nbspwct(wct_cachedir);
}
