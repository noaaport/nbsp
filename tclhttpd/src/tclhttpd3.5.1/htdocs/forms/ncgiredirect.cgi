#!/bin/sh
# \
exec tclsh "$0"

package require ncgi

ncgi::parse

# Dump query data to make sure we were hit

catch {
    set out [open /tmp/ncgiredirect.txt w]

    puts $out "ncgiredirect query data:"
    foreach {n v} [ncgi::nvlist] {
	puts $out "$n = $v"
    }
    close $out
}

ncgi::redirect redirect2.tml?message=[ncgi::encode "kilroy was here"]&time=[ncgi::encode [clock format [clock seconds]]]

exit 0
