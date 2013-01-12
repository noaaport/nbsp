#
# $Id$
#
Direct_Url /nbspmetar nbspmetar

# Read the Metar ICAO definitions. This loads the array metarfilter(icao,...)
# in the global context, but it is used only by the functions in this file.
#
if {[file exists $Config(nbspmetaricaodef)]} {
    source $Config(nbspmetaricaodef);
    foreach key [array names metarfilter "icao,*"] {
	set Config(nbspmetar,$key) $metarfilter($key);
    }
    unset metarfilter;
}

proc nbspmetar/display {} {

    global Config;

    set result [display_collectives $Config(nbspmetarcldir)];
    append result [display_stations $Config(nbspmetarstdir)];

    return $result;
}

proc nbspmetar/station {station} {

    global Config;

    set fmt [proc_fmtrow 2]

    set status [catch {
	set reclist [split \
			 [string trim [exec nbspmtr -a -r $station]] "\n"];
    } errmsg];
    if {$status != 0} {
	set reclist [list];
    }

    if {[llength $reclist] == 0} {
	set result "No reports for $station.";
	return $result;
    }

    set _loc [string toupper $station];
    if {[info exists Config(nbspmetar,icao,$station)]} {
	append _loc " " $Config(nbspmetar,icao,$station);
    }
    set result "<h3>Metar Observations ${_loc}</h3>\n";

    append result \
    [display_metarplots $Config(docRoot) $Config(nbspmetarplothtdir) $station];

    append result "<table border>\n";
    append result [format $fmt "type" ""];

    # We use an array "inserted" indexed by the observation datetime
    # to _try_ to detect duplicate reports. We take the first report
    # and discard the subsequent ones with the same datetime.
    # Chronologically, those subsequent reports arrived earlier
    # than the one we retain since we are processing the data in reverse
    # order.

    foreach record $reclist {
	array set a [proc_metar_stringtoarray $record];
	set obtype $a(9);
	set obdata $a(10);

	set obdatetime [lindex [split $obdata] 1];
	if {[info exists inserted($obdatetime)]} {
	    continue;
	}
	set inserted($obdatetime) 1;
	
	set enc_obdata [Url_Encode $obdata];
	set href "<a href=\"report?obstation=$station&obtype=$obtype&obdata=$enc_obdata\">$obdata</a>";
	append result [format $fmt $obtype $href];
    }
    append result "</table>\n";

    return $result;
}

proc nbspmetar/collective {collective} {

    set fmt [proc_fmtrow 2];

    set status [catch {
	set reclist [split \
			 [string trim [exec nbspmtr -a -r $collective]] "\n"];
    } errmsg];
    if {$status != 0} {
	set reclist [list];
    }

    if {[llength $reclist] == 0} {
	set result "No reports for $collective.";
	return $result;
    }

    set result "<h3>Metar Observations for $collective</h3>\n";
    append result "<table border>\n";
    append result [format $fmt "type" ""];

    foreach record $reclist {
	array set a [proc_metar_stringtoarray $record];
	set obstation [string tolower $a(1)];
	set obtype $a(9);
	set obdata $a(10);
	set enc_obdata [Url_Encode $obdata];
	set href "<a href=\"report?obstation=$obstation&obtype=$obtype&obdata=$enc_obdata\">$obdata</a>";
	append result [format $fmt $obtype $href];
    }
    append result "</table>\n";
    return $result
}

proc nbspmetar/report {obstation obtype obdata} {
#
# nbspmtrd is the tcl decoder that uses the tclmetar library. The command
# executed is
#		nbspmtrd -h -e $data -l $location
#
    global Config;

    append data $obtype " " $obdata;
    set _opts [list "-h" "-e" $data];

    if {[info exists Config(nbspmetar,icao,$obstation)]} {
	lappend _opts "-l" $Config(nbspmetar,icao,$obstation);
    }

    append result [eval exec nbspmtrd ${_opts}];    

    return $result;
}

proc nbspmetar/plotdata {station} {
#
# nbspmtrplotdat is the metarfilter tool that extracts the data from the
# MetarWeather files and decodes them by calling nbspmtrd.

    set output "<table border>\n";

    append output {<tr><th>Site</th><th>Day</th><th>Time</th>
	<th>Wind(mph)</th><th>Wdir(deg)</th><th>Temp(F)</th><th>DewP(F)</th>
	<th>Pre(inHg)</th><th>SLP(mb)</th><th>Pre(mb)</th><th>Hum</th>};

    set status [catch {
	set data [exec nbspmtrplotdat $station];
    } errmsg];

    if {$status != 0} {
	set output "<h3>Error</h3>: Could not get data for $station.";
	return $output;
    }

    set linelist [split $data "\n"];
    foreach line $linelist {
        append output "<tr>";
        set fields [split $line];
        foreach f $fields {
	    append output "<td>$f</td>";
        }
        append output "</tr>\n";
    }
    
    append output "</table>\n";

    return $output;
}
