#!/usr/local/bin/tclsh8.4

lappend auto_path "/usr/local/libexec/nbsp";
package require cspoolbdb;

set conffile "/usr/local/etc/nbsp/cspoolbdb.conf";

if {[file exists $conffile] == 0} {
    puts stderr "$conffile not found";
    exit 1;
}

::cspoolbdb::init $conffile;
::cspoolbdb::open;

while {[gets stdin key] > 0} {
    set key [string trim $key];
    if {$key eq ""} {
	break;
    }
    set result [::cspoolbdb::read $key];
    set code [lindex $result 0];
    set size [lindex $result 1];
    set body [lindex $result 2];

    set f [open "/tmp/$key" w];
    puts -nonewline $f $body;
    close $f;

    puts "Wrote /tmp/$key";
}
::cspoolbdb::close;
