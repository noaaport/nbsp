#!%TCLSH%
#
# $Id$
#
# Usage: nbspgoesrmapc [-v] [-b] [-a] [-K] [-L] [-c] [-g] [-d <outputdir>]
#	[-i] [-I] [-l <first,last>] [-m] [-o <outputfile>] <inputdir>
#
# This is the analog of nbspsatmapc (in rstfilter/gismap), but with
# some differences. By default, it calls nbspgoesr, and therefore it does not
# include the map. With the [-m] option it calls nbspgoesrmap, which includes
# the map (via map2img).
#
# Requires "gifsicle" for the loops
#
# Examples: nbspgoesrcmapc tire05/pao (no map included)
#           nbspgoesrcmapc -l end-3,end tire05/pao  (the last four)
#           nbspgoesrcmapc -g tire05/pao (produces gif)
#           nbspgoesrcmapc -m tire05/pao (includes the map)
#
# An example to work with the asc files inserted by satftp (from the
# aws-s3 files)
#
#           nbspgoesrmapc -a -i -I sat/goesr/g19/z01/c02
#
# This tool can create individual images and/or a loop from them.
# [-l] The [-l] argument specifies a range of files to process from the list
#      that are in the specified directory. When -L is not given, the default
#      is "-l end,end" (the "latest"). When -L is given, the default is
#      "-l 0,end". If the value of -l is a single number n, then it is
#      interpreted as end-n,end.
# [-iI] In the default configuration, the program expects the command line
#      argument to be of the form <wmoid/bbb>, e.g., "tire05/pao". Then
#      the program constructs the input in two steps:
#      (i) a data sub directory (for this example) as
#      "sat/goesr/tir/tire05/pao".
#      (ii) this subdirectory is assumed to be under the dafilter data
#      directory "/var/noaaport/data/digamos".
#      If [-I] is given, the command line argument <inputdir> is taken "as is".
#      If the [-i] argument is given, then the
#      "/var/noaaport/data/digamos" is prepended to the given <inputdir>
#      
# [-o] The name of each output image is the basename of the data file, with the
#      "png" extension. The [-o] gives the name of the loop
#      image file (the default is ${wmoid}.${bbb}.gif) when [-L] is given,
#      or the name of the image file when generating the "latest"
#      (neither -L nor -l is given).
#      
# [-d] Directory to put the output file (the default is the current directory.
#
# [-c] When creating a loop, if one file gives an error, ignore that file
#      and continue with the others.
# -g => gif format (-L => -g)
# -L => create a loop from those images.
# -K => if -L is specified, keep (do not delete) the individual images
# -m => includes the map (uses map2img via nbspgoesrmap)
# -a => the files in the inputdir are asc (implies -m)
#
set usage {nbspgoesrmapc [-b] [-v] [-I] [-K] [-L] [-a] [-c] [-g] [-m]
    [-d <outputdir>] [-i] [-l <first,last>] [-o <outputfile>] <inputdir>};

set optlist {b v I K L a c g i m {d.arg ""} {l.arg ""} {o.arg ""}};

package require cmdline;

#
# defaults
#
set option(range,0) "end,end";     # if option(L) == 0
set option(range,1) "0,end";       # if option(L) == 1
set option(imgext) ".png";
set option(loopext) ".gif";
# example: sat/goesr/tir/tire05/pao for inputdir = "tire05/pao"
set option(goesrsubdir_default) \
    [file join "sat" "goesr" {$g(wmoid02)} {$g(inputdir)}];
set option(latest) 0; # set below depending on -L and -l

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
	log_msg "Empty image files list";
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
set g(loopfile) "";	# determined dynamically below
set g(latestfile) "";	# determined dynamically below
set g(nbspgoesrimgprog) "nbspgoesr"; # or nbspgoesrmap if [-m] is given

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc < 1} {
    log_err $usage;
}

set g(inputdir) [lindex $argv 0];    # default is e.g., tire05/pao
append g(loopfile) \
    [string map {"/" "."} $g(inputdir)] $option(loopext); # e.g., tire05.pao.gif

# Get the (default if [-a] is not given) data file extension
set fext [file extension $dafilter(sat_namefmt_goesr)];
#
# [-a] implies [-m]
#
if {$option(a) == 1} {
    set fext ".asc";
    set option(m) 1;
}

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

if {$option(I) == 0} {
    # use the default, e.g., tire05/pao
    set g(wmoid02) [string range $g(inputdir) 0 2];	# e.g., tir
    eval set g(goesrsubdir) $option(goesrsubdir_default);
    set option(i) 1;
} else {
    # use it as given
    set g(goesrsubdir) $g(inputdir);
}

if {$option(i) != 0} {
    set g(goesrdir) [file join $dafilter(datadir) $g(goesrsubdir)];
} else {
    set g(goesrdir) $g(goesrsubdir);
}

if {$option(l) eq ""} {
    set option(l) $option(range,$option(L));
    if {$option(l) eq "end,end"} {
	set option(latest) 1;
    }
}

if {$option(o) ne ""} {
    set g(loopfile) $option(o);
    set g(latestfile) $option(o);
}

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

    if {($option(latest) == 1) && ($g(latestfile) ne "")} {
	set outputpath $g(latestfile);
    }

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
	    if {$option(a) == 0} {
		exec $g(nbspgoesrimgprog) -o $outputpath $goesrpath;
	    } else {
		exec $g(nbspgoesrimgprog) -a -o $outputpath $goesrpath;
	    }
	} else {
	    if {$option(a) == 0} {
		exec $g(nbspgoesrimgprog) $goesrpath \
		  | pngtopam 2> /dev/null | pamtogif > $outputpath 2> /dev/null;
	    } else {
		exec $g(nbspgoesrimgprog) -a $goesrpath \
		  | pngtopam 2> /dev/null | pamtogif > $outputpath 2> /dev/null;
	    }
	}	   
    } errmsg];

    if {$status == 0} {	    
	lappend output_flist $outputpath;
	 if {$option(v) == 1} {
	     puts "OK";
	 }
    } else {
	file delete $outputpath;
	if {$option(c) == 0} {
	    break;
	} elseif {$option(L) == 1} {
	    set status 0;
	}
    }
}

if {$status != 0}  {
    foreach outputfile $output_flist {
	file delete $outputfile;
    }
    log_err $errmsg;
}

# Make a loop if requested
if {$option(L) == 0} {
    return;
}

set loopfile $g(loopfile);
set loopdir $option(d);

make_loop $output_flist $loopdir $loopfile;

if {$option(K) == 0} {
    foreach outputfile $output_flist {
	file delete $outputfile;
    }
}
