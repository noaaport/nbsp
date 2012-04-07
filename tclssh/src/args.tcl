#!/usr/local/bin/tclsh8.5

package require cmdline;

proc test_function {output args} {

    set usage {test [-all] [-exit]};
    set optlist {all exit};

    array set option [::cmdline::getoptions args $optlist $usage];

    puts $option(all);
    puts $option(exit);
}

#
# main
#
set output Hello;

test_function $output -all;
