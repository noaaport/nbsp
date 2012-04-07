#!/bin/sh
# \
exec tclsh "$0"

# Simple Sample httpd/1.0 server in 250 lines of Tcl
# Stephen Uhler / Brent Welch (c) 1996 Sun Microsystems
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

# This is a working sample httpd server written entirely in TCL with the
# CGI and imagemap capability removed.  It has been tested on the Mac, PC
# and Unix.  It is intended as sample of how to write internet servers in
# Tcl. This sample server was derived from a full-featured httpd server,
# also written entirely in Tcl.
# Comments or questions welcome (stephen.uhler@sun.com)

# Httpd is a global array containing the global server state
#  root:	the root of the document directory
#  port:	The port this server is serving
#  listen:	the main listening socket id
#  accepts:	a count of accepted connections so far

# HTTP/1.0 error codes (the ones we use)

array set HttpdErrors {
    204 {No Content}
    400 {Bad Request}
    404 {Not Found}
    503 {Service Unavailable}
    504 {Service Temporarily Unavailable}
    }

array set Httpd {
    bufsize	32768
    sockblock	0
    default	index.html
}

# Start the server by listening for connections on the desired port.

proc Httpd_Server {root {port 80} {myaddr {}}} {
    global Httpd

    array set Httpd [list root $root]
    if {![info exists Httpd(port)]} {
	set Httpd(port) $port
	if {[string length $myaddr]} {
	    set Httpd(listen) [socket -server HttpdAccept -myaddr $myaddr $port]
	    set Httpd(name) $myaddr
	} else {
	    set Httpd(listen) [socket -server HttpdAccept $port]
	    set Httpd(name) [info hostname]
	}
	set Httpd(accepts) 0
    }
    return $Httpd(port)
}

# Accept a new connection from the server and set up a handler
# to read the request from the client.

proc HttpdAccept {newsock ipaddr port} {
    global Httpd
    upvar #0 Httpd$newsock data

    incr Httpd(accepts)
    fconfigure $newsock -blocking $Httpd(sockblock) \
	-buffersize $Httpd(bufsize) \
	-translation {auto crlf}
    set data(ipaddr) $ipaddr
    set data(left) 50
    fileevent $newsock readable [list HttpdRead $newsock]
}

# read data from a client request

proc HttpdRead { sock } {
    upvar #0 Httpd$sock data

    set readCount [gets $sock line]
    if {![info exists data(state)]} {
	if [regexp {(POST|GET) ([^?]+)\??([^ ]*) HTTP/(1.[01])} \
		$line x data(proto) data(url) data(query) data(version)] {
	    set data(state) mime
	} elseif {[string length $line] > 0} {
	    HttpdError $sock 400
	    Httpd_Log $sock Error "bad first line:$line"
	    HttpdSockDone $sock
	} else {
	    # Probably eof after keep-alive
	    HttpdSockDone $sock
	}
	return
    }

    # string compare $readCount 0 maps -1 to -1, 0 to 0, and > 0 to 1

    set state [string compare $readCount 0],$data(state),$data(proto)
    switch -- $state {
	0,mime,GET	-
	0,query,POST	{ HttpdRespond $sock }
	0,mime,POST	{ set data(state) query }
	1,mime,POST	-
	1,mime,GET	{
	    if [regexp {([^:]+):[ 	]*(.*)}  $line dummy key value] {
		set data(mime,[string tolower $key]) $value
	    }
	}
	1,query,POST	{
	    set data(query) $line
	    HttpdRespond $sock
	}
	default {
	    if [eof $sock] {
		Httpd_Log $sock Error "unexpected eof on <$data(url)> request"
	    } else {
		Httpd_Log $sock Error "unhandled state <$state> fetching <$data(url)>"
	    }
	    HttpdError $sock 404
	    HttpdSockDone $sock
	}
    }
}

# Respond to the query.

proc HttpdRespond { sock } {
    global Httpd
    upvar #0 Httpd$sock data

    set mypath [HttpdUrl2File $Httpd(root) $data(url)]
    if {[string length $mypath] == 0} {
	HttpdError $sock 400
	Httpd_Log $sock Error "$data(url) invalid path"
	HttpdSockDone $sock
	return
    }

    if {![catch {open $mypath} in]} {
	puts $sock "HTTP/1.0 200 Data follows"
	puts $sock "Date: [HttpdDate [clock clicks]]"
	puts $sock "Last-Modified: [HttpdDate [file mtime $mypath]]"
	puts $sock "Connection: Keep-Alive"
	puts $sock "Content-Type: [HttpdContentType $mypath]"
	puts $sock "Content-Length: [file size $mypath]"
	puts $sock ""
	fconfigure $sock -translation binary -blocking $Httpd(sockblock)
	fconfigure $in -translation binary -blocking 1
	flush $sock
	fcopy $in $sock -command [list HttpdCopyDone $in $sock]
    } else {
	HttpdError $sock 404
	Httpd_Log $sock Error "$data(url) $in"
	HttpdSockDone $sock
    }
}
proc HttpdCopyDone {in sock bytes {errorMsg {}}} {
    close $in
    HttpdSockDone $sock [expr {[string length $errorMsg] > 0}]
}

