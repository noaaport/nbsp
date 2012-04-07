#
# $Id$
#

proc proc_metar_stringtoarray {str} {

    set strlist [split $str ","]
    set n [llength $strlist]
    set i 0
    while {$i < $n} {
	set j [expr $i + 1]
	set a($j) [lindex $strlist $i]
	set i $j
    }
    set a(0) $str

    return [array get a]
}

proc display_stations {dir} {

    # Get first the names of the collective subdirectories.
    set cllist [lsort [glob -directory $dir -nocomplain -tails "*"]];
    if {[llength $cllist] == 0} {
	set result "No reports.<br/>";
	return $result;
    }

    set result "<h3>Metar Observations by Station</h3>\n";

    foreach cname $cllist {
	set subdir [file join $dir $cname];
	set stlist [lsort [glob -directory $subdir -nocomplain -tails "*"]];

	if {[llength $stlist] == 0} {
	    continue;
	}

	append result "<h4>$cname</h4>\n";

	foreach f $stlist {
	    set name [file rootname $f];
	    append result "<a href=\"station\?station=$name\">$name</a>" " ";
	}
    }
    return $result;
}

proc display_collectives {dir} {

    set stlist [lsort [glob -directory $dir -nocomplain -tails "*"]];
    if {[llength $stlist] == 0} {
	set result "No reports.<br/>";
	return $result;
    }

    set result "<h3>Metar Observations by Collective</h3>\n";

    foreach name $stlist {
	append result "<a href=\"collective?collective=$name\">$name</a>" " ";
    }
    return $result;
}

proc display_metarplots {htdocsdir metarplotsubdir station} {

    set basedir [file join $htdocsdir $metarplotsubdir];
    set plotsubdir [file join "plots" $station];

    set status [catch {
	exec nbspmtrplot -b $basedir -d $plotsubdir -f png $station;
    } errmsg];

    if {$status != 0} {
	return "";
    }

    set result "<a href=\"plotdata?station=$station\"><h4>Data</h4></a>";

    # Create the code for the output page
    set baseurl [file join / $metarplotsubdir $plotsubdir];
    append result "<img src=\"[file join $baseurl pre.png]\">\n";
    append result "<img src=\"[file join $baseurl temp.png]\">\n";
    append result "<img src=\"[file join $baseurl wspeed.png]\">\n";
    append result "<img src=\"[file join $baseurl wdir.png]\">\n";

    return $result;
}
