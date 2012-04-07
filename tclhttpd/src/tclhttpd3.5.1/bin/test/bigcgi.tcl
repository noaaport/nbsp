#!/bin/sh
# \
exec tclsh "$0"

package require ncgi

# Construct a vast amount of post data to parse

set value "x"
while {[string length $value] < 1024} {
    append value $value
}

set i 0
set sep ""

while {$i < 100} {
    append bigpost "${sep}name$i=$i$value"
    set sep &
    incr i
}

puts "ncgi::reset [time {ncgi::reset $bigpost}]"
puts "ncgi::parse [time {ncgi::parse}]"
puts "ncgi::nvlist [time {ncgi::nvlist}]"
