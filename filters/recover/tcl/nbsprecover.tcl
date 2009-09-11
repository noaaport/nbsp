#!%TCLSH%
#
# $Id$
#
# Usage: nbsprecover [-f missingfile]
#
# If no [-f] is given the default nbspd.missing file is used.
# The tool keeps a state file that contains the index of the last missing
# file processed in the previous run. The tool then goes through the list
# of missing files and tries to recover those in the list starting with
# the next one.

package require cmdline;
package require fileutil;

set usage {nbsprecover [-f missingfile]};
set optlist {{f.arg ""}};

## The common defaults
set _defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists ${_defaultsfile}] == 0} {
    puts "${_defaultsfile} not found.";
    return 1;
}
source ${_defaultsfile};
unset _defaultsfile;

# nbspd.init is needed for spool dir and nbspd.missing
set nbspd_init_file [file join $common(libdir) nbspd.init];
if {[file exists $nbspd_init_file] == 0} {
        puts "$nbspd_init_file not found.";
        return 1;
}
source $nbspd_init_file;
unset nbspd_init_file;

# recover.init
set recover_init_file [file join $common(libdir) recover.init];
if {[file exists $recover_init_file] == 0} {
        puts "$recover_init_file not found.";
        return 1;
}
source $recover_init_file;
unset recover_init_file;

#
# variables of this tool
#
set statefile [file join $recover(dir) "nbsprecover.state"];
set recoverfilter [file join $common(libdir) "recover"];

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$option(f) eq ""} {
    set missingfile $nbspd(missingfile);
} else {
    set missingfile $option(f);
}

# Get the list of mssing files
if {[file exists $missingfile] == 0} {
    exit 0;
}
set missing_list [split [string trim [::fileutil::cat $missingfile]] "\n"];
if {[llength $missing_list] == 0} {
    exit 0;
}

#
# These are the fields of the nbspd.missing file.
#
set variables [list unix_seconds seq type cat code npchidx fbasename];

set last_start 0;
set last_end -1;
if {[file exists $statefile]} {
    set last_state [split [::fileutil::cat $statefile]];
    set last_start [lindex $last_state 0];
    set last_end [lindex $last_state 1];
}

if {$last_end >= 0} {
    set last_start $last_end;
    incr last_start;
    set missing_list [lrange $missing_list $last_start end];
}

# Send the missing_list to the recover filter.
set status [catch {
    set F [open "|$recoverfilter" w];
    foreach entry $missing_list {
	incr last_end;
	puts $F $entry;
    }
} errmsg];
if {[info exists F]} {
    close $F;
}
::fileutil::writeFile $statefile [join [list $last_start $last_end] " "];

if {$status != 0} {
    puts $errmsg;
}
