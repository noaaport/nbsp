#!%TCLSH%
#
# $Id$
#
# usage: nbsptrackplot.tcl [-D] [-i <datafile>] [-o <outputfile>] [-r <region>]
#	[-s <trackscript>] [-m <mapscript>] [-y <year>] <name>
#
# [-D] => debug mode
# [-i] => data file. Default is ${yyyy}.${name}.data in <datadir>/${region}
# [-o] => default is <name>.png in current dir
# [-r] => default is "at".
# [-s] => default is "track-${region}.grads in $localconfdirs, or the
#         variables
#         trackfilter(grads_script,<region>)
#         trackfilter(grads_script,<name>)
# [-m] => default is "map-${region}.grads in $localconfdirs, or the
#         variables
#         trackfilter(grads_map,<region>)
#         trackfilter(grads_map,<name>)
# [-y] => default is current yyyy
#
package require cmdline;

proc err {s} {
    
    global argv0;

    puts stderr "$argv0: $s";
    exit 1;
}

set usage {usage: nbsptrackplot.tcl [-D] [-i <datafile>] [-o <outputfile>]
    [-r <region>] [-s <trackscript>] [-m <mapscript>] [-y <year>] <name>};
set optlist {{D 0} {i.arg ""} {o.arg ""} {r.arg "at"} {s.arg ""} {m.arg ""}
    {y.arg ""}};
array set option [::cmdline::getoptions argv $optlist $usage];

set argc [llength $argv];
if {$argc != 1} {
    err $usage;
}

## The common defaults initialization
set filters_init_file "/usr/local/libexec/nbsp/filters.init";
if {[file exists $filters_init_file] == 0} {
    err "$filters_init_file not found.";
}
source $filters_init_file;
unset filters_init_file;

if {$filters_init_status == 1} {
    return 1;
}
unset filters_init_status;

# The default configuration is shared between the filter and cmd-line tools
# and therefore it is out in a separate file that is read by both.
set track_init_file [file join $common(libdir) "trackfilter.init"];
if {[file exists $track_init_file] == 0} {
    err "$track_init_file not found.";
}
source $track_init_file;
unset track_init_file;

# This script ends up executing
#
# grads -blc "run $trk_script $map_script $hurricane_name $data_file $img_file"
#
# Now build up the relevant argument variables, based on the filter
# configuration and command line arguments.

set name [lindex $argv 0];
set hurricane_name [string totitle $name];

# grads script
if {$option(s) ne ""} {
    set grads_script $option(s);
    if {[file exists $grads_script] == 0} {
	err "$grads_script not found.";
    }
} else {
    set grads_script "";

    if {[info exists trackfilter(grads_track_script,$name)]} {
	set _grads_script $trackfilter(grads_track_script,$name);
    } elseif {[info exists trackfilter(grads_track_script,$option(r))]} {
	set _grads_script $trackfilter(grads_track_script,$option(r));
    } else {
        append _grads_script $trackfilter(grads_track_script_prefix) \
	    $option(r) $trackfilter(grads_track_script_suffix);
    }
    foreach _d $trackfilter(scriptsdirs) {
	if {[file exists [file join ${_d} ${_grads_script}]]} {
	    set grads_script [file join ${_d} ${_grads_script}];
	}
    }
    if {$grads_script eq ""} {
	err "${_grads_script} not found.";
    }
}

# Same logic for the map script
if {$option(m) ne ""} {
    set map_script $option(m);
    if {[file exists $map_script] == 0} {
	err "$map_script not found.";
    }
} else {
    set map_script "";

    if {[info exists trackfilter(grads_map_script,$name)]} {
	set _map_script $trackfilter(grads_map_script,$name);
    } elseif {[info exists trackfilter(grads_map_script,$option(r))]} {
	set _map_script $trackfilter(grads_map_script,$option(r));
    } else {
        append _map_script $trackfilter(grads_map_script_prefix) $option(r) \
	    $trackfilter(grads_map_script_suffix);
    }
    foreach _d $trackfilter(scriptsdirs) {
	if {[file exists [file join ${_d} ${_map_script}]]} {
	    set map_script [file join ${_d} ${_map_script}];
	}
    }
    if {$map_script eq ""} {
	err "${_map_script} not found.";
    }
}

# data file
if {$option(i) ne ""} {
    set data_file $option(i);
} else {
    set yyyy [clock format [clock seconds] -gmt true -format "%Y"];
    if {$option(y) ne "" } {
	set yyyy $option(y);
    }
    set _data_file ${yyyy}.${name}$trackfilter(datafext);
    set _datadir [file join $trackfilter(datadir) $trackfilter(trackdatadir) \
		  $option(r)];
    set data_file [file join ${_datadir} ${_data_file}];
}
if {[file exists $data_file] == 0} {
    err "$data_file not found.";
}

# output file
if {$option(o) eq ""} {
    append img_file $name $trackfilter(imgfext);
} else {
    set img_file $option(o);
}

# The cmd args to grads must be in one string.
# NOTE: Ordinarily we should be able to use "exec". But GrADS does not
# behave well when it encounters an error; instead of writing the errmsg
# and quiting, it writes the error and then a prompt (even with the [-b]
# switch). The result is that exec hangs. To work around this we execute it
# via a pipe.
lappend grads_cmd [list "run" $grads_script $map_script $hurricane_name \
			$data_file $img_file];
##
## set status [catch {exec grads -blc [join $grads_cmd " "]} errmsg];
##
set status [catch {
    if {$option(D) == 0} {
	set F [open "|grads -lb > /dev/null" "w"];
    } else {
	set F [open "|grads -lb" "w"];
    }
    puts $F [join $grads_cmd " "];
    close $F;
    unset F;
} errmsg];

if {$status != 0} {
    if {[info exists F]} {
	close $F;
    }
    err $errmsg;
} elseif {[file exists $img_file] == 0} {
    err "GrADS could not create $img_file.";
}
