#!/usr/local/bin/tclsh8.4
#
# $Id$
#

proc main {argc argv} {

    # defaults
    set gempakfilter "/usr/local/libexec/nbsp/gempakfilter.pl";
    set gempakfilter_bindir "/home/gempak/bin";
    set configfile "/usr/local/etc/gempakfilter.conf";

    if {$argc > 0} {
	set configfile [lindex $argv 0];
    }

    if {[file exists $configfile] == 1} {
	source $configfile;
    }

    set prog_cmd "$gempakfilter -b $gempakfilter_bindir";

    set fout [open "|$prog_cmd" w];

    while {[gets stdin finfo] >= 0} {
	    puts $fout $finfo;
    }

    close $fout;
}

proc log_msg s {

    global argv0;

    set name [file tail $argv0];
    exec logger -t $name $s;
}

main $argc $argv

