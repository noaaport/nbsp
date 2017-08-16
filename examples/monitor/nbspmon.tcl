#!/usr/bin/tclsh
#
# Example for monitoring a remote nbspd server using the web interface
#
# The scripts nbspstats and nbspstatcounters installed with the package
# are suitable for monitoring a local nbspd server.  This script
# is an example of how to use the web interface for monitoring a remote
# nbspd server. The nbsp-stats and nbsp-mon repositories contain
# more examples, including some sample monit configuration files.
# 
set nodeslist [list "1.nbsp.inoaaport.net" \
		   "2.nbsp.inoaaport.net" \
		   "3.nbsp.inoaaport.net" \
		  ];

set urltmpl {${node}:8015/_inbsp/stats?format=csvk};

foreach node $nodeslist {

    set total_files 0;
    set total_bytes 0;

    set url [subst $urltmpl];
    
    set output [split [exec curl -s $url] ","];

    foreach entry $output {
	if {[regexp {chstats_files} $entry]} {
	    set val [lindex [split $entry "="] 1]
	    incr total_files $val;
	}

	if {[regexp {chstats_bytes} $entry]} {
	    set val [lindex [split $entry "="] 1]
	    incr total_bytes $val;
	}

	if {[regexp {chstats_time} $entry]} {
	    set chstats_time [lindex [split $entry "="] 1];
	}
    }

    append r $node " " $chstats_time " " $total_files " " $total_bytes;
    puts $r;
    set r "";
}
