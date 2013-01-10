#!/usr/local/bin/tclsh8.4
#
# $Id$
#
# Example: test-sat.tcl tip02
#
set ginidir "/var/noaaport/data/digatmos/sat/gini";

# To test without installing uncomment the nextline
lappend auto_path "..";
package require gempak;

set usage "$argv0 <type>";
if {$argc != 1} {
    puts stderr $usage;
    exit 1;
}
set type [lindex $argv 0];

set datafile [file join $ginidir $type "latest"];
if {[file exists $datafile] == 0} {
    puts stderr "$datafile not found.";
    exit 1;
}

gempak::init gpmap;

gempak::define satfil $datafile;
gempak::define proj "sat";
gempak::define device "gif|${type}.gif";
gempak::define garea "dset";

gempak::run;
gempak::end;
