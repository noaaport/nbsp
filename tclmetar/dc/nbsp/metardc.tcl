#!%TCLSH%
#
# Copyright (c) 2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
#
# See LICENSE
#
# $Id$

# Usage: metardc [-c | -d] [-h | [-t [-k]]] [-l location] [-o outputfile]
#		[-s recseparator] [-e obdata | inputfile];
#
# The input must consist of just pure data, assumed to have been
# checked and cleaned, one data record per line in inputfile or stdin,
# or a single record in [-e].  With [-c] each line is assumed to be a record
# of the type emitted by nbspmtrcsv. Otherwise, each line must
# start with one of the words "METAR", "SPECI", "M" or "S",
# unless [-d] is given which indicates that the line is just the data portion
# (so that the type is not unknown). The output is either
# the full report, or with [-t] just the parameter values; in the latter
# case, [-k] will give the wind speed in knots instead of mph. If [-h]
# is given instead of [-t] the report is an html table. The output
# records can be separated by giving [-s], otherwise they are not separated;
# passing -s "" will separate them by a blank line.
# Output is to stdout unless [-o] is given. The [-l] option can specify
# the location information of the station that will be printed in the
# header of the report (only in the html version).

package require cmdline;
lappend auto_path %TCLMETAR_INSTALLDIR%;
package require metar;

set usage {Usage: metardc [-c | -d] [-h | [-t [-k]]] [-l location]
    [-o outputfile] [-s recseparator] [-e obdata | inputfile]};

set optlist {c d h t k {l.arg ""} {o.arg ""} {s.arg ""} {e.arg ""}};

set conflict_cd 0;
set conflict_e 0;
set conflict_ht 0;
set inputfile "";

proc print_report {fout line} {

    global option;

    metar::decode $line;

    puts $fout [format "Obs: %s" $line];

    puts $fout [format "Site: %s on %sth at %s UTC - %s" \
		    $::metar::metar(obs,STATION) \
		    $::metar::metar(param,date.dd) \
		    $::metar::metar(param,date.hhmm) \
		    $::metar::metar(param,type)];

    if {$::metar::metar(flag,wind_calm) == 1} {
	puts $fout [format "Wind: %s" $::metar::metar(text,wind_calm)];
    } elseif {$::metar::metar(param,wind.speed_mph) ne ""} {
	puts $fout [format "Wind: %s Mph at %s" \
			$::metar::metar(param,wind.speed_mph) \
			$::metar::metar(param,wind.dir)];
    }

    if {$::metar::metar(param,wind.gust_mph) ne ""} {
	puts $fout [format "Gust: %s Mph" \
			$::metar::metar(param,wind.gust_mph)];
    }

    if {$::metar::metar(param,temp_f) ne ""} {
	puts $fout [format "Temp: %s F" $::metar::metar(param,temp_f)];
    }

    if {$::metar::metar(param,dewp_f) ne ""} {
	puts $fout [format "Dewp: %s F" $::metar::metar(param,dewp_f)];
    }

    if {$::metar::metar(flag,alt_Q) == 0} {
	if {$::metar::metar(param,alt_hg) ne ""} {
	    puts $fout [format "Pressure: %s inHg" \
			    $::metar::metar(param,alt_hg)];
	}
    } else {
	if {$::metar::metar(param,alt_mb) ne ""} {
	    puts $fout [format "Pressure: %s mb" \
			    $::metar::metar(param,alt_mb)];
	}
    }

    if {$::metar::metar(param,slp) ne ""} {
	puts $fout [format "Sea level pressure: %s mb" \
			$::metar::metar(param,slp)];
    }

    if {$::metar::metar(param,visibility) ne ""} {
	puts $fout [format "Visibility: %s Statute Miles" \
			$::metar::metar(param,visibility)];
    }

    if {[llength $::metar::metar(param,weather)] != 0} {
	puts $fout [format "Weather: %s" \
			[join $::metar::metar(param,weather) ", "]];
    }
    
    if {$::metar::metar(param,sky) ne ""} {
	puts $fout [format "Sky: %s" [join $::metar::metar(param,sky) ", "]];
    }

    if {[llength $::metar::metar(param,weatherlog)] != 0} {
	puts $fout [format "Weather log: %s" \
			[join $::metar::metar(param,weatherlog) ", "]];
    }

    if {$::metar::metar(flag,presfr) == 1} {
	puts $fout [format "Remarks: %s" $::metar::metar(text,presfr)];
    }

    if {$::metar::metar(flag,presrr) == 1} {
	puts $fout [format "Remarks: %s" $::metar::metar(text,presrr)];
    }

    if {$::metar::metar(flag,snincr) == 1} {
	puts $fout [format "Remarks: %s" $::metar::metar(text,snincr)];
    }

    if {$option(s) ne ""} {
	puts $option(s);
    }
}

proc print_tabular {fout line} {

    global option

    metar::decode $line;

    # Use csv format because it is easier to spot any missing value.
    # The UTC part of the time is not printed.

    if {$option(k) == 0} {
	set wind $::metar::metar(param,wind.speed_mph);
    } else {
	set wind $::metar::metar(param,wind.speed_kt);
    }

    puts $fout [format "%s,%s,%s,%s,%s,%s,%s,%s,%s" \
		    $::metar::metar(param,station) \
		    $::metar::metar(param,date.dd) \
		    $::metar::metar(param,date.hhmm) \
		    $wind \
		    $::metar::metar(param,wind.dir) \
		    $::metar::metar(param,temp_f) \
		    $::metar::metar(param,dewp_f) \
		    $::metar::metar(param,alt_hg) \
		    $::metar::metar(param,slp)];
}

