# sendsock.tcl
#
# This implements an eval server so you can talk to the server directly.
# This is best used with Eval_ServerLocal that only listens on a
# localhost socket.  In this case you use secure telnet to get onto
# the server machine first.  Or, you can use Eval_Server and implement
# some secret handshake in the EvalOpenProc hook.
#
# SECURITY NOTE - This is a direct eval hole on your machine.
# I basically never, ever use this code, but instead use the
# debug.tcl module to respectfully ask the server to reload
# its source files.
#
# The server should run
# Eval_ServerLocal someport
#
# Then run tclsh (in a secure telnet session) and source this file, then
# set sock [Eval_Open localhost $someport]
# Eval_Remote $sock [list some tcl command]
# and clean up with
# Eval_Close $sock
#
# The example numbers refer to 2nd ed. of
# Practical Programming in Tcl and Tk
#
# Brent Welch (c) 1997 Sun Microsystems
# Brent Welch (c) 1998-2000 Ajuba Solutions
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# RCS: @(#) $Id$

package provide httpd::eval 1.0

#
# Example 37-4
# Remote eval using sockets.
# port is the listening socket port number
# interp is the Tcl interpreter, which defaults to the main one.
# openCmd is a hook you can use for a secret handshake when starting
# a connection.
#

proc Eval_Server {port {interp {}} {openCmd EvalOpenProc}} {
	socket -server [list EvalAccept $interp $openCmd] $port
	Stderr "Warning: Eval Server running on port $port"
}
proc Eval_ServerLocal {port {interp {}} {openCmd EvalOpenProc}} {
	socket -server [list EvalAccept $interp $openCmd] -myaddr localhost $port
	Stderr "Warning: Eval Server running on port $port"
}
proc EvalAccept {interp openCmd newsock addr port} {
	global eval
	set eval(cmdbuf,$newsock) {}
	fileevent $newsock readable [list EvalRead $newsock $interp]
	if [catch {
		interp eval $interp $openCmd $newsock $addr $port
	}] {
		close $newsock
	}
}
proc EvalOpenProc {sock addr port} {
	# do authentication here
	# close $sock to deny the connection
}


#
# Example 37-5
# Reading commands from a socket.
#

proc EvalRead {sock interp} {
	global eval errorInfo errorCode
	if [eof $sock] {
		close $sock
	} else {
		gets $sock line
		append eval(cmdbuf,$sock) $line\n
		if {[string length $eval(cmdbuf,$sock)] && \
				[info complete $eval(cmdbuf,$sock)]} {
			set code [catch {
				if {[string length $interp] == 0} {
					uplevel #0 $eval(cmdbuf,$sock)
				} else {
					interp eval $interp $eval(cmdbuf,$sock)
				}
			} result]
			set reply [list $code $result $errorInfo \
				$errorCode]\n

			# Use regsub to count newlines
			# The reply is a line count followed
			# by a Tcl list that occupies that number of lines

			set lines [regsub -all \n $reply {} junk]
			puts $sock $lines
			puts -nonewline $sock $reply
			flush $sock
			set eval(cmdbuf,$sock) {}
		}
	}
}


#
# Example 37-6
# The client side of remote evaluation.
# Note.  If you implement a secret handshake in the server side open hook
# then you need to do something on the client after Eval_Open and
# before you try Eval_Remote.
#

proc Eval_Open {server port} {
	global eval
	set sock [socket $server $port]
	# Save this info for error reporting
	set eval(server,$sock) $server:$port
	interp alias {} $sock {} Eval_Remote $sock
	return $sock
}
proc Eval_Remote {sock args} {
	global eval
	# Preserve the concat semantics of eval
	if {[llength $args] > 1} {
		set cmd [concat $args]
	} else {
		set cmd [lindex $args 0]
	}
	puts $sock $cmd
	flush $sock
	# Read return line count and the result.
	gets $sock lines
	set result {}
	while {$lines > 0} {
		gets $sock x
		append result $x\n
		incr lines -1
	}
	set code [lindex $result 0]
	set x [lindex $result 1]
	# Cleanup the end of the stack
	regsub "\[^\n]+$" [lindex $result 2] \
		"*Remote Server $eval(server,$sock)*" stack
	set ec [lindex $result 3]
	return -code $code -errorinfo $stack -errorcode $ec $x
}
proc Eval_Close {sock} {
	close $sock
}


