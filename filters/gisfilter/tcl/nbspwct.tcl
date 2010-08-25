#!%TCLSH%
#
# $Id$
# 
# Usage: nbspwct [-b] [-d <outputdir>] [-e <fext>] [-f <fmt>] [-K]
#                   [l <latestname>] [-o <outputfile>] [-p <postrcfile>]
#                   [-w <wct_bin>] -x <wctconffile> <inputfile>
#
# This is a cmdline tool (really a wrapper for WCT) with no configuration file.
#
# -b => background mode
# -d => output dir
# -f => format of the output file
# -K => delete the .wct-cache dir
# -l => create a link to the file
# -o => rootname of the outpufile (otherwise default is constructed)
# -p => postrc filename
# -V => debug wct errors
# -w => path to wct-export
# -x => the wct xml config file (required)

package require cmdline;

set usage {nbspwct [-b] [-d <outputdir>] [-f <fmt>]
    [-K] [-l <latestname>] [-o <outputfile_rootname>]
    [-p <postrc>] [-V] [-w <wctbin>] -x <wctconfigfile> <inputfile>};
set optlist {b {d.arg ""} {f.arg ""} K {l.arg ""} {o.arg ""} {p.arg ""}
    V {w.arg ""} {x.arg ""}};

# defaults
set nbspwct(wct_bin) "wct-export";

# parameters
set nbspwct(wct_cachedir) [file join $env(HOME) ".wct-cache"];
set nbspwct(wct_fmt) "tif";	# default
#
set nbspwct(wct_fmeta,tif) "-var-1-8bit";
set nbspwct(wct_fmeta,nc) "-var-1";
set nbspwct(wct_fmeta,shp) "-var-1";
set nbspwct(wct_fmeta,asc) "-var-1";
set nbspwct(wct_fmeta,wkt) "-var-1";
# extensions of the outputfile(s)
set nbspwct(wct_fext,tif) [list ".tif"];
set nbspwct(wct_fext,nc) [list ".nc"];
set nbspwct(wct_fext,shp) [list ".shp" ".shx" ".dbf" ".prj"];
set nbspwct(wct_fext,asc) [list ".asc" ".prj"];
set nbspwct(wct_fext,wkt) [list ".wkt.txt" ".wkt.prj"];

# variables
set nbspwct(debug) 0;
set nbspwct(wct_rcfile) "";
set nbspwct(post_rcfile) "";
#
set nbspwct(inputfile) "";
set nbspwct(outputfile_rootname) "";
set nbspwct(outputfile) "";	# main file  (for the postrc)
set nbspwct(latestname) "";
#
set nbspwct(wct_fmeta) "";	# set according to fmt
set nbspwct(wct_fext) [list];	# set according to fmt

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
    global errorInfo;

    set status [catch {
		exec $nbspwct(wct_bin) \
		    $nbspwct(inputfile) \
		    $nbspwct(outputfile_rootname) \
		    $nbspwct(wct_fmt) \
		    $nbspwct(wct_rcfile); 
    } errmsg];

    foreach fext $nbspwct(wct_fext) {
	# This is the actual name that wct uses in the output file(s)
	append wct_name $nbspwct(outputfile_rootname) \
	    $nbspwct(wct_fmeta) $fext;

	set outputfile $nbspwct(outputfile_rootname);
	append outputfile $fext;

	if {[file exists $wct_name] == 0} {
	    log_err $errmsg;
	    if {$nbspwct(debug) != 0} {
		log_err $errorInfo;
	    }
	} else {
	    file rename -force $wct_name $outputfile;
	}
    }

    # The main outputfile
    append nbspwct(outputfile) \
	$nbspwct(outputfile_rootname) [lindex $nbspwct(wct_fext) 0];

    if {$nbspwct(latestname) ne ""} {
	set savedir [file dirname $nbspwct(outputfile)];
	set savename [file tail $nbspwct(outputfile)];
	make_latest $savedir $savename $nbspwct(latestname);
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

proc make_latest {savedir savename latestname} {
#
# Create a "latest" link to the file. This functionality is put here rather
# than the filter so that this program can be executed in the background
# and the the link created after WCT finishes.
#
    set currentdir [pwd];
    cd $savedir;

    set latest $savename;
    set linkpath $latestname;
    if {[file exists $latest] == 0} {
 	cd $currentdir;
	return;
    }

    set status [catch {
        file delete $linkpath;
        # file link -symbolic $linkpath $latest;
	exec ln -s $latest $linkpath;
    } errmsg];

    cd $currentdir;
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

if {$option(f) ne ""} {
    set nbspwct(wct_fmt) $option(f);
}
set nbspwct(wct_fmeta) $nbspwct(wct_fmeta,$nbspwct(wct_fmt));
set nbspwct(wct_fext) $nbspwct(wct_fext,$nbspwct(wct_fmt));  # list of fext

if {$option(l) ne ""} {
    set nbspwct(latestname) $option(l);
}

if {$option(p) ne ""} {
    set nbspwct(post_rcfile) $option(p);
}

if {$option(V) != 0} {
    set nbspwct(debug) 1;
}

if {$option(w) ne ""} {
    set nbspwct(wct_bin) $option(w);
}

if {$option(o) ne ""} {
    set nbspwct(outputfile_rootname) $option(o);
    if {$option(d) ne ""} {
	set nbspwct(outputfile_rootname) [file join $option(d) $option(o)];
    }
} else {
    set nbspwct(outputfile_rootname) \
	[file rootname [file tail $nbspwct(inputfile)]];
    if {$option(d) ne ""} {
	set nbspwct(outputfile_rootname) \
	    [file join $option(d) $nbspwct(outputfile_rootname)];
    }
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
