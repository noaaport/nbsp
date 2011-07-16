#!%TCLSH%
#
# $Id$
#
# Convert a station daily file to mts.
#
# Usage: metartomts [-f] [-d <dir>] [-o <outputfile>] \
#                   [-u <undef>] [-y] [-Y <Y>] <name>
#
#         <name> is interpreted as a <icao>. Then the file in
#         "archive/<ymd>/daily" for that <stid> is processed.
#         The defaults is to use the <ymd> for the current day,
#         except as modified by -y:
#
# [-y] => Use the previous day or <Y> days before if "-Y" is given.
# [-f] => Take <name> as the inputfile name as is.
# [-d] => Write outputfile in the given directory.
#         By default output is to a file with the same name as the input file
#         but with a "mdf" extension, in the weatherscope data dir.
# [-o] => Name of outputfile. "-" for stdout.
#
# nbspmtrd output the records in the format
#
# <station>,<dd>,<hhmm>,<wspeed(mph)>,<wdir>,<tempf>,<dewpf>,<alt(inHg)>,<slp>
#
# This script translates them to the form
#
# <station> <stnm> <time=h*60> <wind(m/s)> <wdir> <tempc> <dewpc> <alt(mb)> \
#                              <lat> <lon> <elev>
#

## The common defaults
set defaults_file "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaults_file] == 0} {
    puts "wsfilter disabled: $defaults_file not found.";
    return 1;
}
source $defaults_file;
unset defaults_file;

## The common tools initialization
set wsfilter_initfile [file join $common(libdir) "wsfilter.init"];
if {[file exists $wsfilter_initfile] == 0} {
    puts "$wsfilter_initfile not found.";
    return 1;
}
source $wsfilter_initfile;
unset wsfilter_initfile;

# The metar siteloc file
if {[file exists $wsfilter(metar_siteloc)]} {
    source $wsfilter(metar_siteloc);
}

package require cmdline;
package require fileutil;

set usage {metartomdf [-d <dir>] [-f] [-o <outputfile>] [-u <undef>]
    [-y] [-Y <Y>] [<name>]};
set optlist {{d.arg ""} {f} {o.arg ""} {u.arg ""} {y} {Y.arg "1"}};
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

# Defaults unless given in cmd line
set g(undef) $wsfilter(metar_undef);

