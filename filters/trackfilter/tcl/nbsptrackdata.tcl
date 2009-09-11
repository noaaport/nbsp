#!%TCLSH%
#
# $Id$
#
# usage: nbsptrackdata.tcl [-o <outputfile>] [inputfile]
#
# This script reads from the file (or stdin), looking for lines of the form
#
# ''TROPICAL DEPRESSION CENTER LOCATED NEAR 12.0N  59.9W AT 01/0300Z''
# ``ESTIMATED MINIMUM CENTRAL PRESSURE 1008 MB''
# ``MAX SUSTAINED WINDS  30 KT WITH GUSTS TO  40 KT.''
# ``FORECAST VALID 01/1200Z 12.5N  62.2W''
# 
# It then extracts the position, and prints it, to stdout, in the form
#
# T 29/2100Z 32.7W 9.8N <pressure> <wind> <gusts>
# T 29/1500Z 32.2W 9.6N <pressure> <wind> <gusts>
# T 30/0300Z 33.7W 9.7N <pressure> <wind> <gusts>
# F 01/1200Z 62.2W 12.5N
#
# but without the trailing Z, W, N, suitable for input to a plotting program.
# T lines indicate track positions, F mean forecast.

package require cmdline;
package require fileutil;

proc err {s} {
    
    global argv0;

    puts stderr "$argv0: $s";
    exit 1;
}

set usage {usage: nbsptrackdata.tcl [-o <outputfile>] [inputfile]};
set optlist {{o.arg ""}};
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

set inputfile "";
set outputfile $option(o);
if {$argc == 1} {
    set inputfile [lindex $argv 0];
} elseif {$argc > 1} {
    err $usage;
}

set months(JUN) "06";
set months(JUL) "07";
set months(AUG) "08";
set months(SEP) "09";
set months(OCT) "10";
set months(NOV) "11";

if {$inputfile eq ""} {
    fconfigure stdin -translation binary -encoding binary;
    set body [split [string trimright [read stdin]] "\n"];
} else {
    set body [split [string trimright [::fileutil::cat \
	-translation binary -encoding binary $inputfile]] "\n"];
}

foreach line $body {

    if {[regexp {^\s*$} $line]} {
	continue;
    }

    if {[regexp {^TCM\S{3}\s*$} $line]} {
        if {[array exists fcast_data]} {
            array unset fcast_data;
        }
    }

   if {[regexp {^\d{4}\s+UTC\s+[A-Z]{3}\s+([A-Z]{3})\s+\d{2}\s+\d{4}\s*$} \
	$line s s1]} {
	set month_name $months($s1);
    }

    if {[regexp {CENTER LOCATED} $line] && ![regexp {REPEAT} $line]} {
	# The data comes after "NEAR". Split on NEAR, keep the second part,
	# and split that on blank, getting rid of trailing Z, W, N.
	if {[regexp {.+NEAR(.+)} $line match a]} {
	    regexp {(\S+)(N|S)\s+(\S+)(W|E)\s+AT\s+(\S+)Z} [string trim $a] \
		match lat lat_sign long long_sign date;
	    
	    if {$lat_sign eq "S"} {
		set lat -$lat;
	    }

	    if {$long_sign eq "W"} {
		set long -$long;
	    }

	    # our (x, y) data, indexed by the date/time
	    set d "$month_name/$date";
	    set track_data($d) "$long $lat";
	}
    } elseif {[regexp {ESTIMATED MINIMUM CENTRAL PRESSURE\s+(\S+)\s+MB} $line \
		  match barom]} {
	append track_data($d) " " $barom;
    } elseif {[regexp {MAX SUSTAINED WINDS\s+(\S+).+GUSTS TO\s+(\S+)} $line \
			  match wind gust]} {	
	append track_data($d) " " $wind " " $gust;
    }

    if {[regexp \
           {^FORECAST VALID\s+(\S+)Z\s+(\d+\.?\d*)(N|E)\s+(\d+\.?\d*)(W|E)} \
		$line match date lat lat_sign long long_sign]} {

	if {$lat_sign eq "S"} {
	    set lat -$lat;
	}

	if {$long_sign eq "W"} {
	    set long -$long;
	}

	# our (x, y) data, indexed by the date/time
	set d "$month_name/$date";
	set fcast_data($d) "$long $lat";
    }
}

set data [list];
foreach key [lsort [array names track_data]] {
    lappend data [join [list "T" $key $track_data($key)] " "];
}
foreach key [lsort [array names fcast_data]] {
    lappend data [join [list "F" $key $fcast_data($key)] " "];
}
if {$outputfile eq ""} {
    puts stdout [join $data "\n"];
} else {
    # This creates the parent subdirectories.
    ::fileutil::writeFile $outputfile [join $data "\n"];
}
