#
# $Id$
#

#
# Functions to support rss and station catalog
#

# Read the station definitions. This loads the arrays rssstation(), rssstate()
# and rsscollective in the global context, but they are used only by the
# functions in this file.
#
if {[file exists $Config(nbsprsswfodef)]} {
    source $Config(nbsprsswfodef);
}

if {[file exists $Config(nbsprssstconf)]} {
    source $Config(nbsprssstconf);
}

proc nbsprss_station_catalog {} {

    global rsscollective;

    set result "";

    foreach coll [lsort [array names rsscollective]] {
	append result "<h3>$coll</h3>\n";
	set stationlist [split $rsscollective($coll) "|"];
	foreach station $stationlist {
	    append result \
	    "<a href=\"/nbsprss/received_by_station?station=$station\">$station</a>\n";
	}
    }
    
    return $result;
}

proc nbsprss_received_by_station {station rssdir rssfext} {
#
# List the feeds from a given station
#
    global rssstation;

    set flist [glob -nocomplain -tails -directory $rssdir \
		   *${station}${rssfext}];
    if {[llength $flist] == 0} {
	set result "No files available.\n";

	return $result;
    }

    append result "<h3>Feeds from " [string toupper $station] "</h3>\n";
    if {[info exists rssstation($station)]} {
	append result "<h4>" $rssstation($station) "</h4>\n";
    }
    append result "<ul>\n";
    foreach f [lsort $flist] {
	set rsschannel [file rootname $f];
	set href "<a href=\"/_get/rss/$rsschannel\">$rsschannel</a>";
	append result "<li>$href</li>\n";
    }
    append result "</ul>\n";

    return $result;
}