proc print_report_html {fout line} {

    global option;

    metar::decode $line;

    set fmt "<tr><td>%s</td><td>%s</td></tr>\n";

    append _header $::metar::metar(param,type) " Report for " \
	$::metar::metar(obs,STATION);
    if {$option(l) ne ""} {
	append _header " at " $option(l);
    }
    append result "<h3>" ${_header} "</h3>\n";
    
    append result "<table border>\n";
    
    append result [format $fmt "Obs" $line];
    append result [format $fmt "Date/Time" \
     "$::metar::metar(param,date.dd) at $::metar::metar(param,date.hhmm) UTC"];
    
    if {$::metar::metar(flag,wind_calm) == 1} {
	append result [format $fmt "Wind" $::metar::metar(text,wind_calm)];
    } elseif {$::metar::metar(param,wind.speed_mph) ne ""} {
        append result [format $fmt "Wind" \
       "$::metar::metar(param,wind.speed_mph) Mph at $::metar::metar(param,wind.dir)"];
    }

    if {$::metar::metar(param,wind.gust_mph) ne ""} {
	append result [format $fmt "Gust" \
			   "$::metar::metar(param,wind.gust_mph) Mph"];
    }

    if {$::metar::metar(param,temp_f) ne ""} {
	append result [format $fmt "Temp" "$::metar::metar(param,temp_f) F"];
    }
    if {$::metar::metar(param,dewp_f) ne ""} {
	append result [format $fmt "Dewp" "$::metar::metar(param,dewp_f) F"];
    }

    if {$::metar::metar(param,alt_hg) ne ""} {
	append result [format $fmt "Pressure" \
			   "$::metar::metar(param,alt_hg) inHg"];
    }

    if {$::metar::metar(param,slp) ne ""} {
	append result [format $fmt "Sea level pressure" \
			   "$::metar::metar(param,slp) mb"];
    }

    if {$::metar::metar(param,visibility) ne ""} {
	append result [format $fmt "Visibility" \
			"$::metar::metar(param,visibility) Statute Miles"];
    }

    if {[llength $::metar::metar(param,weather)] != 0} {
	append result [format $fmt "Weather" \
			[join $::metar::metar(param,weather) ", "]];
    }
    
    if {$::metar::metar(param,sky) ne ""} {
	append result [format $fmt "Sky" \
			   [join $::metar::metar(param,sky) ", "]];
    }

    if {[llength $::metar::metar(param,weatherlog)] != 0} {
	append result [format $fmt "Weather log" \
			[join $::metar::metar(param,weatherlog) ", "]];
    }

    if {$::metar::metar(flag,presfr) == 1} {
	append result [format $fmt "Remarks" $::metar::metar(text,presfr)];
    }

    if {$::metar::metar(flag,presrr) == 1} {
	append result [format $fmt "Remarks" $::metar::metar(text,presrr)];
    }

    if {$::metar::metar(flag,snincr) == 1} {
	append result [format $fmt "Remarks" $::metar::metar(text,snincr)];
    }

    append result "</table>";
    puts $fout $result;
}

proc process_line {fout line} {

    global option;

    if {$option(c) == 1} {
	# Split on commad and then join the last two elements
	set elements [split $line ","];
	set line [join [lreplace $elements 0 end-2]];
    }

    if {$option(t) == 1} {
	print_tabular $fout $line;
    } elseif {$option(h) == 1} {
	print_report_html $fout $line;
    } else {
	print_report $fout $line;
    }
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

# Check for conflicting options
if {$argc == 1} {
    incr conflict_e
    set inputfile [lindex $argv 0];
}
if {$option(e) ne ""} {
    incr conflict_e;
}

if {$option(c) != 0} {
    incr conflict_cd;
}

if {$option(d) != 0} {
    incr conflict_cd;
}

if {$option(h) != 0} {
    incr conflict_ht;
}

if {$option(t) != 0} {
    incr conflict_ht;
}

if {($conflict_cd > 1) || ($conflict_e > 1) || ($conflict_ht > 1)} {
    puts $usage;
    exit 1;
}

if {$inputfile ne ""} {
    set status [catch {
	set fin [open $inputfile r];
    } errmsg];
    if {$status != 0} {
	puts $errmsg;
	exit 1;
    }
} else {
    set fin stdin;
}

if {$option(o) ne ""} {
    set status [catch {
	set fout [open $option(o) w];
    } errmsg];
    if {$status != 0} {
	puts $errmsg;
	exit 1;
    }
} else {
    set fout stdout;
}

if {$option(e) ne ""} {
    process_line $fout $option(e);
} else {
    while {[gets $fin line] > 0} {
	process_line $fout $line;
    }
}

if {$inputfile ne ""} {
    set status [catch {
	close $fin;
    } errmsg];
    if {$status != 0} {
	puts $errmsg;
    }
}

if {$option(o) ne ""} {
    set status [catch {
	close $fout;
    } errmsg];
    if {$status != 0} {
	puts $errmsg;
    }
}
