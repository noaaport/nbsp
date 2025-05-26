#!%TCLSH%
#
# $Id$
#
# Usage: nbspgoesrmapc [-v] [-b] [-K] [-L] [-g] [-d <outputdir>]
#       [-i] [-I <goesrsubdir>] [-l <first,last>] [-m] [-o <outputfile>] <wmoid>
#
# This is the analog of nbspsatmapc (in rstfilter/gismap), but with
# some differences. By default, it calls nbspgoesr, and therefore it does not
# include the map. With the [-m] option it calls nbspgoesrmap, which includes
# the map (via map2img).
#
# Requires "gifsicle" for the loops
#
# NOTE: Fri May 23 21:22:44 AST 2025
#       The -L option should not be used because the files under
#       a given wmoid do not correspond to the same "tile".
#       So this program is effective only for creating the "latest" image.
#
# Examples: nbspgoesrcmapc tire05 (no map included)
#           nbspgoesrcmapc -l end-3,end tire05  (the last three)
#           nbspgoesrcmapc -g tire05 (produces gif)
#           nbspgoesrcmapc -m tire05 (includes the map)
# 
# This tool can create individual images and/or a loop from them.
# [-l] The [-l] argument specifies a range of files to process from the list
#      that are in the specified directory. When -L is not given, the default
#      is "-l end,end". When -L is given, the default is "-l 0,end".
#      If the value of -l is a single number n, then it is interpreted as
#      end-n,end.
# [-I] specifies the data directory; the default, for tire05 as example,
#      is "sat/goesr/tir/tire05". If [-i] is given in addition to [-I],
#      then the [-I] <goesr_subdir> argument is assumed to be relative to the
#      dafilter data directory (/var/noaaport/data/digamos). If [-I] is not
#      given, the program behaves as if [-i] is given.
# [-o] The name of each output image is the basename of the data file, with the
#      "png" extension. If -L is given then -o gives the name of the loop
#       image file; the default is ${wmoid}.gif.
# -g => gif format (-L => -g)
# -L => create a loop from those images.
# -K => if -L is specified, keep (do not delete) the individual images
# -m => includes the map (uses map2img via nbspgoesrmap)

set usage {nbspgoesrmapc [-b] [-v] [-K] [-L] [-g] [-m] [-d <outputdir>]
    [-i] [-I <goesrsubdir>] [-l <first,last>] [-o <outputfile>] <wmoid>};

set optlist {b v K L g i m {d.arg ""} {l.arg ""} {I.arg ""} {o.arg ""}};

package require cmdline;

#
# defaults
#
set option(range,0) "end,end";     # if option(L) == 0
set option(range,1) "0,end";       # if option(L) == 1
set option(imgext) ".png";
set option(loopext) ".gif";
# example: sat/goesr/tir/tire05
set option(default_I) [file join "sat" "goesr" {$g(wmoid02)} {$g(wmoid)}];

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

proc log_msg s {

    global argv0;
    global option;

    if {$option(v) == 0} {
	return;
    }

    set name [file tail $argv0];
    if {$option(b) == 0} {
        puts $s;
    } else {
        exec logger -t $name $s;
    }
}

proc make_loop {flist loopdir loopfile} {

    global rstfilter;

    if {[llength $flist] == 0} {
	return;
    }

    file mkdir [file dirname $loopfile];
    set looppath [file join $loopdir $loopfile];
    set looplock ${looppath}.lock.[pid];

    set status [catch {
      eval exec $rstfilter(satloop_program) \
	$rstfilter(satloop_program_preoptions) $flist \
	$rstfilter(satloop_program_postoptions)  > $looplock;
    } errmsg];

    if {$status == 0} {
	if {[file exists $looplock]} {
	    file rename -force $looplock $looppath;
	}
    } else {
	file delete $looplock;
	log_err $errmsg;
    } 
}

## The common defaults
set defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaultsfile] == 0} {
   log_err "$defaultsfile not found.";
}
source $defaultsfile;
source $common(filterslib);
foreach f [list "rstfilter.init" "dafilter.init"] {
    set f [file join $common(libdir) $f];
    if {[file exists ${f}] == 0} {
	log_err "$f not found.";
    }
    source $f;
}
unset f;

# variables
set g(loopfile) "";
set g(nbspgoesrimgprog) "nbspgoesr"; # or nbspgoesrmap if [-m] is given

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc < 1} {
    log_err $usage;
}

set g(wmoid) [lindex $argv 0];			# e.g., tire05
set g(wmoid02) [string range $g(wmoid) 0 2];	# e.g., tir

# [-L] requires that the output files be gif
if {$option(L) == 1} {
    set option(g) 1;
}

if {$option(g) == 1} {
    set option(imgext) ".gif";
}

if {$option(m) == 1} {
    set g(nbspgoesrimgprog) "nbspgoesrmap";
}

if {$option(I) eq ""} {
    # use the default
    eval set g(goesrsubdir) $option(default_I);
    set option(i) 1;
} else {
    set g(goesrsubdir) $option(I);
}

if {$option(i) != 0} {
    set g(goesrdir) [file join $dafilter(datadir) $g(goesrsubdir)];
} else {
    set g(goesrdir) $g(goesrsubdir);
}

if {$option(l) eq ""} {
    if {($option(L) == 1) && ($rstfilter(satloop_count) > 0)} {
	set option(l) "end-$rstfilter(satloop_count),end";
    } else {
	set option(l) $option(range,$option(L));
    }
}

# default output file for the loop file
append g(loopfile) $g(wmoid) $option(loopext);
if {$option(o) ne ""} {
    set g(loopfile) $option(o);
}

# Get the data file extension
set fext [file extension $dafilter(sat_namefmt_goesr)];

set frange_list [split $option(l) ","];
if {[llength $frange_list] == 1} {
    set last "end";
    set first "end-$option(l)";
} else {
    set first [lindex $frange_list 0];
    set last [lindex $frange_list 1];
}
set flist [lrange [lsort \
          [glob -directory $g(goesrdir) -tails -nocomplain *$fext] \
    ] $first $last];

if {[llength $flist] == 0} {
    log_err "Empty file list.";
}

set output_flist [list];
foreach f $flist {
    set goesrpath [file join $g(goesrdir) $f];

    set goesrbasename [file rootname [file tail $goesrpath]];
    set outputpath [string cat ${goesrbasename} $option(imgext)];
    if {$option(d) ne "" } {
	set outputpath [file join $option(d) $outputpath];
    }
    
    if {$option(v) == 1} {
	puts -nonewline "$f ... ";
    }

    set status [catch {
	if {$option(d) ne ""} {
	    file mkdir $option(d);
	}

	if {$option(g) == 0} {
	    exec $g(nbspgoesrimgprog) -o $outputpath $goesrpath;
	} else {
	    exec $g(nbspgoesrimgprog) $goesrpath \
		| pngtopam 2> /dev/null | pamtogif > $outputpath 2> /dev/null;
	}
	    
	lappend output_flist $outputpath;
    } errmsg];

    if {$status != 0} {
	log_err $errmsg;
    } elseif {$option(v) == 1} {
	puts "OK";
    }
}

# Make a loop if requested
if {$option(L) == 0} {
    return;
}

set loopfile g(loopfile);
set loopdir $option(d);

make_loop $output_flist $loopdir $loopfile;

if {$option(K) == 0} {
    foreach outputfile $output_flist {
	file delete $outputfile;
    }
}
