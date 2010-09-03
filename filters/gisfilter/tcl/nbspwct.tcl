#!%TCLSH%
#
# $Id$
# 
# Usage: nbspwct [-b] [-d <outputdir>] [-f <fmt>] [-K] [-l <latestname>]
#                   [-n] [-o <outputfile>] [-p <postrcfile>] -V
#		    [-t <type>] [-w <wct_bin>] [-x <wctconffile>] <inputfile>
#
# This is a cmdline tool (really a wrapper for WCT). If "-x" is not given,
# then it tries to use "/usr/local/etc/wct-export.xml".
#
# -b => background mode
# -d => output dir
# -f => format of the output file
# -K => delete the .wct-cache dir
# -l => create a link to the file
# -n => do not call wct; only do the post renaming
# -o => outpufile (otherwise default is constructed)
# -p => postrc filename
# -t => type: sat, rad, ... (to determine the fmeta in the name) 
# -V => debug wct errors
# -w => path to wct-export
# -x => the wct xml config file (uses /usr/local/etc/wct-export.xml otherwise)

package require cmdline;

set usage {nbspwct [-b] [-d <outputdir>] [-f <fmt>]
    [-K] [-l <latestname>] [-n] [-o <outputfile_rootname>]
    [-p <postrc>] [-t <type>] [-V] [-w <wctbin>]
    [-x <wctconfigfile>] <inputfile>};
set optlist {b {d.arg ""} {f.arg ""} K {l.arg ""} n {o.arg ""} {p.arg ""}
    {t.arg ""} V {w.arg ""} {x.arg ""}};

# defaults
set nbspwct(wct_bin) "wct-export";
set nbspwct(wct_rcfile) "/usr/local/etc/wct-export.xml";

# parameters
set nbspwct(wct_cachedir) [file join $env(HOME) ".wct-cache"];
set nbspwct(wct_fmt) "tif";	# default
#
set nbspwct(wct_fmeta_sat,tif) "-var-1-8bit";
set nbspwct(wct_fmeta_sat,nc) "-var-1";
set nbspwct(wct_fmeta_sat,asc) "-var-1";
set nbspwct(wct_fmeta_sat,tif32) "-var-1";
set nbspwct(wct_fmeta_sat,rnc) "-var-1";
set nbspwct(wct_fmeta_sat,csv) "-var-1";
set nbspwct(wct_fmeta_sat,shp) "-var-1";
set nbspwct(wct_fmeta_sat,wkt) "-var-1";
# wct extensions of the outputfile(s)
set nbspwct(wct_fext,tif) [list ".tif"];
set nbspwct(wct_fext,nc) [list ".nc"];
set nbspwct(wct_fext,asc) [list ".asc" ".prj"];
set nbspwct(wct_fext,tif32) [list ".tif"];
set nbspwct(wct_fext,rnc) [list ".rnc"];
set nbspwct(wct_fext,csv) [list ".csv" ".prj"];
set nbspwct(wct_fext,shp) [list ".shp" ".shx" ".dbf" ".prj"];
set nbspwct(wct_fext,wkt) [list ".wkt.txt" ".wkt.prj"];

# variables
set nbspwct(debug) 0;
set nbspwct(post_rcfile) "";
#
set nbspwct(inputfile) "";
set nbspwct(inputtype) "";
set nbspwct(outputfile) "";
set nbspwct(latestname) "";
#
set nbspwct(wct_enable) 1;	# option -n disables it

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

    if {$nbspwct(inputtype) eq "sat"} {
	exec_wct_sat;
    } else {
	exec_wct_default;
    }
}

