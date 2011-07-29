#!%TCLSH%
#
# $Id$
#
# Usage: nbspsatmapc [-d <output_subdir>] [-l <first,last>] \
#			[-n <gini_subdir>] [-o <outputfile>] [-r <rcfile>] \
#			[-R <rcfile_path>] [-v] [-L] [-K] <wmoid>
#
# Examples: nbspsatmapc tige01
#           nbspsatmapc -l end-3,end tige01
#
# This tool is designed for use from the command line.
# It can create individual images and/or a loop from them.
# The [-l] argument specifies a range of files to process from the list
# that are in the specified directory. When -L is not given, the default
# is "-l end,end". When -L is given, the default is "-l 0,end".
# If the value of -l is a single number n, then it is interpreted as end-n,end.
# The [-n] <gini_subdirectory> argument is relative to the dafilter
# data directory (e.g., sat/gini/tig). If -L is not given and there is only
# one output image, the [-o] option can be used to specify the name
# of the output image. If -L is given then -o gives the name of the loop image
# file. The default is ${wmoid}.gif in any case.
#
# -L => create a loop from those images.
# -K => if -L is specidied, keep (do not delete) the individual images
#
# Requires "gifsicle"

package require cmdline;

set usage {nbspsatmapc [-v] [-d <output_subdir>] [-l <first,last>]
    [-n <gini_subdir>] [-o <outputfile>] [-r <rcfile>] [-R <rcfile_path>]
    [-L] [-K] <wmoid>};

set optlist {v {d.arg ""} {l.arg ""} {n.arg ""} {o.arg ""}
    {r.arg ""} K L {R.arg ""}};

#
# defaults
#
set option(range,0) "end,end";     # if option(L) == 0
set option(range,1) "0,end";       # if option(L) == 1
set option(ginisubdir) [file join "sat" "gini" "tig"];
set option(fext) ".gif";

proc log_warn s {

    global argv0;

    set name [file tail $argv0];
    exec logger -t $name $s;
    puts stderr "$name: Error: $s";
}

proc log_err s {

    log_warn $s;
    exit 1;
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

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc < 1} {
    log_err $usage;
}

# First try the options then the config file.
if {$option(R) ne ""} {
    set rcfile $option(R);
} elseif {$option(r) ne ""} {
    set rcfile [filterlib_find_conf $option(r) \
        $rstfilter(radmap_rcdirs) $rstfilter(radmap_rcsubdir)];
} else {
    # This will make nbspsatmap use the configuration file defaults.
    set rcfile "";
}

if {$option(n) eq ""} {
    set option(n) $option(ginisubdir);
}

if {$option(l) eq ""} {
    if {($option(L) == 1) && ($rstfilter(satloop_count) > 0)} {
	set option(l) "end-$rstfilter(satloop_count),end";
    } else {
	set option(l) $option(range,$option(L));
    }
}

set wmoid [lindex $argv 0];
set ginidir [file join $dafilter(datadir) $option(n) $wmoid];

if {$option(o) eq ""} {
    append option(o) $wmoid $option(fext);
}

# Get the data file extension
set fext [file extension $dafilter(sat_namefmt)];

set frange_list [split $option(l) ","];
if {[llength $frange_list] == 1} {
    set last "end";
    set first "end-$option(l)";
} else {
    set first [lindex $frange_list 0];
    set last [lindex $frange_list 1];
}
set flist [lrange [lsort \
          [glob -directory $ginidir -tails -nocomplain *$fext] \
    ] $first $last];

if {[llength $flist] == 0} {
    log_err "Empty file list.";
}

set opts [list "-D" "wmoid=$wmoid"];
if {$option(d) ne ""} {
    lappend opts "-d" $option(d) "-t" $option(d);
}

if {($option(L) == 0) && ([llength $flist] == 1)} {
    lappend opts "-o" $option(o);
}

set output_flist [list];
foreach f $flist {
    set ginipath [file join $ginidir $f];

    if {$option(v) == 1} {
	puts -nonewline "$f ... ";
    }

    set status [catch {
	set params [split [eval exec nbspsatmap $opts $ginipath $rcfile]];
	lappend output_flist [lindex $params end];
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

set loopfile $option(o);
set loopdir $option(d);

make_loop $output_flist $loopdir $loopfile;

if {$option(K) == 0} {
    foreach outputfile $output_flist {
	file delete $outputfile;
    }
}
