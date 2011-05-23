#!%TCLSH%
#
# $Id$
#
# Usage: nbspstatcounters [-f <fmt>] [-q <qstatefile>] [-s <statusfile>]
#
# nbsp counters in the last minute (ending at "unix_seconds").
#
# If no <statusfile> is given the default nbspd.status file is used.
# If no <qstatefile> is given the default nbspd.qstate file is used.
# The last line is written to stdout; the order of the columns is given below.
# The <fmt> can be: std (default), xml, csv, csvk
# The motivation for the existence of this tool is to use it for extracting
# and feeding the data to rrdtool or similar programs (it is used
# by the _inbsp extension in the web interface).

package require cmdline;
set usage {nbspstatcounters [-f <fmt>] [-q <qstatefile>] [-s <statusfile>]};
set optlist {{f.arg "std"} {q.arg ""} {s.arg ""}};

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

# defaults
set fmt $option(f);
set qstatefile $nbspd(qstatelogfile);
set statusfile $nbspd(statusfile);

if {$option(q) ne ""} {
    set qstatefile $option(q);
}

if {$option(s) ne ""} {
    set statusfile $option(s);
}

#
# These are the fields of the nbspd.status file, followed by the 
# nbspd.qstate file.
#
set keywords [list \
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
products_bad \
queue_processor \
queue_filter \
queue_server];

set qvalues [split [exec tail -n 1 $qstatefile]];
set svalues [split [exec tail -n 1 $statusfile]];
set values [concat $svalues [lrange $qvalues 1 end]];

if {$fmt eq "std" } {    
    puts {#
# nbsp counters in the last minute (ending at "unix_seconds").
#
DATA_START};

    foreach k $keywords v $values {
	puts "$k = $v";
    }
    puts "DATA_END";
} elseif {$fmt eq "csv"} {
    puts [join $values ","];
} elseif {$fmt eq "csvk"} {
    set r "";
    foreach k $keywords v $values {
	append r "$k=$v,";
    }
    puts [string trim $r ","];
} elseif {$fmt eq "xml"} {
    foreach k $keywords v $values {
	puts "<$k>$v</$k>";
    }
} else {
    puts "Invalid format: $fmt";
    return 1;
}
