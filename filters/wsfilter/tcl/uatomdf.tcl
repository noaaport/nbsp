#!%TCLSH%
#
# $Id$
#
# Convert an upperair hourly file to mdf.
#
# Usage: uatomdf [-d <dir>] [-f] [-n <n>] [-o <outputfile>]
#                [-u <undef>] [-y] [-Y <Y>] [<name>]
#
#         If <name> is not given, process the (last) most recent
#         file in the standard directory "<upperair>/<ws>/ymd/hourly", except
#         as modified by "-n" and "-y":
# [-n]    Choose a different file (end, end-1, ...).
# [-y]    As above, but use the ymd for the previous day, or <Y> days before
#         if "-Y <Y>" is given.
#         If name is given, it is assumed to be a date yyyymmddhh and the file
#         yyyymmddhh.<fext> in the "archive/ymd/hourly" is processed.
# [-f]    Take <name> as the inputfile name as is.         
# [-d] => Write outputfile in the given directory.
#         By default output is to a file with the same name as the input file
#         but with a "mdf" extension, in the weatherscope data dir.
# [-o] => Name of outputfile. "-" for stdout.
#
#
# The function 
# 
# uatomdf_cvt {data separator na_val}
#
# returns the parameters in the following order, all in the same line,
#
# stid stnm time lat lon elev
# pres_x tair_x tdew_x wspd_x wdir_x      (x = sfc)
# pres_i hght_i tair_i tdew_i wspd_i wdir_i 
#
# with (i = 1000 925 850 700 500 400 300 250 200 150 100)
#
# The function
#
# uatomdf_header {}
#
# returns the variable names (for the header) as indicated above.
#
# The data should be in the format such as
#
# 42410,2112|surface,1000,26.4,-85.6,3,270|1000,54,26.4,-85.6,3,270
# 42410,2112 surface,1000,26.4,-85.6,3,270 1000,54,26.4,-85.6,3,270
#
# as returned by the dcfm35 decoder, with the <separator> in the argument
# of the function indicating what character separates the different levels.

#
# Functions
#
proc uatomdf_cvt {ref_dd data na_val {separator " "}} {
#
# Here <ref_dd> is the $dd of the date of the file that is being
# requested. Only the data records corresponding to that same day
# are returned.
#
    global wsfilter;	# siteloc array

    set record [split $data $separator];
    set info [lindex $record 0];
    set data [lrange $record 1 end];

    set info_list [split $info ","];
    set stid [lindex $info_list 0];
    set stnm [lindex $info_list 2];
    set ddhh $stnm;	          # ddhh => convert to whole hour minutes

    if {[regexp {(\d{2})(\d{2})} $ddhh match dd hh] == 0} {
	return "";
    }

    if {$dd ne $ref_dd} {	
	return "";
    }

    set hour [wsfilter_strip_zeros $hh];
    set time [expr $hour * 60];

   if {[info exists wsfilter(upperair,siteloc,$stnm)] == 0} {
       set lat $na_val;
       set lon $na_val;
       set elev $na_val;
    } else {
	set latlonelev [split $wsfilter(upperair,siteloc,$stnm) ","];
	set lat [lindex $latlonelev 0];
	set lon [lindex $latlonelev 1];
	set elev [lindex $latlonelev 2];
    }
    
    set r [list];
    lappend r $stid $stnm $time $lat $lon $elev;

    # Convert the data. Values for missing levels, or empty values (missing)
    # returned by nbspfm35d will be set to undef below.
    foreach d $data {
	set values [split $d ","];	# some values will be empty
	set level [lindex $values 0];
	if {$level eq "surface"} {
	    set level "sfc";
	    set val($level,pres) [lindex $values 1];
	    set val($level,tair) [lindex $values 2];
	    set val($level,tdew) [lindex $values 3];
	    set val($level,wspd) [lindex $values 4];
	    set val($level,wdir) [lindex $values 5];
	} else {
	    set val($level,pres) $level;
	    set val($level,hght) [lindex $values 1];
	    set val($level,tair) [lindex $values 2];
	    set val($level,tdew) [lindex $values 3];
	    set val($level,wspd) [lindex $values 4];
	    set val($level,wdir) [lindex $values 5];
	}
    }

    # Initialize the missing or empty values
    foreach name [list pres tair tdew wspd wdir] {
	if {([info exists val(sfc,$name)] == 0) || ($val(sfc,$name) eq "")} {
	    set val(sfc,$name) $na_val;
	}
    }
    foreach level [list 1000 925 850 700 500 400 300 250 200 150 100] {
	foreach name [list pres hght tair tdew wspd wdir] {
	    if {([info exists val($level,$name)] == 0) || \
		    ($val($level,$name) eq "")} {
		set val($level,$name) $na_val;
	    }
	}
    }

    foreach name [list pres tair tdew wspd wdir] {
	lappend r $val(sfc,$name);
    }

    foreach level [list 1000 925 850 700 500 400 300 250 200 150 100] {
	foreach name [list pres hght tair tdew wspd wdir] {
	    lappend r $val($level,$name);
	}
    }

    return [join $r " "];
}

proc uatomdf_header {} {

    set header [list stid stnm time lat lon elev];

    foreach name [list pres tair tdew wspd wdir] {
	lappend header ${name}_sfc;
    }

    foreach level [list 1000 925 850 700 500 400 300 250 200 150 100] {
	foreach name [list pres hght tair tdew wspd wdir] {
	    lappend header ${name}_${level};
	}
    }

    return $header;
}

