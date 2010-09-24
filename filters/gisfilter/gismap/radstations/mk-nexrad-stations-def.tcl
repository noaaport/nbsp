#!/usr/local/bin/tclsh8.6

set data [split [exec cat nexrad-stations.csv] "\n"];
foreach line $data {
    if {[regexp {^\#} $line]} {
	continue;
    }
    set parts [split $line ","];
    set ssss [lindex $parts 1];
    set sss [string range $ssss 1 3];
    set state [lindex $parts 4];

    set parts [lreplace $parts 0 0];
    set parts [lreplace $parts 1 1];

    set line [join $parts ","];
    lappend a($state) $line;
    lappend b($state) $sss;
}
 
foreach state [lsort [array names a]] {
    
    foreach line $a($state) {
	set parts [split $line ","];
	set ssss [lindex $parts 0];
	set sss [string range $ssss 1 3];

	puts "set nexrad_stations(site,$sss) \\"
	puts "  \"$line\";";
    }
}

foreach state [lsort [array names b]] {
    puts "set nexrad_stations(state,$state) [join $b($state) ","];";
}
