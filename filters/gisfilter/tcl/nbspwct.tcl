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
# -t => type: sat, rad, ... (not currently used)
# -V => debug wct errors
# -w => path to wct-export
# -x => the wct xml config file (uses the built-in defaults otherwise)
#
package require fileutil;
package require cmdline;

set usage {nbspwct [-b] [-d <outputdir>] [-f <fmt>]
    [-K] [-l <latestname>] [-n] [-o <outputfile>]
    [-p <postrc>] [-t <type>] [-V] [-w <wctbin>]
    [-x <wctconfigfile>] <inputfile>};
set optlist {b {d.arg ""} {f.arg ""} K {l.arg ""} n {o.arg ""} {p.arg ""}
    {t.arg ""} V {w.arg ""} {x.arg ""}};

# defaults
set nbspwct(wct_bin) "wct-export";
set nbspwct(wct_rcfile) "";
set nbspwct(wct_rcname_default) "wct-export.xml";

# parameters
set nbspwct(wct_cachedir) [file join ~/ ".wct-cache"];
set nbspwct(wct_fmt) "tif";	# default
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
set nbspwct(post_rcfile) "";
#
set nbspwct(inputfile) "";
set nbspwct(inputtype) "";	# not currently used
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

proc log_wct_err s {

    global option;
    global errorInfo;

    if {$option(V) == 1} {
	log_warn $s;
	log_err $errorInfo;
    } else {
	log_err $s;
    }
}    

proc exec_wct {} {

    global nbspwct;

    exec_wct_default;
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
	    log_wct_err $errmsg;
	}
    }

    # The problem is that wct does not always honor the output file name;
    # it adds some meta data to it, e.g., if we pass
    #
    # tige04_20100910_2115.tif
    #
    # it creates
    #
    # tige04_20100910_2115-var-1-z0-rt0-t0-8bit.tif
    #
    # The situation is worse with other types (e.g., shp) because there
    # are several files with different extensions.

    set output_dir [file dirname $nbspwct(outputfile)];
    set output_root [file rootname $nbspwct(outputfile)];

    # This is the actual name (without the extension)
    # that wct uses in the output file(s)
    if {$nbspwct(wct_enable) == 1} {
	set wct_name [file rootname [file tail $nbspwct(outputfile)]];
    } else {
	set wct_name [file rootname [file tail $nbspwct(inputfile)]];
    }

    set files [glob -nocomplain -directory $output_dir "${wct_name}*"];
    if {[llength $files] == 0} {
	# No output file produced.
	if {$nbspwct(wct_enable) == 1} {
	    log_wct_err $errmsg;
	} else {
	    log_err "No $wct_name files found in $output_dir.";
	}
    }

    foreach f $files {
	set fext [file extension $f];
	set newname $output_root;
	append newname $fext;
	if {$f ne $newname} {
	    file rename -force $f $newname;
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

proc write_wct_defaults_file {} {

    global nbspwct;

    set nbspwct(wct_rcbody_default) {
	@wct_defaults_xml@
    }

    set nbspwct(wct_rcfile) [file join [file dirname $nbspwct(outputfile)] \
				 $nbspwct(wct_rcname_default)];

    # If wct will not be executed, there is no need to actually create the
    # file.
    if {$nbspwct(wct_enable) == 1} {
	::fileutil::writeFile \
	    $nbspwct(wct_rcfile) $nbspwct(wct_rcbody_default);
    }
}

proc clean_wct_cache_dir {} {

    global nbspwct option;

    if {($option(K) == 1) && [file isdirectory $nbspwct(wct_cachedir)]} {
	file delete -force $nbspwct(wct_cachedir);
    }
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

# This should go after the outputfile because the default wct config file
# is written in the output directory if it is not specified.
#
if {$option(x) ne ""} {
    set nbspwct(wct_rcfile) $option(x);
} else {
    write_wct_defaults_file;
}

# The post_rcfile is optional but the wct rcfile is required.
if {[file exists $nbspwct(wct_rcfile)] == 0} {
    log_err "$nbspwct(wct_rcfile) not found.";
}

clean_wct_cache_dir;
exec_wct;
exec_post $nbspwct(post_rcfile) $nbspwct(outputfile);
clean_wct_cache_dir;

if {$option(x) eq ""} {
    file delete $nbspwct(wct_rcfile);
}
