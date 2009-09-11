#!%TCLSH%
#
# $Id$
#
# Usage: nbspstatcounters [-f statusfile] [-n numlines]
#
# If no [-f] is given the default nbspd.status file is used.
# By default only the last line is output, unless [-n] is given.
# The data is written to stdout; the order of the columns is given below.
# The motivation for the existence of this tool is to use it for extracting
# and feeding the data to rrdtool or similar programs.

package require cmdline;
set usage {nbspstatcounters [-f statusfile] [-n numlines]};
set optlist {{f.arg ""} {n.arg ""}};

## The common defaults
set _defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists ${_defaultsfile}] == 0} {
    puts "${_defaultsfile} not found.";
    return 1;
}
source ${_defaultsfile};
unset _defaultsfile;

# nbspd.init is needed for nbspd.status
set nbspd_init_file [file join $common(libdir) nbspd.init];
if {[file exists $nbspd_init_file] == 0} {
        puts "$nbspd_init_file not found.";
        return 1;
}
source $nbspd_init_file;
unset nbspd_init_file;

array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$option(f) == ""} {
    set datafile $nbspd(statusfile);
} else {
    set datafile $option(f);
}

#
# These are the fields of the nbspd.status file.
#
set variables [list \
unix_seconds \
frames_received \
frames_processed \
frames_jumps \
frames_bad \
frames_data_size \
products_transmitted \
products_completed \
products_missed \
products_retransmitted \
products_retransmitted_complete \
products_retransmitted_processed \
products_retransmitted_ignored \
products_retransmitted_recovered \
products_rejected \
products_aborted \
products_bad];

if {$option(n) == ""} {
    set values [split [exec tail -n 1 $datafile]];
    
puts {#
# nbsp counters in the last minute (ending at "unix_seconds").
#
DATA_START};

    set i 0;
    foreach k $variables {
      set v [lindex $values $i];
      puts "$k = $v";
	incr i;
    }

    puts "DATA_END";
} else {
    if {$option(n) <= 0} {
    puts {Bad value of [-n].};
    return 1;
    } else {
	puts [exec tail -n $option(n) $datafile];
    }
}
