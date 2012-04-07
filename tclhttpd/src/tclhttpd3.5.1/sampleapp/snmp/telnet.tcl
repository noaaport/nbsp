# telnet.tcl --
# This is some support to telnet into a box and display the
# screens as HTML pages, where the menu items are hot links.
#
# Stephen Uhler  (c) 1997 Sun Microsystems
# Brent Welch (c) 1998-2000 Ajuba Solutions
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# RCS: @(#) $Id$

package provide httpd::telnet 1.0

# run a telnet session with the box
# args:
#   host: The host to connect to
#   -channel:  The session state, use "" for new session
#   -page:     The page to goto
#   -key:      The key chosen by the user

proc Telnet {host args} {
    global Snmp
    array set options {
       -session {}
       -page {}
       -key {}
       -password {}
       -addcr 0
       -channel {}
       -start_page /
    }
    optionSet options $args
    catch {Import $Snmp(statevar)}
    foreach table $session {
	Import $table
    }
#    poptions options
    catch {Import_clear $Snmp(statevar)}

    # close an existing session

    if {$key == "exit"} {
	puts "Closing $channel"
    	catch {close $channel}
    	append result "<title>Telnet session completed<title>"
    	append result "<h1>$host Telnet session completed</h1>"
	append result "<a href=$start_page>New session?</a>"
    	return $result
    }

    # setup a new session, do initial handshaking

    if {$channel == {}} {
	set channel [socket $host telnet]
	puts "Session $channel"
	fconfigure $channel -blocking 0 -translation binary
	while {1} {
	    switch [expect $channel [list "^(\xff..)+" "password:  "] got 5000] {
		1 {		# handle telnet content negotiation (poorly)
		    foreach i [split $got {}] {
			# puts [scan $i %c x; set x]
		    }
		    puts -nonewline $channel $got
		    flush $channel
		}
		2 {		# Send the password
		    puts "sending password ($password)"
		    puts -nonewline $channel $password\r\n
		    flush $channel
		    break
		}
		default {	# format a better message here
		    close $channel
		    return "Invalid password, sorry"
		}
	    }
	}
    expect $channel Selection: got 1000
    return [mung $got $page $channel]
    }

    # have session, get next page

    puts -nonewline $channel $key
    if {$addcr} {
	puts -nonewline $channel \r\n
    }
    puts "Sending <$key> ($addcr)"
    flush $channel
    set which [expect $channel [list {: +$} {==> +$}] got 1000]
    #puts "Got: <$got>"
    switch $which {
    	1 {
	    set result [mung $got $page $channel]
	    append result "\n<hr><a href=$page?channel=$session&key=exit>"
	    append result "Exit</a>"
	    return $result
	}
    	2 {
	    set result [mung2 $got $page $channel]
	    append result "\n<hr><a href=$page?channel=$session&key=exit>"
	    append result "Exit</a>"
	    return $result
	}
    	default {
	    close $channel
	    append result "<title>Oops</title>\n"
	    append result "<h1>Oops, I'm lost, sorry!</h1>\n"
	    append result "Got:<pr><pre>$got</pre>\n"
	    append result "<hr><a href=$start_page>try again</a>"
	    return $result
	}
    }
}

# Whimpy expect stuff
# Read from socket until an expression is matched"
#  sock:	The socket to read from
#  exp:		The list of exp's to match
#  data:	The name of the variable to hold the result
#  timeout:	ms's to wait if no data is available
#  incr:	How long to wait before next poll

proc expect {sock exps data timeout {incr 250}} {
    upvar $data result
    set result ""
    puts "expect: [join $exps ,]"
    while {$timeout > 0} {
	if {[catch {read $sock} data]} {
	    set result $data
	    return -1
	}
	# puts "read: [string length $data]: <$data>"
	if {[string length $data] == 0} {
	    incr timeout -$incr
	    after $incr {set Poke 1}
	    vwait Poke
	    continue
	}
	append result $data
	set index 0
	foreach exp $exps {
	    incr index
	    if {[regexp $exp $result]} {
		puts "   match ($exp) $index"
		return $index
	    }
	}
    }
    return 0
}

# Turn the telnet output into a web page
# This device asks for 3 different types of inforation
# - [X]		a single character
# - [nnn]	one of a list of characters
# "==> $"	a data value

proc mung {data page session} {
    # strip leading and trailing white space
    # regsub ^\[\x80-\xff\r\n]+ $data {} data
    regsub {^[^C]+C} $data C data
    regsub \[\r\n]+$ $data {} data

    # Highlight the title (Kludge for now)

    regsub "Cat\[^\r\n]+" $data "<h1>&</h1>" data
    # handle multiple choice case

    set re {\[([^]][^]]+)]}
    set count [regsub -all $re $data \
	    "\[<b>\\1</b>\]" data]
    if {$count} {
    	set form "<form href=$page>\n"
    	append form "<input type=hidden name=channel value=$session>"
    	append form "<input type=hidden name=addcr value=1>"
    	set data "$form\n$data <input name=key size=8>\n"
    	append data </form>
	regsub -all {\[(.)]} $data \
	    "\[<b>\\1</b>\]" data
    } else {
	# handle single character case
	regsub -all {\[(.)]} $data \
	    "<a href=$page?channel=$session\\&key=\\1><b>\\1</b></a>" \
	    data
    }


    return "<pre>$data</pre>"
}

# Mung2
# "==> $"	a data value

proc mung2 {data page session} {
    set value ""
    regexp "\nCurrent setting ===> (\[^\r\n]+)" $data {} value
    set form "<form href=$page>\n"
    append form "<input type=hidden name=channel value=$session>"
    append form "<input type=hidden name=addcr value=1>"
    append form "<pre>$data "
    append form "<input name=key value=\"$value\">\n</pre></form>"
    return $form
}

# This belongs elsewhere

proc Import_clear {{name Global}} { 
    upvar #0 $name data
    catch {unset data}
}
