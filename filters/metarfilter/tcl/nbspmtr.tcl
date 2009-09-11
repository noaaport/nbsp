#!%TCLSH%
#
# $Id$
#
# Usage:
#
# nbspmtr [-r] [-n number| -a] <collective>
# nbspmtr [-r | -t | -d] [-n number| -a] <obstation>
#
# [-r] prints the entire csv record;
# [-d] prints only the data portion; 
# [-t] it prints a textual report;
# Without options prints the data portion prefixed with an "M" or "S".
# [-n] print the first number entries; otherwise only the first.
# [-a] print all entries
#
package require cmdline;

set usage {nbspmtr [-r] [-n number| -a] <collective>
nbspmtr [-r | -t | -d] [-n number| -a] <station>};

set optlist {{d 0} {r 0} {t 0} {n.arg 1} {a 0}};
set conflict_drt 0;
set conflict_an 0;

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
	puts "metarfilter disabled: $metar_init_file not found.";
	return 1;
}
source $metar_init_file;
unset metar_init_file;

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc != 1} {
    puts $usage;
    exit 1;
} else {
    set name [lindex $argv 0];
}

# Check for conflicting options
if {$option(d) != 0} {
    incr conflict_drt;
}

if {$option(r) != 0} {
    incr conflict_drt;
}

if {$option(t) != 0} {
    incr conflict_drt;
}

if {$option(n) != 1} {
    incr conflict_an;
}

if {$option(a) != 0} {
    incr conflict_an;
}

if {($conflict_drt > 1) || ($conflict_an > 1) || ($option(n) <= 0)} {
    puts $usage;
    exit 1;
}

# Find out if we are dealing with a collective.
set _iscollective -1;
if {[info exists collective($name)]} {
    set _iscollective 1;
    set datadir \
      [file join $metarfilter(datadir) $metarfilter(collectivedatadir) $name]; 
} else {
    # Assume it is a station. Find the first collective to which it belongs.
    foreach cname [array names collective] {
	if {($collective($cname) != "") && \
	    [regexp $collective($cname) $name]} {

	    set _iscollective 0;
	    set datadir [file join $metarfilter(datadir) \
		$metarfilter(stationdatadir) $cname]; 

	    break;
	}
    }
}

if {${_iscollective} == -1} {
    puts "$name not defined.";
    exit 1;
}

if {${_iscollective} == 0} {
    # If there were several files for a station (e.g, labeled with
    # a date we could use something like
    # set filelist [lsort -decreasing \
    #	[glob -directory $datadir -nocomplain -tails "$name*"]];
    # But the metarfilter uses one csv file for each station.
    set _stationfile [file join $datadir ${name}];
    append _stationfile $metarfilter(rawfext);
    set itemlist [list];
    if {[file exists ${_stationfile}]} {
	set itemlist [split [string trimright [exec cat ${_stationfile}]] "\n"];
    }
} else {
    # Get the list of stations in the collective
    set itemlist [split $collective($name) "|"];
}

if {[llength $itemlist] == 0} {
    puts "No reports from $name.";
    exit 1;
}

# How many entries to consider
set count [llength $itemlist];
if {($option(a) == 0) && ($option(n) < $count)} {
    set count $option(n);
}

# We use this to _try_ to detect duplicate records in the stations data file
set last_obdata "";

set i 0;
foreach item $itemlist {
    if {$i == $count} {
	break;
    }

    if {${_iscollective} != 0} {
	set file $item;
	append file $metarfilter(rawfext);
    	# nbsp saves the files with lower case names
	set file [string tolower $file];
	if {[file exists [file join $datadir $file]]} {
	    set record [string trimright [exec cat [file join $datadir $file]]];
	} else {
	    set record "";
	}
    } else {
	set record $item;
    }

    if {$record eq ""} {
	continue;
    }

    set elements [split $record $metarfilter(FS)];
    set obdata [lindex $elements end];
    # Try to detect duplicates
    if {[string compare $obdata $last_obdata] == 0} {
	continue;
    }
    set last_obdata $obdata;
    if {$option(r) == 0} {
	if {$option(t) == 0} {
	    if {$option(d) == 0} {
		# Add the report type as the first word of the data line
		puts [join [lrange $elements end-1 end] " "];
	    } else {
		puts [lindex $elements end];
	    }
	} elseif {${_iscollective} == 0} {
	    # nbspmtrd expects the report type as the first word
	    # of the data line.
	    puts [exec nbspmtrd -e [join [lrange $elements end-1 end] " "]];
	    break;
	} else {
	    puts $usage;
	    exit 1;
	}
    } else {
	puts $record;
    }

    incr i;
}
