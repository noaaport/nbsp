#!%TCLSH%
#
# $Id$
#
# Usage: nbspmtrplotdat [-o outputfile [-b basedir] [-d subdir]] \
#			[-m marker] [-n numpoints] [-f datafile] <station>
#
# Without options, the data is written to stdout. Otherwise, 
# the tool will cd to the "basedir", create "subdir", and save
# the data in <station>.<ext> or what is given in the [-o] option.
# By default, the number of points included is defined by the
# by the setting of the variable $metarfilter(plotnumpoints).
# The number can be specified with [-n] option. The [-m] option specifies
# a marker to use when the slp is missing. (See sanity_check() below.)

package require fileutil;
package require cmdline;
set usage {nbspmtrplotdat [-o outputfile [-b basedir] [-d subdir]]
    [-n numpoints] [-f datafile] <station>};
set optlist {{o.arg ""} {b.arg ""} {d.arg ""} {n.arg ""} {f.arg ""}
    {m.arg "na"}};

proc sanity_check dataline {
#
# Check that the fields are non blank. Fields 0-7 are the importat ones.
# Field 8 is the slp but we do not use it in the plots so its check is
# omited ; if it missing a special marker (na) is substituted
# in the convert function.
#
    set fields [lrange [split $dataline ","] 0 end-1];
    foreach f $fields {
	if {$f eq ""} {
	    return 1;
	}
    }

    return 0;
}

proc convert_list {origlist numpoints slp_missing_mark} {
#
# The data file is split into lines, and the list of lines is passed
# to this function. The original file has the lines in reverse
# chronological order (metar file list) so here we rearrange that and
# also eliminate lines that duplicate the data for a given hour.
# The function returns the new list, with each field separated by a space
# (for gnuplot). In addition to the original fields, two calculated
# fields are included at the end: pressure in mb, and relative humidity.
# Only the last $numpoints lines are included, unless it is 0 or negative.

    # Determine the field separator from the first line. If there are commas
    # we take FS to be a comma, otherwise a blank.
    set FS ",";
    if {[string match *,* [lindex $origlist 0]] == 0} {
	set FS " ";
    }

    set newlist [list];
    set i 0;
    foreach line $origlist {
	set a [split $line $FS];
	set hhmm [lindex $a 2];
	set ddhh [lindex $a 1][string range $hhmm 0 1];
	if {([info exists data($ddhh)] == 0) && ([sanity_check $line] == 0)} {
	    set data($ddhh) 1;
	    # If slp is missing insert a marker
	    set slp [lindex $a 8];
	    if {$slp eq ""} {
		set a [lreplace $a 8 8 $slp_missing_mark];
	    }
	    # Add the pressure in mb and relative humid at the end of the data.
	    lappend a [expr [lindex $a 7] * 33.8639];
	    lappend a [relative_humidity [lindex $a 5] [lindex $a 6]];

	    set newlist [linsert $newlist 0 [join $a " "]];

	    incr i;
	    if {($numpoints > 0) && ($i == $numpoints)} {
		break;
	    }
	}
    }

    return $newlist;
}

proc relative_humidity {T D} {
#
# T and D are supplied in fahrenheit.
#
#########################################################################
# We use formulas based on those given in
#
# http://www.srh.noaa.gov/elp/wxcalc/formulas/rhTdFromWetBulb.html
#
# According to these, the relative humidity is given by
#
#	H = (e/e_s) * 100
#
# where
#
#	e = 6.112 * exp^{f(t)}
#
# and
#
#	e_s = 6.112 * exp^{f(d)}  (obtained by inverting their formula for T_d)
#
# with t and d here being T and D but expressed in degC. The function f(x)
# is given by
#
#	f(x) = \frac{bx}{a + x}
#
# with
#
#	b = 17.67
#	a = 243.5
#
# We first define (for X = T,D)
#
#	x = (5/9)*(X - 32)
#
#	F(X) = f(x) = f((5/9)*(X - 32))
#
# and then
#
#	H/100 = exp^{F(D) - F(T)}
#
# F(X) is given by
#
#	F(X) = \frac{9.82 X - 314.13}{225.72 + 0.56 X}
#########################################################################

    set FT [expr (9.82*$T - 314.13)/(225.72 + 0.56*$T)];
    set FD [expr (9.82*$D - 314.13)/(225.72 + 0.56*$D)];
    
    set H [expr $FD - $FT];
    set H [expr int(100*exp($H))];

    return $H;
}

## The common defaults
set _defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists ${_defaultsfile}] == 0} {
    puts "${_defaultsfile} not found.";
    return 1;
}
source ${_defaultsfile};
unset _defaultsfile;

# The default configuration is shared between the filter and cmd-line tools
# and therefore it is in a separate file that is read by both.
set metar_init_file [file join $common(libdir) metarfilter.init];
if {[file exists $metar_init_file] == 0} {
        puts "$metar_init_file not found.";
        return 1;
}
source $metar_init_file;
unset metar_init_file;

array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc != 1} {
    puts $usage;
    exit 1;
} else {
    set station [lindex $argv 0];
}

if {$option(f) == ""} {
    set dir [file join $metarfilter(datadir) $metarfilter(mwdir)];
    append fname $station $metarfilter(mwfext); 
    set datafile [exec find $dir -name $fname];
    if {$datafile == ""} {
	puts "$fname not found in $dir.";
	exit 1;
    }
} else {
    set datafile $option(f);
}

set numpoints $metarfilter(plotnumpoints);
if {$option(n) != ""} {
    set numpoints $option(n);
}

set body [exec nbspmtrd -t -d $datafile];
set lineslist [split $body "\n"];
set newlist [convert_list $lineslist $numpoints $option(m)];
if {[llength $newlist] == 0} {
    puts "No useful data in $datafile.";
    exit 1;
}
set newbody [join $newlist "\n"];

if {($option(b) == "") && ($option(d) == "") && ($option(o) == "")} {
    puts $newbody;
    exit 0;
}

if {$option(b) != ""} {
    cd $option(b);
}

if {$option(d) != ""} {    
    file mkdir $option(d);
    cd $option(d);
}

if {$option(o) == ""} {
    set outputfile $station$metarfilter(plotdataext);
} else {
    set outputfile $option(o);
}

set status [catch {
    ::fileutil::writeFile $outputfile $newbody;
} errmsg];

if {$status != 0} {
    puts $errmsg;
    exit 1;
}
