#!%TCLSH%
#
# $Id$
#
# Usage: nbspradmapc [-d <output_subdir>] [-f <fext>] [-l <first,last>] \
#			[-n <nids_subdir>] [-r <rcfile>] \
#			[-R <rcfile_path>] [-v] <site> <type>
#
# Examples: nbspradmapc jua n0r
#           nbspradmapc -l end-3,end jua n0r
#
# This tool is designed for use from the command line.
# The [-l] argument specifies a range of files to process from the list
# that are in he specified directory. The [-n] <nids_subdirectory> argument
# is relative to the dafilter data directory (e.g., nexrad/nids).
# The [-f] option specifies the file extension of the data files (e.g., nids)
# to distinguish them from other files (e.g, latest, ...).
#
package require cmdline;

set usage {nbspradmapc [-d <output_subdir>] [-f <fext>] [-l <first,last>]
    [-n <nids_subdir>] [-r <rcfile>] [-R <rcfile_path>] [-v] <site> <type>};
set optlist {{d.arg ""} {f.arg ".nids"} {l.arg "end,end"} {n.arg ""}
    {r.arg ""} {R.arg ""} {v 0}};
array set option [::cmdline::getoptions argv $optlist $usage];

if {$option(n) eq ""} {
    set option(n) [file join "nexrad" "nids"];
}

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

#
# main
#

# Read the init (instead of conf) because the filterlib_find_conf() function
# from filter.lib is used.
set f "/usr/local/libexec/nbsp/filters.init";
if {[file exists ${f}] == 0} {
   log_err "$f not found.";
}
source $f;

foreach f [list "rstfilter.init" "dafilter.init"] {
    set f [file join $common(libdir) $f];
    if {[file exists ${f}] == 0} {
	log_err "$f not found.";
    }
    source $f;
}
unset f;

# First try the options then the config file.
if {$option(R) ne ""} {
    set rcfile $option(R);
} elseif {$option(r) ne ""} {
    set rcfile [filterlib_find_conf $option(r) \
        $rstfilter(radmap_rcdirs) $rstfilter(radmap_rcsubdir)];
} else {
    # This will nbspradmap use the configuration file defaults.
    set rcfile "";
}

if {$argc < 2} {
    log_err $usage;
}
set site [lindex $argv 0];
set type [lindex $argv 1];
set nidsdir [file join $dafilter(datadir) $option(n) $site $type];

set frange_list [split $option(l) ","];
set first [lindex $frange_list 0];
set last [lindex $frange_list 1];
set flist [lrange [lsort \
          [glob -directory $nidsdir -tails -nocomplain *$option(f)] \
    ] $first $last];

if {[llength $flist] == 0} {
    log_err "Empty file list.";
}

set opts [list "-D" "awips=${type}${site}"];
if {$option(d) ne ""} {
    lappend opts "-d" $option(d) "-t" $option(d);
}

foreach f $flist {
    set nidspath [file join $nidsdir $f];

    if {$option(v) == 1} {
	puts -nonewline "$f ... ";
    }

    set status [catch {
	eval exec nbspradmap $opts $nidspath $rcfile;
    } errmsg];

    if {$status != 0} {
	log_err $errmsg;
    } elseif {$option(v) == 1} {
	puts "OK";
    }
}
