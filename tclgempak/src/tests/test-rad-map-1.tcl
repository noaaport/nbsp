#!/usr/local/bin/tclsh8.4
#
# $Id$
#
# Example: test-rad-map.tcl jua n0r
#
# Test for joining map definitions
# 
set nidsdir "/var/noaaport/data/digatmos/nexrad/nids";

# To test without installing uncomment the nextline
#lappend auto_path "..";
#package require gempak;
source ../gempak.tcl

set usage "$argv0 <site> <type>";
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

# No need to add \$ to mapfil
gempak::define mapfil {county + cities};

#
# Define the equivalent of "map = = 1//2 + 2//3"
#

# Initialize the list
set maplist [list];

# first one
gempak::map_color 1;
gempak::map_width 2;
::gempak::set_map
lappend maplist [::gempak::get "map"];

# second one
gempak::map_color 2;
gempak::map_width 3;
::gempak::set_map
lappend maplist [::gempak::get "map"];

# join them with a "+" and set the "map" parameter to the result
gempak::define map [join $maplist "+"];

gempak::init gpmap;
gempak::run;
gempak::end;
