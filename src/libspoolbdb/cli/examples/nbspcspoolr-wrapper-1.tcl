#!/usr/local/bin/tclsh8.4

set dir /var/noaaport/nbsp/cache
set name cachedb
set mb 32
set ndb 4

set cmd [list "|nbspcspoolr" -b -d $dir -f $name -c $mb -n $ndb]
set F [open $cmd r+];
fconfigure $F -translation binary -encoding binary -buffering none;

while {[gets stdin key] > 0} {
    set key [string trim $key];
    if {$key eq ""} {
	break;
    }

    puts $F $key;
    flush $F;

    set code [read $F 3];
    puts $code
       
    scan [read $F 8] "%x" data_size;
    puts $data_size;
    
    set data [read $F $data_size;];

    set f [open "/tmp/$key" w];
    fconfigure $f -translation binary -encoding binary;
    puts -nonewline $f $data;
    close $f;

    puts "Wrote /tmp/$key";
}
close $F;
