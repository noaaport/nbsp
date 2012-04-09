#!/usr/local/bin/tclsh8.4
#
# $Id$
#

proc log_msg s {

    global argv0;

    set name [file tail $argv0];
    exec logger -t $name $s;
}

set g(logfile)	"/var/noaaport/istext.log";
set g(tool) "/usr/local/site/bin/istext";
###set g(logfile)	"istext.log";
###set g(tool) "./istext";
# variables
set g(list) [list];

proc main {argc argv} {

    global g;
    global errorInfo;

    # gets returns -1 on eof. In addition the server explicitly
    # signals the filters to quit by sending a blankline
    # (gets returns 0 in this case, and finfo is "").

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

    global g;

    scan $finfo "%u %d %d %d %d %s %s" seq type cat code npchidx fname fpath;

    # Initalize all keys
    set station "";
    set wmoid "";
    set awips "";
    set awips1 "";
    set awips2 "";
    set nawips "";
    
    if {[regexp {\-} $fname] == 1} {
        scan $fname {%[a-z0-9]_%[a-z0-9]-%[a-z0-9]} station wmoid awips;
        set awips1 [string range $awips 0 2];
        set awips2 [string range $awips 3 end];
    } elseif {[regexp {\+} $fname] == 1} {
        scan $fname {%[a-z0-9]_%[a-z0-9]+%[a-z0-9]} station wmoid nawips;
    } else {
        scan $fname {%[a-z0-9]_%[a-z0-9]} station wmoid;
    }

    set istext_status [exec $g(tool) $fpath];

    if {$istext_status == 0} {
	if {[string length $awips1] != 0} {
	    set key $awips1;
	} else {
	    set key "#$wmoid";
	}
	if {[lsearch $g(list) $key] == -1} {
	    lappend g(list) $key;
	}

	log_file;
    }
}

proc log_file {} {

    global g;

    set status [catch {
        set FP [open $g(logfile) w 0644];
	foreach key $g(list) {
	    puts $FP $key;
	}
	close $FP;
    } errmsg];

    if {$status != 0} {
	log_msg $errmsg;
    }
}

main $argc $argv

