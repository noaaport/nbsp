#!/usr/bin/tclsh

foreach file [split [string trim [exec cat filelist]] "\n"] {
    regexp {(.*)_(.*)-(.*)\.(\d+)_(\d+)} $file match station wmoid awips \
	time seq;
    set AWIPS [string toupper $awips];
    set firstline [exec head -n 1 $file];
    set wmo [string trim [string range $firstline 24 end]];
    set fname [file rootname [file tail $file]];
    puts "$seq 0 0 0 0 $fname [pwd]/$file";
}
