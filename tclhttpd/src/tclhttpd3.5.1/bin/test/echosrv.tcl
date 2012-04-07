# This is a collection of procedures used to
# do simple timing tests of Tcl sockets

proc Echo_Server {port} {
	global echo
	set echo(main) [socket -server EchoAccept $port]
	set echo(accepts) 0
	set echo(bytes) 0
}
proc EchoAccept {sock addr port} {
	global echo
#	puts "Accept $sock from $addr port $port"
	incr echo(accepts)
	set echo(addr,$sock) [list $addr $port]
	fconfigure $sock -buffering line
	fileevent $sock readable [list Echo $sock]
}
proc Echo {sock} {
	global echo
	if {[eof $sock] || [catch {gets $sock line} numbytes]} {
		# end-of-file or abnormal connection drop
		close $sock
#		puts "Close $echo(addr,$sock)"
		unset echo(addr,$sock)
	} elseif {$numbytes >= 0} {
		if {[string compare $line "quit"] == 0} {
			# Prevent new connections.
			# Existing connections stay open.
			close $echo(main)
		}
		puts $sock $line
		incr echo(bytes) [string length $line]
		incr echo(bytes)
	}
}
proc Echo_Client {host port} {
    set s [socket $host $port]
    fconfigure $s -buffering line
    return $s
}
proc Echo_Time {s {count 1000}} {
    set start [clock clicks]
    set max $count
    while {[incr count -1] >=0} {
	puts $s "line $count"
	gets $s
    }
    set us [expr { [clock clicks] - $start}]
    puts "[expr $us/$max/1000.0] ms/echo"
}
proc Connect_Time {host port {count 1000}} {
    set start [clock clicks]
    set max $count
    while {[incr count -1] >=0} {
	set s [socket $host $port]
	close $s
    }
    set us [expr { [clock clicks] - $start}]
    puts "[expr $us/$max/1000.0] ms/connect"
}
proc EchoOne_Time {host port {count 100}} {
    set start [clock clicks]
    set max $count
    while {[incr count -1] >=0} {
	set s [Echo_Client $host $port]
	puts $s "line $count"
	gets $s
	close $s
    }
    set us [expr { [clock clicks] - $start}]
    puts "[expr $us/$max/1000.0] ms/open+echo"
}
