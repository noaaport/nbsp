#!/usr/local/bin/tclsh8.4
#
# $Id$
#
# Example: test-rad-3.tcl jua n0r
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
gempak::define device "gif|${type}${site}.gif";
gempak::define garea "dset";

gempak::imcbar_color 5;
gempak::imcbar_orientation "V";
gempak::imcbar_anchor "LL";
gempak::imcbar_xy 0.05 0.1
gempak::imcbar_lengthwidth 0.5 0.1;
gempak::imcbar_frequency 1;
gempak::set_imcbar;

gempak::run;
gempak::end;