#
# Functions
#
proc metartomdf_daily {ymd inputfile undef} {

    global wsfilter;

    if {[regexp {(\d{4})(\d{2})(\d{2})} $ymd match \
	     arg_yyyy arg_mm arg_dd] == 0} {
        return -code error "Invalid date.";
    }

    set ws_result_list [list];	# what will be returned
    # Write header
    lappend ws_result_list "  101";
    lappend ws_result_list "  8 $arg_yyyy $arg_mm $arg_dd 00 00 00";
    lappend ws_result_list \
            " stid stnm time wspd wdir tair tdew pmsl lat lon elev";

    # Sort the list to delete duplicates entries for the same hour.
    # (The list is sorted on the time => -k 3).
    set status [catch {
	# Pass -t for tabular and -d to indicate type of record in file
	set result [exec nbspmtrd -t -d $inputfile | sort -k 3 -t ","];
    } errmsg];
    if {$status != 0} {
        return -code error $errmsg;
    }
    
    set current "";
    foreach r [split $result "\n"] {

	set p [split $r ","];

	set station [lindex $p 0];
	set hhmm [lindex $p 2];
	set hh [lindex [split $hhmm ":"] 0];

	if {$hh eq $current} {
	    continue;
	}
	set current $hh;

	set dd [lindex $p 1];
	# set hhmm [lindex $p 2];
	set wspeed_mph [lindex $p 3];
	set wdir [lindex $p 4];
	set tempf [lindex $p 5];
	set dewpf [lindex $p 6];
	set alt_inhg [lindex $p 7];
	set slp_mb [lindex $p 8];

	# I have seen empty records like "dxxx,,,,,,", so we check everything
	if {$station eq ""} {
	    continue;
	}

	if {[regexp {(\d{2}):(\d{2})} $hhmm match hh mm] == 0} {
	    continue;
	}

	if {$dd ne $arg_dd} {
	    continue;
        }

	set wspeed_mtps $undef;
	set tempc $undef;
	set dewpc $undef;
	set alt_mb $undef;

	if {$wspeed_mph ne ""} {
	    # change to meters/second
	    set wspeed_mtps [format "%.2f" [expr $wspeed_mph * 0.447]];
	}

	if {$wdir eq ""} {
	    set wdir $undef;
	}

	if {$tempf ne ""} {
	    set tempc [format "%.2f" [expr ($tempf - 32.0) * 0.55]];
	}

	if {$dewpf ne "" } {
	    set dewpc [format "%.2f" [expr ($dewpf - 32.0) * 0.55]];
	}

	if {$alt_inhg ne ""} {
	    set alt_mb [expr $alt_inhg * 33.865];
	}

	set hour [wsfilter_strip_zeros $hh];
	set time [expr $hour * 60];

	if {[info exists wsfilter(metar,siteloc,$station)] == 0} {
	    set lat $undef;
	    set lon $undef;
	    set elev $undef;
	} else {
	    set latlonelev [split $wsfilter(metar,siteloc,$station) ","];
	    set lat [lindex $latlonelev 0];
	    set lon [lindex $latlonelev 1];
	    set elev [lindex $latlonelev 2];
	}

	lappend ws_result_list \
	        "$station 000 $time $wspeed_mtps $wdir $tempc $dewpc $alt_mb $lat $lon $elev";
    }

    return [join $ws_result_list "\n"];
}

#
# main
#
if {$argc == 0} {
    puts $usage;
    exit 1;
}

if {$option(u) ne ""} {
    set g(undef) $option(u);
}

# Even if [-f] is given, the date must be given separately since we do not
# assume a particular file name format. The default date is the current day
# except as modified by the -y option.
set seconds [clock seconds];
if {$option(y) == 1} {
    set seconds [expr $seconds - $option(Y)*24*3600];
}
set ymd [clock format [clock seconds] -format "%Y%m%d" -gmt true];

if {$option(f) == 1} {
    set g(inputfile) [lindex $argv 0];
    if {[file exists $g(inputfile)] == 0} {
	return -code error "$g(inputfile) not found.";
    }
} else {
    set stid [lindex $argv 0];
    set dir [file join $wsfilter(metar_basedir) $ymd \
		 $wsfilter(metar_daily_subdir)];
    # The file is inside a collective, si we must search for it.
    set flist [exec find $dir -name "*$stid*"];
    if {[llength $flist] == 0} {
	return -code error "$dir is empty.";
    }
    # There should be only one file
    set g(inputfile) [lindex $flist 0];
}

set fname [file tail [file rootname $g(inputfile)]];

if {$option(o) eq ""} {
    set fbasename $fname;
    append fbasename $wsfilter(metar_mts_fext);
    if {$option(d) ne ""} {
	set g(outputfile) [file join $option(d) [file tail $fbasename]];
    } else {
	set g(outputfile) [file join $wsfilter(ws_datadir) $ymd \
			       $wsfilter(ws_metardir) $fbasename];
    }
} else {
    set g(outputfile) $option(o);
    if {($g(outputfile) ne "-") && ($option(d) ne "")} {
	set g(outputfile) [file join $option(d) $option(o)];
    }
}

set status [catch {
    set result [metartomdf_daily $ymd $g(inputfile) $g(undef)];
} errmsg];

if {$status == 0} {
    set status [catch {
	if {$g(outputfile) ne "-"} {
	    # This will create the intermediate directories
	    ::fileutil::writeFile $g(outputfile) $result;
	} else {
	    puts $result;
	}
    } errmsg];
}

if {$status != 0} {
    puts $errmsg;
    exit 1;
}
