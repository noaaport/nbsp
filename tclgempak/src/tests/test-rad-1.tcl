#!/usr/local/bin/tclsh8.4
#
# $Id$
#
# Test gempak::device_xxxx functions
# 
# Example: test-rad.tcl jua n0r
#
set nidsdir "/var/noaaport/data/digatmos/nexrad/nids";

# To test without installing uncomment the nextline
lappend auto_path "..";
package require gempak;

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

gempak::init gpmap;

gempak::define radfil $datafile;
gempak::define proj "rad";

gempak::device_device "gif";
gempak::device_name ${type}${site}.gif;
gempak::device_size 1024 768;
gempak::set_device;

gempak::define garea "dset";

gempak::run;
gempak::end;
