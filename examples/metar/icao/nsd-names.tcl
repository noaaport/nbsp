#!/usr/local/bin/tclsh8.4
#
# $Id$
#
# This file is adapted from the nsd.awk file in the examples directory
# of the Nbsp distribution.
#
# The command
#
#	nsd.tcl < nsd_cccc
#
# creates the ".def" file used in the weatherscope-extension.

proc convert_minsec {str} {

    set d "";;
    set m "";
    set s "";

    set direction [string range $str end end];
    set str [string range $str 0 end-1];

    set parts [split $str "-"];
    set d [string trim [string trimleft [lindex $parts 0] 0]];

    if {[string length $parts] > 1} {
	set m [string trim [string trimleft [lindex $parts 1] 0]];
    }

    if {[string length $parts] > 2} {
	set s [string trim [string trimleft [lindex $parts 2] 0]];
    }
	
    if {$d eq ""} {
	set d 0;
    }

    if {$m eq ""} {
	set m 0;
    }

    if {$s eq ""} {
	set s 0;
    }

    set r [format "%.4f" [expr $d + $m/60.0 + $s/3600.0]];
    if {[regexp {W|S} $direction]} {
	set r "-$r";
    }

    return $r;
}

while {[gets stdin line] > 0} {

    if {[regexp {^#} $line]} {
	continue;
    }
    set p [split $line ";"];

    set icao [string tolower [lindex $p 0]];
    set block_number [lindex $p 1];
    set station_number [lindex $p 2];
    set place_name [lindex $p 3];
    set state [lindex $p 4];
    set country [lindex $p 5];
    set wmo_region [lindex $p 6];
    set lat [lindex $p 7];
    set lon [lindex $p 8];
    set elev [lindex $p 11];

    set lat_dec [convert_minsec $lat];
    set lon_dec [convert_minsec $lon];

    if {$icao ne ""} {
	puts -nonewline "set weatherscope(metar,sitename,$icao) ";
	puts "\"$place_name\"";
    }
}
