#!%TCLSH%
#
# $Id$
#
# Usage: nbspradmapc [-b] [-c] [-v] [-K] [-L] [-d <output_subdir>]
#                    [-i] [-I] [-l <first,last>] [-o <outputfile>]
#                    [-r <rcfile>] [-R <rcfile_path>] <inputdir>
#
# Examples: nbspradmapc jua/n0q
#           nbspradmapc -l end-3,end jua/n0q
#
# This tool is designed for use from the command line.
# It can create individual images and/or a loop from them.
# [-l] The [-l] argument specifies a range of files to process from the list
#      that are in the specified directory. When -L is not given, the default
#      is "-l end,end". When -L is given, the default is "-l 0,end".
#      If the value of -l is a single number n, then it is
#      interpreted as end-n,end.
#
# [-iI] In the default configuration, the program expects the command line
#      argument to be of the form <site>/<type>, e.g., "jua/n0q". Then
#      the program constructs the input in two steps:
#      (i) a data sub directory (for this example) as
#      "nexrad/nids/jua/n0q".
#      (ii) this subdirectory is assumed to be under the dafilter data
#      directory "/var/noaaport/data/digamos".
#      If [-I] is given, the data directory is taken to be <inputdir>
#      and it is used "as is".
#      If the [-i] argument is given, then "/var/noaaport/data/digamos"
#      is prepended to the given to the data directory.
# [-o] The name of each output image is the basename of the data file, with the
#      "gif" extension. The [-o] gives the name of the loop
#      image file (the default is ${type}${site}.gif) when [-L] is given,
#      or of the latest file (neither -L nor -l given).
#
# [-d] Directory to put the output file (the default is the current directory.
# [-c] When creating a loop, if one file gives an error, ignore that file
#      and continue with the others.
# -L => create a loop from those images.
# -K => if -L is specified, keep (do not delete) the individual images
# -b => background
# -v => verbose
#
# Requires "gifsicle"

package require cmdline;

set usage {nbspradmapc [-b] [-c] [-v] [-I] [-K] [-L] [-d <output_subdir>]
    [-i] [-l <first,last>] [-o <outputfile>]
    [-r <rcfile>] [-R <rcfile_path>] <inputdir>};

set optlist {b c v I K L i {d.arg ""} {l.arg ""}  {o.arg ""}
    {r.arg ""} {R.arg ""}};

#
# defaults
#
set option(range,0) "end,end";     # if option(L) == 0
set option(range,1) "0,end";       # if option(L) == 1
set option(imgext) ".gif";
set option(loopext) ".gif";
set option(nidssubdir_default) [file join "nexrad" "nids" {$g(inputdir)}];
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
	return;
    }

    file mkdir [file dirname $loopfile];
    set looppath [file join $loopdir $loopfile];
    set looplock ${looppath}.lock.[pid];

    set status [catch {
      eval exec $rstfilter(radloop_program) \
	$rstfilter(radloop_program_preoptions) $flist \
	$rstfilter(radloop_program_postoptions)  > $looplock;
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

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc < 1} {
    log_err $usage;
}

set g(inputdir) [lindex $argv 0]; # default is, e.g., jua/n0q
append g(loopfile) \
    [string map {"/" "."} $g(inputdir)] $option(loopext); # e.g., jua.n0q.gif

if {$option(I) == 0} {
    # use the default
    eval set g(nidssubdir) $option(nidssubdir_default);
    set option(i) 1;
} else {
    # use it as given
    set g(nidssubdir) $g(inputdir);
}

if {$option(i) != 0} {
    set g(nidsdir) [file join $dafilter(datadir) $g(nidssubdir)];
} else {
    set g(nidsdir) $g(nidssubdir);
}

# First try the options then the config file.
if {$option(R) ne ""} {
    set rcfile $option(R);
} elseif {$option(r) ne ""} {
    set rcfile [filterlib_find_conf $option(r) \
        $rstfilter(radmap_rcdirs) $rstfilter(radmap_rcsubdir)];
} else {
    # This will make nbspradmap use the configuration file defaults.
    set rcfile "";
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

# Get the data file extension
set fext [file extension $dafilter(rad_namefmt)];

set frange_list [split $option(l) ","];
if {[llength $frange_list] == 1} {
    set last "end";
    set first "end-$option(l)";
} else {
    set first [lindex $frange_list 0];
    set last [lindex $frange_list 1];
}
set flist [lrange [lsort \
          [glob -directory $g(nidsdir) -tails -nocomplain *$fext] \
    ] $first $last];

if {[llength $flist] == 0} {
    log_err "Empty file list.";
}

#
# If we find the need to use this, we should add an option to the
# script "-a <awips>"
# set opts [list "-D" "awips=${g(type)}${g(site)}"];
#
set opts {};
if {$option(d) ne ""} {
    #lappend opts "-d" $option(d) "-t" $option(d);
    lappend opts "-t" $option(d);
}

set output_flist [list];
foreach f $flist {
    set nidspath [file join $g(nidsdir) $f];
    
    set nidsbasename [file rootname [file tail $nidspath]];
    set outputpath [string cat $nidsbasename $option(imgext)];

    if {($option(latest) == 1) && ($g(latestfile) ne "")} {
	set outputpath $g(latestfile);
    }

    # do it explicitly instead of using the [-d] option 
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

	# June 2025 - This is the old way to construct the list of images 
	#
	#lappend output_flist \
	#   [eval exec nbspradmap -v $opts $nidspath $rcfile];
	#
	eval exec nbspradmap $opts -o $outputpath $nidspath $rcfile;
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
