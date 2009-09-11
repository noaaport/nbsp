#!/usr/local/bin/tclsh8.4
#
# Used to create uafilter-siteloc.def
#
# http://www.ncdc.noaa.gov/oa/climate/igra/index.php?name=coverage
# http://www1.ncdc.noaa.gov/pub/data/igra/igra-stations.txt
# http://www1.ncdc.noaa.gov/pub/data/igra/readme.txt
#
package require textutil;

puts {#
# $Id$
#
# This file was created with
#
# igra.tcl < igra-stations.txt
#
# http://www.ncdc.noaa.gov/oa/climate/igra/index.php?name=coverage
# http://www1.ncdc.noaa.gov/pub/data/igra/igra-stations.txt
# http://www1.ncdc.noaa.gov/pub/data/igra/readme.txt
#
# The index is the <station_number>, and the values are
# <lat>,<lon>,<elev>,<country_code>
#}

foreach line [split [exec cat igra-stations.txt] "\n"] {
    set line1 [string trim [string range $line 0 45]];
    set line2 [string trim [string range $line 47 end]];
    set parts1 [::textutil::splitx $line1];
    set parts2 [::textutil::splitx $line2];

    set country_code [lindex $parts1 0];
    set stnm [lindex $parts1 1];

    set lat [lindex $parts2 0];
    set lon [lindex $parts2 1];
    set elev [lindex $parts2 2];
    set first_year_rec [lindex $parts2 end-1];
    set last_year_rec [lindex $parts2 end];
    puts "set uafilter(siteloc,$stnm) \"$lat,$lon,$elev,$country_code\";";
}
