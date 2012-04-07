# Simple Sample httpd/1.1 server in Tcl
# Stephen Uhler / Brent Welch (c) 1996 Sun Microsystems
# Comments or questions welcome (stephen.uhler@sun.com)

# Httpd is a global array containing the global server state
# Httpd$sock is a global array containing current connection state

# Specify which methods expect to have entity bodies.
# Which methods allow entity-bodies is referred to in the spec,
# but never defined (I think this is a bug in the spec).
# "start" is a dummy method used to initialize the state machine (yuk)

array set Httpd {
    body,HEAD 0  body,GET 0  body,OPTIONS 0  body,TRACE 0
    body,POST 1  body,PUT 1
    body,start 0
}

array set Httpd {method,GET {} method,HEAD {} method,POST {} method

# HTTP/1.1 error codes (the ones we use)

array set HttpdErrors {
    204 {No Content}
    400 {Bad Request}
    404 {Not Found}
    405 {Method not allowed}
    500 {Internal server error}
    503 {Service Unavailable}
    505 {Http Version not supported}
    }

# Start the server by listening for connections on the desired port.

proc Httpd_Server {root {port 80} {default index.html}} {
    global Httpd
    catch {rename unsupported0 copychannel}
    catch {close $Httpd(listen)}
    if {[catch {socket -server HttpdAccept $port} Httpd(listen)]} {
	error "[info hostname]:$port $Httpd(listen)"
    }
    array set Httpd [list root $root default $default port $port accepts 0]
}

# Accept a new connection from the server and set up a handler
# to read the request from the client.

proc HttpdAccept {sock ipaddr port} {
    global Httpd
    upvar #0 Httpd$sock data

    incr Httpd(accepts)
    fconfigure $sock -blocking 0 -translation {auto crlf}
    Httpd_Log $sock Connect $ipaddr $port
    array set data [list ipaddr $ipaddr proto none state start tries 0 version 0]
    fileevent $sock readable [list HttpdRead $newsock]
}

# Read data from a client request.

proc HttpdRead {sock} {
    upvar #0 Httpd$sock data

    # gets:
    #   0 - blank line
    #   >0 - got stuff (complete line) (or partial line if EOF
    #   -1 complete line not available
    # read:
    #  reads as many bytes that are available, up till the amount specified
    # figure out if we are expecting an entity-body by looking at the
    # mime headers.  Look for chunked transfer encoding or content-length

    # reading the header line

    if {$data(state) != "body"} {
	set readCount [gets $sock line]
	set state [string compare $readCount 0],$data(state),$data(version)
	switch -- $state {
	    1,start* {		# Read and validate the request line
		set ok [regexp {([A-Z]+) ([^?]+)\??([^ ]*) HTTP/([1-9]\.[0-9])} \
			$line x data(proto) data(url) data(query) version]
		if {!$ok} {
		    HttpdError $sock 400 $line
		} elseif {![regexp {1\.([01])} $version x $data(version)} {
		    HttpdError $sock 505 $version
		} elseif {![info exists data(body,$data(proto)]} {
		    HttpdError $sock 405 $data(proto)
		}
		set data(state) head
		Httpd_Log $sock Query $line
	    }
	    1,head* {		# read the mime headers
		if [regexp {([^:]+):[ 	]*(.*)}  $line dummy key value] {
		    set data(key) [string tolower $key]
		    if {[info exists data($data(key))]} {
			append data(mime,$data(key)) ", " $value
		    } else {
			set data(mime,$data(key)) $value
		    }
		} elseif [regexp {^[ 	]+(.*)} $line dummy value] {
		    append data(mime,$data(key)) " " $value
		} else {
		    HttpdError $sock 400 $line
		}
	    }
	    0,head,0 {		# end of header, check for body: Http 1.0
	    	if {[info exists data(mime,content-length)]} {
		    fconfigure $sock -translation {binary crlf}
		    set data(body) ""
		    set data(state) body
		} else {
		    HttpdRespond $sock
		}
	    }
	    0,head,1 {		# end of header, check for body: Http 1.1
	    	set chunked ""; set length ""
	    	catch {set chunked $data(mime,transfer-encoding)} 
	    	catch {set length $data(mime,content-length)}
	    	set do_chunk [string match {chunked[ 	+]*} $chunked]

	    	# should call-back application here to make sure query
	    	# is valid, then issue either continue or error.  I haven't
	    	# decided on what the callback interface looks like yet, 
	    	# so always "continue" for now

	    	if {$do_chunk || $length != ""} {
		    fconfigure $sock -translation {none crlf}
		    puts $sock "HTTP/1.1 100 continue"
		    flush $sock
		    set data(body) ""
		    if {$do_chunk) {
			set data(state) chunked
		    } else {
			set data(state) body
		    }
		} else {
		    HttpdRespond $sock
		}
	    }
	    default {
		if [eof $sock] {
		    HttpdError $sock 400 "Unexpected EOF ($state)"
		} else {
		    HttpdError $sock 400 $state
		}
	    }
	}
    } else {
    	# do chunked or content length
    }
}

# Close a socket.
# We'll use this to implement keep-alives some day.

proc HttpdSockDone { sock } {
upvar #0 Httpd$sock data
    unset data
    close $sock
}

# Respond to the query.

proc HttpdRespond {sock} {
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
	puts $sock "HTTP/1.1 200 Data follows"
	puts $sock "Date: [HttpdDate [clock clicks]]"
	puts $sock "Last-Modified: [HttpdDate [file mtime $mypath]]"
	puts $sock "Content-Type: [HttpdContentType $mypath]"
	puts $sock "Content-Length: [file size $mypath]"
	puts $sock ""
	# This gets around a bug that is fixed in the next patch release.
	fconfigure $sock -translation binary -blocking 1
	fconfigure $in -translation binary -blocking 1
	copychannel $in $sock
	Httpd_Log $sock Done "$mypath"
	close $in
    } else {
	HttpdError $sock 404
	Httpd_Log $sock Error "$data(url) $in"
    }
    HttpdSockDone $sock
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
    puts stderr "==>$sock\t$reason\t$args"
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

Httpd_Server $env(HOME)/public_html 8080 index.html
puts stderr "Starting Tcl httpd server on [info hostname] port 8080"
vwait forever		;# start the Tcl event loop
