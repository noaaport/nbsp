#!/usr/local/bin/tclsh8.4
#
# $Id$
#

proc main {argc argv} {

    # defaults
    set nbspfilterrc "/usr/local/etc/nbspfilter.rc";
    set configfile "/usr/local/etc/nbsp.conf";

    if {$argc > 0} {
	set configfile [lindex $argv 0];
    }

    if {[file exists $configfile] == 1} {
	source $configfile;
    }

    if {[file exists $nbspfilterrc] == 0} {
	log_msg "Filter disabled: $nbspfilterrc not found.";
	return 1;
    }

    while {[gets stdin finfo] >= 0} {
	process $finfo $nbspfilterrc;
    }
}

proc process {finfo rulesfile} {

    scan $finfo "%d %d %d %d %s" seq type cat code fpath;
    set fname [file tail $fpath];

    if {[regexp {\-} $fname] == 1} {
	scan $fname {%[a-z0-9]_%[a-z0-9]-%[a-z0-9]} station wmoid awips;
	set nawips "";
    } elseif {[regexp {\+} $fname] == 1} {
	scan $fname {%[a-z0-9]_%[a-z0-9]+%[a-z0-9]} station wmoid nawips;
	set awips "";
    } else {
	scan $fname {%[a-z0-9]_%[a-z0-9]} station wmoid;
	set awips "";
	set nawips "";
    }

    source $rulesfile;
}

proc filter {fpath prog_cmd {
    
    set fout [open "|$prog_cmd" w];
    set fin  [open $fpath r];
    fconfigure $fin -translation binary -encoding binary;
    fconfigure $fout -translation binary -encoding binary;

    set garbage [read $fin 24];
    puts $fout [read $fin];
    close $fout;
    close $fin;
}

proc log_msg s {

    global argv0;

    set name [file tail $argv0];
    exec logger -t $name $s;
}

main $argc $argv

