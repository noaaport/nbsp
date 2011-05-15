#!%TCLSH%
#
# $Id$
#
# Usage: nbspstatplotdata [-o outputfile [-b basedir] [-d subdir]]
#			[-h <hh>] [-m] [-D mday] [statusfile]
#
# Without options, the data is written to stdout. Otherwise, 
# the tool will cd to the "basedir", create "subdir", and save
# the data in what is given in the [-o] option. If no [statusfile] is given
# the default nbspd.status file is used. [-D] can specify a day.
# The [-m] option instructs to output minutely (as opposed to hourly) data.
# The [-h] option specifies a cut-off hour <hh>. Without that option,
# all the hours in the file are included. With the [-h] option, only
# those hours <hh> that are equal or greater than the cutoff are included.
# If <hh> is a negative number -n, then the cutoff hour is taken to be the
# current hour minus n.
#
# The order of the columns is
#
# <frames received> <frames jumps> \
# <files transmitted> <files retransmitted> <files missed> \
# <MBytes>
#
# This preceeded by <hh> (for hourly files) or <hh:mm> for minutely files.

package require cmdline;
package require fileutil;

set usage {nbspstatplotdata [-o outputfile [-b basedir] [-d subdir]]
    [-h <hh>] [-m] [-D mday] [statusfile]};
set optlist {{o.arg ""} {b.arg ""} {d.arg ""} {h.arg ""} m {D.arg ""}};

## The common defaults
set _defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists ${_defaultsfile}] == 0} {
    puts stderr "${_defaultsfile} not found.";
    return 1;
}
source ${_defaultsfile};
unset _defaultsfile;

# The default configuration is shared between the filter and cmd-line tools
# and therefore it is in a separate file that is read by both.
set inv_init_file [file join $common(libdir) inventory.init];
if {[file exists $inv_init_file] == 0} {
    puts stderr "$inv_init_file not found.";
    return 1;
}
source $inv_init_file;
unset inv_init_file;

# Same with nbspd.init, which is needed for nbspd.status
set nbspd_init_file [file join $common(libdir) nbspd.init];
if {[file exists $nbspd_init_file] == 0} {
    puts stderr "$nbspd_init_file not found.";
    return 1;
}
source $nbspd_init_file;
unset nbspd_init_file;

array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$argc > 1} {
    puts stderr $usage;
    exit 1;
} elseif {$argc == 1} {
    set datafile [lindex $argv 0];
} else {
    set datafile $nbspd(statusfile);
}

set body [split [string trimright [exec cat $datafile]] "\n"];

set seconds [clock seconds];
if {$option(D) eq ""} {
    set mday [clock format $seconds -format "%d"];
} else {
    set mday $option(D);
}

if {[regexp {^-} $option(h)]} {
    set seconds [expr $seconds + $option(h) * 3600];
    set option(h) [clock format $seconds -format "%H"];
}

foreach line $body {
    set fields [split $line];
    set seconds [lindex $fields 0];
    set dd [clock format $seconds -format "%d"];
    set HH [clock format $seconds -format "%H"];

    if {$dd != $mday} {
	continue;
    }

    if {($option(h) ne "") && ([string compare $HH $option(h)] == -1)} {
	continue;
    }

    if {$option(m) == 0} {
	# Hourly data
	set hh $HH;
    } else {
	# Minute data
	set hh [clock format $seconds -format "%H:%M"];
    }

    if {[info exists frames_rcv($hh)]} {
	incr frames_rcv($hh) [lindex $line 1];
    } else {
	set frames_rcv($hh) [lindex $line 1];
    }

    if {[info exists frames_jumps($hh)]} {
	incr frames_jumps($hh) [lindex $line 3];
    } else {
	set frames_jumps($hh) [lindex $line 3];
    }

    if {[info exists files_tx($hh)]} {
	incr files_tx($hh) [lindex $line 6];
    } else {
	set files_tx($hh) [lindex $line 6];
    }

    if {[info exists files_rtx($hh)]} {
	incr files_rtx($hh) [lindex $line 9];
    } else {
	set files_rtx($hh) [lindex $line 9];
    }

    if {[info exists files_missed($hh)]} {
	incr files_missed($hh) [lindex $line 8];
    } else {
	set files_missed($hh) [lindex $line 8];
    }

    if {[info exists bytes($hh)]} {
	set bytes($hh) [expr $bytes($hh) + [lindex $line 5]/1000000];
    } else {
	set bytes($hh) [expr [lindex $line 5]/1000000];
    }
}

set result [list];
foreach hh [lsort [array names bytes]] {
    lappend result "$hh $frames_rcv($hh) $frames_jumps($hh) \
        $files_tx($hh) $files_rtx($hh) $files_missed($hh) $bytes($hh)";
}

if {$option(b) ne ""} {
    cd $option(b);
}

if {$option(d) ne ""} {    
    file mkdir $option(d);
    cd $option(d);
}

if {$option(o) eq ""} {
    puts [join $result "\n"];
    return;
}

set outputfile $option(o);
set status [catch {
    ::fileutil::writeFile $outputfile [join $result "\n"];
} errmsg];

if {$status != 0} {
    puts $errmsg;
    exit 1;
}