proc exec_wct_sat {} {

    global nbspwct;
    global errorInfo;

    if {$nbspwct(wct_enable) == 1} {
	set status [catch {
	    exec $nbspwct(wct_bin) \
		$nbspwct(inputfile) \
		$nbspwct(outputfile) \
		$nbspwct(wct_fmt) \
		$nbspwct(wct_rcfile); 
	} errmsg];
    }

    # the extension of the main file
    set main_fext [lindex $nbspwct(wct_fext) 0];

    # the meta information in the name
    set fmeta $nbspwct(wct_fmeta_sat,$nbspwct(wct_fmt));

    foreach fext $nbspwct(wct_fext) {
	# This is the actual name that wct uses in the output file(s)
	set wct_name "";   # clear it
	if {$nbspwct(wct_enable) == 1} {
	    append wct_name [file rootname $nbspwct(outputfile)] $fmeta $fext;
	} else {
	    set _dir [file dirname $nbspwct(outputfile)];
	    set _name [file rootname [file tail $nbspwct(inputfile)]];
	    append wct_name [file join $_dir $_name] $fmeta $fext;
	}

	# This is the name we want
	if {$fext eq $main_fext} {
	    set output_name $nbspwct(outputfile);
	} else {
	    set output_name [file rootname $nbspwct(outputfile)];
	    append output_name $fext;
	}

	if {[file exists $wct_name] == 0} {
	    if {$nbspwct(wct_enable) == 1} {
		log_err $errmsg;
		if {$nbspwct(debug) != 0} {
		    log_err $errorInfo;
		}
	    } else {
		log_err "$wct_name not found.";
	    }
	} else {
	    file rename -force $wct_name $output_name;
	}
    }

    # Create latest link for the (main) outputfile
    if {$nbspwct(latestname) ne ""} {
	set savedir [file dirname $nbspwct(outputfile)];
	set savename [file tail $nbspwct(outputfile)];
	make_latest $savedir $savename $nbspwct(latestname);
    }
}

proc exec_wct_default {} {

    global nbspwct;
    global errorInfo;

    if {$nbspwct(wct_enable) == 1} {
	set status [catch {
	    exec $nbspwct(wct_bin) \
		$nbspwct(inputfile) \
		$nbspwct(outputfile) \
		$nbspwct(wct_fmt) \
		$nbspwct(wct_rcfile); 
	} errmsg];

	if {[file exists $nbspwct(outputfile)] == 0} {
	    log_err $errmsg;
	    if {$nbspwct(debug) != 0} {
		log_err $errorInfo;
	    }
	}
    } else {
	# the extension of the main file
	set main_fext [lindex $nbspwct(wct_fext) 0];

	foreach fext $nbspwct(wct_fext) {
	    # This is the actual name that wct uses in the output file(s)
	    set wct_name "";   # clear it
	    if {$nbspwct(wct_enable) == 1} {
		append wct_name [file rootname $nbspwct(outputfile)] \
		    $fmeta $fext;
	    } else {
		set _dir [file dirname $nbspwct(outputfile)];
		set _name [file rootname [file tail $nbspwct(inputfile)]];
		append wct_name [file join $_dir $_name] $fext;
	    }

	    # This is the name we want
	    if {$fext eq $main_fext} {
		set output_name $nbspwct(outputfile);
	    } else {
		set output_name [file rootname $nbspwct(outputfile)];
		append output_name $fext;
	    }

	    if {[file exists $wct_name] == 0} {
		log_err "$wct_name not found.";
	    } else {
		file rename -force $wct_name $output_name;
	    }
	}
    }

    # Create latest link for the (main) outputfile
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

if {$option(f) ne ""} {
    set nbspwct(wct_fmt) $option(f);
}
set nbspwct(wct_fext) $nbspwct(wct_fext,$nbspwct(wct_fmt));  # list of fext

if {$option(l) ne ""} {
    set nbspwct(latestname) $option(l);
}

if {$option(n) == 1} {
    set nbspwct(wct_enable) 0;
}

if {$option(p) ne ""} {
    set nbspwct(post_rcfile) $option(p);
}

if {$option(t) ne ""} {
    set nbspwct(inputtype) $option(t);
}

if {$option(x) ne ""} {
    set nbspwct(wct_rcfile) $option(x);
}

if {$option(V) != 0} {
    set nbspwct(debug) 1;
}

if {$option(w) ne ""} {
    set nbspwct(wct_bin) $option(w);
}

if {$option(o) ne ""} {
    set nbspwct(outputfile) $option(o);
    if {$option(d) ne ""} {
	set nbspwct(outputfile) [file join $option(d) $option(o)];
    }
} else {
    # the default main fext
    set fext [lindex $nbspwct(wct_fext) 0];
    append nbspwct(outputfile) \
    [file rootname [file tail $nbspwct(inputfile)]] $fext;
    if {$option(d) ne ""} {
	set nbspwct(outputfile) \
	    [file join $option(d) $nbspwct(outputfile)];
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