# convert the file suffix into a mime type
# add your own types as needed

array set HttpdMimeType {
    {}		text/plain
    .txt	text/plain
    .htm	text/html
    .html	text/html
    .gif	image/gif
    .jpg	image/jpeg
    .xbm	image/x-xbitmap
}

proc HttpdContentType {path} {
    global HttpdMimeType

    set type text/plain
    catch {set type $HttpdMimeType([file extension $path])}
    return $type
}

# Generic error response.

set HttpdErrorFormat {
    <title>Error: %1$s</title>
    Got the error: <b>%2$s</b><br>
    while trying to obtain <b>%3$s</b>
}

proc HttpdError {sock code} {
    upvar #0 Httpd$sock data
    global HttpdErrors HttpdErrorFormat

    append data(url) ""
    set message [format $HttpdErrorFormat $code $HttpdErrors($code)  $data(url)]
    puts $sock "HTTP/1.0 $code $HttpdErrors($code)"
    puts $sock "Date: [HttpdDate [clock clicks]]"
    puts $sock "Content-Length: [string length $message]"
    puts $sock ""
    puts $sock $message
}

# Generate a date string in HTTP format.

proc HttpdDate {clicks} {
    return [clock format $clicks -format {%a, %d %b %Y %T %Z}]
}

# Log an Httpd transaction.
# This should be replaced as needed.

proc Httpd_Log {sock reason args} {
    global httpdLog httpClicks
    if {[info exists httpdLog]} {
	if ![info exists httpClicks] {
	    set last 0
	} else {
	    set last $httpClicks
	}
	set httpClicks [clock clicks]
	catch {
	    puts $httpdLog "[clock format [clock seconds]] ([expr $httpClicks - $last])\t$sock\t$reason\t[join $args { }]"
	}
    }
}

# Convert a url into a pathname.
# This is probably not right.

proc HttpdUrl2File {root url} {
    global HttpdUrlCache Httpd

    if {![info exists HttpdUrlCache($url)]} {
    	lappend pathlist $root
    	set level 0
	foreach part  [split $url /] {
	    set part [HttpdCgiMap $part]
	    if [regexp {[:/]} $part] {
		return [set HttpdUrlCache($url) ""]
	    }
	    switch -- $part {
		.  { }
		.. {incr level -1}
		default {incr level}
	    }
	    if {$level <= 0} {
		return [set HttpdUrlCache($url) ""]
	    } 
	    lappend pathlist $part
	}
    	set file [eval file join $pathlist]
	if {[file isdirectory $file]} {
	    set file [file join $file $Httpd(default)]
	}
    	set HttpdUrlCache($url) $file
    }
    return $HttpdUrlCache($url)
}

# Decode url-encoded strings.

proc HttpdCgiMap {data} {
    regsub -all {([][$\\])} $data {\\\1} data
    regsub -all {%([0-9a-fA-F][0-9a-fA-F])} $data  {[format %c 0x\1]} data
    return [subst $data]
}

proc bgerror {msg} {
    global errorInfo
    puts stderr "bgerror: $msg\n$errorInfo"
}

# See if we should close the socket
#  sock:  the connection handle

proc HttpdClose {sock} {
    upvar #0 Httpd$sock data

    if {($data(left) > 0) &&
	    (([info exists data(mime,connection)] &&
	     ([string tolower $data(mime,connection)] == "keep-alive")) ||
	    ($data(version) >= 1.1))} {
	set close 0
    } else {
	set close 1
    }
    return $close
}

# Close a socket.

proc HttpdSockDone { sock {close 0}} {
    upvar #0 Httpd$sock data
    global Httpd

    if {!$close && ($data(left) > 0) &&
	    (([info exists data(mime,connection)] &&
	     ([string tolower $data(mime,connection)] == "keep-alive")) ||
	    ($data(version) >= 1.1))} {
	set close 0
    } else {
	set close 1
    }
    if [info exists data(cancel)] {
	after cancel $data(cancel)
    }
    catch {close $data(infile)}
    if {$close} {
	catch {close $sock}
	unset data
    } else {

	# count down transations

	set left [incr data(left) -1]

	# Reset the connection

	flush $sock
	set ipaddr $data(ipaddr)
	unset data
	array set data [list linemode 1 version 0 \
		left $left ipaddr $ipaddr]

	# Close the socket if it is not reused within a timeout

	set data(cancel) [after 10000 [list HttpdSockDone $sock 1]]
	fconfigure $sock -blocking 0 -buffersize $Httpd(bufsize) \
	    -translation {auto crlf}
	fileevent $sock readable [list HttpdRead $sock]
	fileevent $sock writable {}
    }
}


set httpdLog stderr

set root /export/htdocs.v5
set port 80
Httpd_Server $root $port images.scriptics.com
puts stderr "Starting Tcl httpd server on [info hostname] port $port"
vwait forever		;# start the Tcl event loop