#
# Tests
# set record "dems,211200,42410,2112|surface,1000,26.4,-85.6,3,270|1000,54,26.4,-85.6,3,270|925,728,20.2,-90.8,28,275|850,1446,13.2,-94.8,23,235|700,3037,1.2,-128.8,75,270|500,5690,-12.3,-137.3,85,265|400,7360,-23.5,-143.5,102,276|300,9390,-41.7,-158.7,126,276|250,1610,-48.1,-167.1,52,330"
#
# puts [uatomdf_header];
# puts "";
# puts [uatomdf_cvt $record -996];
#

proc uatomdf_hourly {date inputfile undef} {
#
# date here is yyyymmddhh.
#

    if {[regexp {(\d{4})(\d{2})(\d{2})(\d{2})} $date match s1 s2 s3 s4] == 0} {
        return -code error "Invalid date.";
    }

    set arg_yyyy $s1;
    set arg_mm $s2;
    set arg_dd $s3;
    set arg_hh $s4;
    set ymd ${arg_yyyy}${arg_mm}${arg_dd};

    set ws_result_list [list];	# what will be returned
    # Write header
    lappend ws_result_list "  101";
    lappend ws_result_list "  74 $arg_yyyy $arg_mm $arg_dd 00 00 00";
    lappend ws_result_list " [uatomdf_header]";

    # The data is run through the fm35 decoder, and the result is then
    # sent line by line to the cvt function above to translate it to
    # the WS format. The "-c" flag indicates that it is not a raw (.upa) data
    # file but a cleaned up (.upperair) data file (also produced by the
    # decoder as run by the uafilter).

    set status [catch {
	set result [exec nbspfm35d -c $inputfile];
    } errmsg];

    if {$status != 0} {
        return -code error $errmsg;
    }

    foreach data [split $result "\n"] {
	set r [uatomdf_cvt $arg_dd $data $undef];
	if {$r ne ""} {
	    lappend ws_result_list $r;
	}
    }

    return [join $ws_result_list "\n"];
}

#
# main
#
package require cmdline;
package require fileutil;

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

# The upperair siteloc file
if {[file exists $wsfilter(upperair_siteloc)]} {
    source $wsfilter(upperair_siteloc);
}

set usage {uatomdf [-d <dir>] [-f] [-n <n>] [-o <outputfile>] [-u <undef>]
    [-y] [-Y <Y>] [<name>]};
set optlist {{d.arg ""} {f} {n.arg "end"} {o.arg ""} {u.arg ""}
    {y} {Y.arg "1"}};

# Defaults unless given in cmd line
set g(undef) $wsfilter(upperair_undef);

array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$option(u) ne ""} {
    set g(undef) $option(u);
}

if {$argc != 0} {
    if {$option(f) == 1} {
	# The format is checked below
	set g(inputfile) [lindex $argv 0];
    } else {
	set ymdh [lindex $argv 0];
	if {[regexp {(\d{4})(\d{2})(\d{2})(\d{2})} \
		$ymdh match yyyy mm dd hh] == 0} {
	    puts "Invalid file name format.";
	    exit 1;
	}
	set ymd ${yyyy}${mm}${dd};
	set g(inputfile) [file join $wsfilter(upperair_basedir) $ymd \
			  $wsfilter(upperair_hourly_basedir) $ymdh];
	append g(inputfile) $wsfilter(upperair_fext);
    }
} else {
    set seconds [clock seconds];
    if {$option(y) == 1} {
	set seconds [expr $seconds - $option(Y)*24*3600];
    }
    set ymd [clock format $seconds -format "%Y%m%d" -gmt true];
    set dir [file join $wsfilter(upperair_basedir) $ymd \
		 $wsfilter(upperair_hourly_basedir)];
    set flist [lsort -dictionary [glob -nocomplain -directory $dir "*"]];
    if {[llength $flist] == 0} {
	return -code error "$dir is empty.";
    }
    # The format is checked below
    set g(inputfile) [lindex $flist $option(n)];
}

if {[file exists $g(inputfile)] == 0} {
    return -code error "$g(inputfile) not found.";
}

# Recompute the ymdh parameter
set fname [file tail [file rootname $g(inputfile)]];
set ymdh $fname;	# must have the standard names yyyymmddhh.<fext>
if {[regexp {(\d{4})(\d{2})(\d{2})(\d{2})} \
	 $ymdh match yyyy mm dd hh] == 0} {
    puts "Invalid file name format: $fname";
    exit 1;
}
set ymd ${yyyy}${mm}${dd};

if {$option(o) eq ""} {
    set fbasename $fname;
    append fbasename $wsfilter(upperair_mdf_fext);
    if {$option(d) ne ""} {
	set g(outputfile) [file join $option(d) [file tail $fbasename]];
    } else {
	set g(outputfile) [file join $wsfilter(ws_datadir) $ymd \
			       $wsfilter(ws_upperairdir) $fbasename];
    }
} else {
    set g(outputfile) $option(o);
    if {($g(outputfile) ne "-") && ($option(d) ne "")} {
	set g(outputfile) [file join $option(d) $option(o)];
    }
}

set status [catch {
    set result [uatomdf_hourly $ymdh $g(inputfile) $g(undef)];
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
