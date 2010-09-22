#!/usr/local/bin/tclsh8.6
#
# ./cvt-nexrad-stations.tcl nexrad-stations.txt > nexrad-stations.csv
#
package require textutil::split;


puts -nonewline "\# ncdcid";
foreach k [list icao wban name state county lat lon elev] {
    puts -nonewline ",$k";
}
puts ",time";

set data [split [exec cat "tmp"] "\n"];
foreach line $data {
    set line [string tolower $line];
    set p1 [string trim [string range $line 0 50];]
    set p2 [string trim [string range $line 50 70]];
    set p3 [string trim [string range $line 70 100]];
    set p4 [string trim [string range $line 100 end]];
    
    set plist [::textutil::split::splitx $p1];
    set ncdcid [lindex $plist 0];
    set icao [lindex $plist 1];
    set wban [lindex $plist 2];
    set name [join [lrange $plist 3 4] " "];
    
    set country $p2;
    if {$country ne "united states"} {
	continue;
    }

    set plist [::textutil::split::splitx $p3];
    set state [lindex $plist 0];
    set county [join [lrange $plist 1 end] " "];

    set plist [::textutil::split::splitx $p4];
    foreach {lat lon elev time stntype} $plist {};

    puts -nonewline $ncdcid;
    foreach k [list icao wban name state county lat lon elev] {
	puts -nonewline ",[set $k]";
    }
    puts ",$time";
}
