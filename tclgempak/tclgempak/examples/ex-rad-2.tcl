#!%TCLSH%
#
# $Id$
#
# Example: test-rad.tcl jua n0r
#
package require gempak;

# Uncomment this if necessary
source "/usr/local/etc/nbsp/gempak.env"

# Edit this is necessary
set nidsdir "/var/noaaport/data/digatmos/nexrad/nids";

# main
set usage "Usage: $argv0 <site> <type>";
if {$argc != 2} {
    puts stderr $usage;
    exit 1;
}
set site [lindex $argv 0];
set type [lindex $argv 1];

set datafile [file join $nidsdir $site $type "latest"];
if {[file exists $datafile] == 0} {
    puts stderr "$datafile not found.";
    exit 1;
}

gempak::init gpmap;

gempak::define radfil $datafile;
gempak::define proj "rad";
gempak::define device "gif|${type}${site}.gif";
gempak::define garea "dset";

gempak::map_color 1;
gempak::map_dash 7;
gempak::set_map;

gempak::run;
gempak::end;
