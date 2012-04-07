#!%TCLSH%
#
# $Id$
#
# Example: test-rad.tcl jua n0r
#
# Test of gempak::append_map for setting the map attributes, and mapfil param.
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

gempak::define radfil $datafile;
gempak::define proj "rad";
gempak::define device "gif|${type}${site}.gif";
gempak::define garea "dset";

gempak::imcbar_color 5;
gempak::imcbar_orientation "V";
gempak::imcbar_anchor "LL";
gempak::imcbar_xy 0.05 0.1
gempak::imcbar_lengthwidth 0.5 0.1;
gempak::imcbar_frequency 1;
gempak::set_imcbar;

# No "$" or "+" signgs
gempak::mapfil "county" "cities";

#
# Define the equivalent of "map = = 1//2 + 2//3" for the mapfil using
#
# gempak::append_map
#

# first one
gempak::map_color 1;
gempak::map_width 2;
gempak::set_map

# second one
gempak::map_color 2;
gempak::map_width 3;
gempak::append_map

gempak::init gpmap;
gempak::run;
gempak::end;
