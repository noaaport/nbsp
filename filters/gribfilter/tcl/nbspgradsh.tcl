#!%EXPECT%
#
# $Id$
#
# Usage: nbspgradsh [-a | -I <n>] <model/key>
#
# Examples: nbspgradsh "gfs/*211*_6h*"
#           nbspgradsh "gfs/*211*"
#
package require cmdline;
package require fileutil;

set usage {Usage: nbspgradsh [-a | -I <n>] <model/key>};
set optlist {{a} {I.arg ""}}

## The common grib tools initialization
set initfile "/usr/local/libexec/nbsp/nbspgribtools.init";
if {[file exists $initfile] == 0} {
    puts "$initfile not found.";
    return 1;
}
source $initfile;
unset initfile;

package require nbsp::errx;

# Configuration
set grads(ctldir) [file join $gribfilter(datadir) $gribfilter(ctldatadir)];
set grads(prompt) $gribfilter(gradsprompt);

# main
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc != 1} {
    puts "Error: $usage";
    exit 1;
}
set _ctlfile [lindex $argv 0];

if {[file extension ${_ctlfile}] eq ""} {
    append _ctlfile $gribfilter(ctlfext);
}

# Check conflicting options
if {($option(a) != 0) && ($option(I) ne "")} {
    puts $usage;
    exit 1;
}

if {$option(I) eq ""} {
    set grads(ctlfile) [file join $grads(ctldir) ${_ctlfile}];
    if {[file exists $grads(ctlfile)] == 0} {
        # Retry (below) as a glob pattern
        set option(I) "end";
    }
}

if {$option(I) ne "" } {
    set _dir [file join $grads(ctldir) [file dirname ${_ctlfile}]];
    set _fname [file tail ${_ctlfile}];
 
    set _ctlflist [lsort -dictionary \
		       [split [exec find ${_dir} -name ${_fname}] "\n"]];

    if {[llength ${_ctlflist}] == 0} {
        ::nbsp::errx::err "${_ctlfile} not found.";
    } else {
        set grads(ctlfile) [lindex ${_ctlflist} $option(I)];
        # The index option(I) could be out of bounds.
        if {$grads(ctlfile) eq ""} {
            ::nbsp::errx::err "${_ctlfile} not found.";
        }
    }
}

if {$option(a) != 0} {
    foreach ctl ${_ctlflist} {
	puts [::fileutil::stripPath $grads(ctldir) $ctl];
    }
    exit 0;
}

spawn grads -l
expect $grads(prompt);
send "open $grads(ctlfile)\n";
expect $grads(prompt);
interact;

exit 0;
