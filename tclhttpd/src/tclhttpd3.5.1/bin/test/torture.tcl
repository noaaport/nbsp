# Sample Exorsizer for Http servers
# Stephen Uhler (c) 1996 Sun Microsystems

# Fetch a document many times, simultaneously

switch $tcl_platform(platform) {
    unix {
	set null_path /dev/null
    }
    windows {
	set null_path NUL
    }
}
puts "Calibrating clock clicks"
set start [clock clicks]
after 2000
set end [clock clicks]
set rate [expr {($end - $start) / 2.0}]
puts "$rate clicks/second"

proc Spray {server port count args} {
    global max finish done start total null null_path
    set max $count
    set done 0
    set total 0
    puts "Starting $count fetches"
    set start [clock clicks]
    while {[incr count -1] >=0} {
	if {[catch {socket $server $port} s]} {
	    set max [expr $max - $count]
	    puts "Only $max fetches started"
	    puts ($s)
	    break
	}
	fconfigure $s -block 0
	puts $s "GET [lindex $args [expr $count%[llength $args]]] HTTP/1.0"
    	puts $s "User-agent: Tcl-Web-tester"
    	puts $s "Accept: */*"
	puts $s ""
	flush $s
	set null [open $null_path w]
	if {[info commands fcopy] == "fcopy"} {
	    fcopy $s $null -command [list CopyDone $s $null]
	} else {
	    fileevent $s readable [list Read $s $null]
	}
    }
    vwait finish
    Report $max
}

proc Report {max} {
    global start total rate
    set sec [expr {(([clock clicks] - $start) / $rate)}]
    set msec [expr {$sec * 1000.}]
    puts "$sec sec $total bytes $max fetches"
    puts "[expr $msec/$max] ms/fetch"
    puts "[expr $total/$sec] bytes/sec"
    puts "[expr $max/$sec] fetches/sec"
    puts "[expr $total/$max] bytes/fetch"
}
# file-event handler

proc Read {s null} {
    global start total
    if [eof $s] {
	CopyDone $s $null
    } else {
	incr total [unsupported0 $s $null]
    }
}

proc CopyDone {s null {bytes 0} {error {}}} {
    global done max finish total
    close $s
    close $null
    incr total $bytes
    if {[incr done] == $max} {
	set finish 1
    }
}

proc IterateInner {sockcmd server port count url {postdata {}} args} {
    global max finish done start total null_path
    set countOrig $count
    set total 0
    if {[string length $postdata]} {
	set op POST
    } else {
	set op GET
    }
    set urls [concat [list $url] $args]
    puts "Starting $count fetches"
    set start [clock clicks]
    while {[incr count -1] >=0} {
	set max 1
	set done 0
	if {[catch {$sockcmd $server $port} s]} {
	    set countOrig [expr $max - $count]
	    puts "Only $countOrig fetches started"
	    puts ($s)
	    break
	}
	fconfigure $s -block 0
	puts $s "$op [lindex $urls [expr $count%[llength $urls]]] HTTP/1.0"
    	puts $s "User-agent: Tcl-Web-tester"
    	puts $s "Accept: */*"
	if {$op == "POST"} {
	    puts $s "Content-Length: [string length $postdata]"
	    puts $s ""
	    puts $s $postdata
	} else {
	    puts $s ""
	}
	flush $s
	set null [open $null_path w]
	if {[info commands fcopy] == "fcopy"} {
	    fcopy $s $null -command [list CopyDone $s $null]
	} else {
	    fileevent $s readable [list Read $s $null]
	}
	vwait finish

	if {($count % 1000) == 0} {
	    puts $count
	    update
	}
    }
    Report $countOrig
}
proc Iterate {args} {
    eval {IterateInner socket} $args
}

if {![catch {package require tls}]} {
    puts "Defining SIterate for https torture"

    proc SIterate {args} {
	eval {IterateInner tls::socket} $args
    }
}
