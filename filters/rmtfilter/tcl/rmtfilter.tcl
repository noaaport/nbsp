#!/usr/local/bin/tclsh8.6
#
# $Id$
#
# Open any number of filters in any number of remote hosts.
#

## The common defaults
set filters_init_file "/usr/local/libexec/nbsp/filters.init";
if {[file exists $filters_init_file] == 0} {
    puts "rmtfilter disabled: $filters_init_file not found.";
    return 1;
}
source $filters_init_file;
unset filters_init_file;

if {$filters_init_status == 1} {
    return 1;
}
unset filters_init_status;

# packages
#
## package require nbsp::util;

#
# testing
#
set common(libdir) ".";
set common(confdir) ".";
set common(localconfdirs) [list site];

set _init_file [file join $common(libdir) "rmtfilter.init"];
if {[file exists $_init_file] == 0} {
    log_msg "rmtfilter disabled: $rmt_init_file not found.";
    return 1;
}
source $_init_file;
unset _init_file;

#
# Functions
#

proc rmtfilter_open_filter {node filter} {

    global rmtfilter;

    set cmd [join [concat "|ssh" $rmtfilter(sshopts,$node,$filter) \
		       $node $filter] " "];

    set status [catch {
	set F [open $cmd w];
	fconfigure $F -buffering line -encoding binary -translation binary;
    } errmsg];

    if {$status == 0} {
	set rmtfilter(F,$node,$filter) $F;
	log_msg "Opened $node:$filter";
    } else {
	log_errc "Could not open $node:$filter. $errmsg";
    }

    return $status;
}

proc rmtfilter_close_filter {node filter} {
    
    global rmtfilter;

    set status 0;

    if {[info exists rmtfilter(F,$node,$filter)]} {
	set status [catch {close $rmtfilter(F,$node,$filter)} errmsg];
	unset rmtfilter(F,$node,$filter);
    }

    if {$status != 0} {
	log_errc $errmsg;
    } else {
	log_msg "Closed $node:$filter";
    }
}

proc rmtfilter_isopen_filter {node filter} {

    global rmtfilter;

    if {[info exists rmtfilter(F,$node,$filter)]} {
	return 1;
    } else {
	return 0;
    }
}

proc rmtfilter_writeto_filter {node filter data} {

    global rmtfilter;

    if {[rmtfilter_isopen_filter $node $filter] == 0} {
	rmtfilter_open_filter $node $filter;
    }

    if {[rmtfilter_isopen_filter $node $filter] == 0} {
	return 1;
    }

    set status [catch {
	puts $rmtfilter(F,$node,$filter) $data;
    } errmsg];

    if {$status != 0} {
	rmtfilter_close_filter $node $filter;
	log_errc "Could not write to $node:$filter. $errmsg";
    }

    return $status;
}

proc rmtfilter_remove_filter {node filter} {

    global rmtfilter;

    set i [lsearch -exact $rmtfilter(filter_list,$node) $filter];
    if {$i != -1} {
	set $rmtfilter(filter_list,$node) \
	    [lreplace $rmtfilter(filter_list,$node) $i $i];
    }
}

proc rmtfilter_open_all_filters {} {

    global rmtfilter;

    foreach node $rmtfilter(nodes) {
	foreach filter $rmtfilter(filter_list,$node) {
	    set status [rmtfilter_open_filter $node $filter];
	    if {$status != 0} {
		rmtfilter_remove_filter $node $filter;
		log_msg "Removed $node:$filter";
	    }
	}
    }
}

proc rmtfilter_close_all_filters {} {

    global rmtfilter;

    foreach node $rmtfilter(nodes) {
	foreach filter $rmtfilter(filter_list,$node) {
	    rmtfilter_close_filter $node $filter;
	}
    }
}

proc main {argc argv} {

    global errorInfo;
    while {[gets stdin finfo] >= 0} {
        if {$finfo == ""} {
            log_msg "Received request to quit.";
            break;
        }

        set status [catch {process $finfo} errmsg];
        if {$status == 1} {
            log_msg "Error processing $finfo";
            log_msg $errmsg;
            log_msg $errorInfo;
        }
    }
}

proc process {finfo} {

    global rmtfilter;

    filterlib_get_rcvars rc $finfo -nobody;

    foreach node $rmtfilter(nodes) {
	foreach filter $rmtfilter(filter_list,$node) {

	    if {[filterlib_uwildmat \
		     $rmtfilter(fname_uregex,$node,$filter) $rc(fname)] == 0} {
		continue;
	    }

	    set status [rmtfilter_writeto_filter $node $filter $finfo];
	    if {$status != 0} {
		# Retry
		set status [rmtfilter_writeto_filter $node $filter $finfo];
		if {$status == 0} {
		    log_msg "Retrying suceeded writing to $node:$filter";
		}
	    }
	}
    }
}

#
# main
#
rmtfilter_open_all_filters;
main $argc $argv
rmtfilter_close_all_filters;
