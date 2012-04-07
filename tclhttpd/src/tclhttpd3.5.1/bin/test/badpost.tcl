#!/bin/sh
# \
exec tclsh "$0"

package require http 2.0

set url [lindex $argv 0]
if {[string length $url] == 0} {
    set url http://localhost:8015/debug/echo
}
regexp {(http://)?([^:/]+)(:([0-9]+))?(/.*)$} $url x j1 server j2 port url
if {$port == ""} {
    set port 80
}
set body "hello\n"
while {[string length $body] < 17000} {
    set body $body$body
}
		
set f [socket $server $port]
puts $f "POST $url HTTP/1.0"
puts $f "Content-length: [string length $body]"
puts $f ""
flush $f
after 200
puts $f [string range $body 0 100]
flush $f

puts stderr "Pausinng 5 minutes"
after [expr 1000 * 300]
puts $f [string range $body 101 end]
flush $f

