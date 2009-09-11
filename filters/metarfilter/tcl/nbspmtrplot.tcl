#!%TCLSH%
#
# $Id$
#
# Usage: nbspmtrplot [-b basedir] [-d subdir] [-f <fmt>] [-g <fmtopt>] \
#                    <station>
#
# The tool will cd to the "basedir", create "subdir", and save
# the three plots in that subdir, with the names "pre".<fmt>,
# "temp".<fmt> and "wind".<fmt>. The <fmt> is whatever the tool
# nbspmtrplot uses as defined in metarfilter.conf file, or
# what is passed here in the [-f] option.

package require cmdline;
set usage {nbspmtrplot [-b basedir] [-d subdir]
    [-f <fmt>] [-g <fmtopt>] <station>};
set optlist {{b.arg ""} {d.arg ""} {f.arg ""} {g.arg ""}};

array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc != 1} {
    puts $usage;
    exit 1;
} else {
    set station [lindex $argv 0];
}

if {$option(b) != ""} {
    cd $option(b);
}

if {$option(d) != ""} {
    file mkdir $option(d);
    cd $option(d);
}

if {$option(f) != ""} {
    set fmtoption "-f $option(f)";
} else {
    set fmtoption "";
}

if {$option(g) != ""} {
    append fmtoption "-g $option(g)";
}

set status [catch {
    eval exec nbspmtrplot1 -p -o pre $fmtoption -d $station.dat $station;
    eval exec nbspmtrplot1 -t -o temp $fmtoption -i $station.dat $station;
    eval exec nbspmtrplot1 -w -o wind $fmtoption -k -i $station.dat $station;
} errmsg];

if {$status != 0} {
    puts $errmsg;
    exit 1;
}
