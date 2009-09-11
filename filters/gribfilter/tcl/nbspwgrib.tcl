#!%TCLSH%
#
# $Id$
#
# Usage: nbspwgrib [wgrib or wgrib2 options] <grib1 or grib2 file>
#
# Uses nbspgrib to determine the "grib edition" and then calls wgrib or wgrib2
# accordingly.

if {[llength $argv] == 0} {
    puts {usage: nbspwgrib [wgrib options] <file>};
    exit 1;
}
set file [lindex $argv end];

set edition [lindex [exec nbspgrib -m $file] 0];

if {$edition == 1} {
    set output [eval exec wgrib $argv];
} elseif {$edition == 2} {
    set output [eval exec wgrib2 $argv];
} else {
    puts "Unsupported grib edition $edition.";
    exit 1;
}
puts $output;
