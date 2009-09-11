#!%TCLSH%
#
# $Id$
#
# usage: nbspcleanup [-f] <configfile>
#
# The <configfie> file is searched first in the main configuration directory,
# then in the "defaults" and "site" subdirectories, and the last one found
# is used.
#
# -f => Use the <confgfile> "as is" without searching the config directories
#

## Common defaults
set defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaultsfile] == 0} {
    puts "$argv0: $defaultsfile not found.";
    return 1;
}
source $defaultsfile;
unset defaultsfile;
#
package require cmdline;
package require hscheduler;

set usage {usage: nbspcleanup [-f] <configfile>};
set optlist {{f}};
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

set schedule [dict create];

proc process_dir {dir expression} {

    if {[file isdirectory $dir] == 0} {
	puts "Warning: $dir not found.";
	return;
    }

    puts "### Scheduled cleanup of $dir - [exec date]";

    set pwd [pwd];
    cd $dir;

    if {[regexp {^-c} $expression]} {
	process_dir_bycount "." $expression;
    } elseif {[regexp {^-i} $expression]} {
	process_dir_byinv "." $expression;
    } else {
	process_dir_bytime "." $expression;
    }

    cd $pwd;
}

proc process_dir_bytime {dir expression} {

    # In FreeBSD -delete always returns true, but in linux it can give
    # an error. If the error is not catched, the script exits at this
    # point.

    set status [catch {
	set output [eval exec find $dir $expression -delete -print];
	puts $output;
    } errmsg];

    if {$status != 0} {
	puts $errmsg;
    }
}

proc process_dir_bycount {dir expression} {
#
# expression is a cmd line option of the form 
#
# "-c <count> [-d] [-e <excludepatt>]"
#
# If "-d" is given, then directories are treated as the files, instead
# of being processed recursively (e.g., when dir has subdirectories
# with names like 20090223 20090224 ...).
#
    set optlist {{c.arg 0} {d} {e.arg ""}};
    set exprv [split $expression];
    array set option [::cmdline::getoptions exprv $optlist];
    set count $option(c);
    set excludepatt $option(e);

    set filelist [lsort [glob -directory $dir -nocomplain -types f *]];
    set dirlist [lsort [glob -directory $dir -nocomplain -types d *]];
    set filecount [llength $filelist];
    set dircount [llength $dirlist];

    if {$count < 0} {
	set count 0;
    }

    if {$filecount > $count} {
	set max_index_del [expr $filecount - $count - 1];
	set deletelist [lrange $filelist 0 $max_index_del];
	foreach f $deletelist {
	    if {($excludepatt eq "") || \
		    ([regexp $excludepatt [file tail $f]] == 0)} {
		file delete $f;
		puts $f;
	    }
	}
    }

    if {$option(d) == 0} {
	foreach d $dirlist {
	    process_dir_bycount $d $expression;
	}
	return;
    }

    # If option "d" is set, then process the directories the same as the
    # files, but delete with "-force".

    if {$dircount > $count} {
	set max_index_del [expr $dircount - $count - 1];
	set deletelist [lrange $dirlist 0 $max_index_del];
	foreach f $deletelist {
	    if {($excludepatt eq "") || \
		    ([regexp $excludepatt [file tail $f]] == 0)} {
		file delete -force $f;
		puts $f;
	    }
	}
    }
}

proc process_dir_byinv {dir expression} {
#
# Here expression is a cmd line option of the form: -i <find options>.
# We must delete the "-i" before passing the expression to find.
#
# $dir must be the parent directory of a set of "inventory directories".
# An inventory directory contains only inventory files (see nbspinvrm.c
# for the format and assumtions about an inventory file). This function
# finds the inventory directories within $dir (only at depth 1) that match
# the $expression. Then, for each of them, it calls ``find -exec nbspinvrm''
# to process each inventory file. It deletes each inventory after processing
# all the files in it.

    set exprv [split $expression];
    set expression [join [lrange $exprv 1 end] " "];

    # First get the list of inventory files and then process them one by one.
    # Each inventory file should must contain files with
    # the same [file dirname] and the first entry must be a full path.
    # Having inventory files in $dir actually defeats the purpose of this
    # function, since what we want to avoid is ``find'' evaluating the
    # $expression for individual files. But it could be used if the number of
    # of files is small, and for this reason we leave this here.

    # In linux, -maxdepth and -mindepth must be put before the expression
    # (as an option) or find will emit a warning.
    set status [catch {
	set output [eval exec find $dir -maxdepth 1 -mindepth 1 \
			$expression -type f -exec nbspinvrm -s -v {\{\}} {+}];
	if {$output ne ""} {
	    puts $output;
	}
    } errmsg];

    if {$status != 0} {
	puts $errmsg;
    }

    # Now process each inventory subdirectory. Each subdirectory is assumed to
    # contain inventory files only.
    set status [catch {
	set invlist \
	    [split [string trim [eval exec find $dir -maxdepth 1 -mindepth 1 \
				     $expression -type d]] "\n"];
    } errmsg];

    if {$status != 0} {
	puts $errmsg;
    }

    foreach invdir $invlist {
	set status [catch {
	    set output [eval exec find $invdir -type f \
			    -exec nbspinvrm -s -v {\{\}} {+}];
	    if {$output ne ""} {
		puts $output;
	    }

	    file delete $invdir;
	} errmsg];

	if {$status != 0} {
	    puts $errmsg;
	}
    }
}

proc read_schedule {schedule_varname conffile} {

    upvar $schedule_varname schedule;

    foreach line [split [string trim [exec cat $conffile]] "\n"] {

	if {[regexp {^#|^\s*$} $line]} {
	    continue;
	}

	set parts [split $line ":"];
	if {[llength $parts] < 2} {
	    puts "Error: Invalid specification: $line";	
	    continue;
	}

	set dir [string trim [lindex $parts 1]];
	dict set schedule $dir $line;
    }	
}

#
# main
#
puts "- Scheduled run of nbspcleanup - [exec date]";

if {$argc == 0} {
    puts $usage;
    exit 1;
}
set confname [lindex $argv 0];

if {$option(f) == 1} {
    if {[file exists $confname]} {
	set conffile $confname;
	read_schedule schedule $confname;
    }
} else {
    # Search all of them in the standard config directories
    set conffile "";
    foreach d [concat $common(confdir) $common(localconfdirs)] {
        if {[file exists [file join $d $confname]]} {
            set conffile [file join $d $confname];
	    read_schedule schedule $conffile;
        }
    }
}

if {($conffile eq "") || ([file exists $conffile]  == 0)} {
    puts "$confname not found.";
    exit 1;
}

# Process every line in the schedule
dict for {dir line} $schedule {

    set parts [split $line :];
    set hour [string trim [lindex $parts 0]];
    set dir [string trim [lindex $parts 1]];
    if {[llength $parts] == 3} {
        set excludedirs "";
        set options [string trim [lindex $parts 2]];
    } elseif {[llength $parts] == 4} {
        set excludedirs [string trim [lindex $parts 2]];
        set options [string trim [lindex $parts 3]];
    } else {
	puts "Error: Invalid specification: $line";	
	continue;
    }

    set status [catch {
        set match [::hscheduler::match_timespec $hour];
    } errmsg];

    if {$status != 0} {
        puts $errmsg;
        continue;
    } elseif {$match == 0} {
        continue;
    }

    if {$excludedirs == ""} {
	process_dir $dir $options;
    } else {
	foreach subdir [glob -directory $dir -nocomplain -type d *] {
	    if {[regexp $excludedirs [file tail $subdir]]} {
		continue;
	    }
	    process_dir $subdir $options;
	}
    }
}
