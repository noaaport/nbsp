#
# $Id$
#
# Here functions are functions that override some of the functions
# in dist.
#
# Stderr
# Log_SetFile
# Httpd_ReturnFile
# Httpd_VirtualHosts

#
# Provide a version of Sdterr using syslog
# (original is in utils.c)
#
rename Stderr Stderr.orig
proc Stderr {string} {

    global Config

    Stderr.orig $string

    if {$Config(syslog) == 1} {
	exec logger -t $Config(syslogident) $string
    }
}

#
# Eliminate compressing of the logfile in the background, which was
# leaving a defunct (gzip) process lying around until the next rotation.
# (original function is in log.tcl)
#
rename Log_SetFile Log_SetFile.orig
proc Log_SetFile {{basename {}}} {
    global Log
    if {[string length $basename]} {
	set Log(log) $basename
    }
    if {![info exists Log(log)]} {
	catch {close $Log(log_fd)}
	catch {close $Log(error_fd)}
	catch {close $Log(debug_fd)}
	return
    }
    catch {Counter_CheckPoint} 		;# Save counter data

    # set after event to switch files after midnight
    set now [clock seconds]
    set next [expr {([clock scan 23:59:59 -base $now] -$now + 1000) * 1000}]
    after cancel Log_SetFile
    after $next Log_SetFile

    # Set the log file and error file.
    # In the original, log files rotate, error files don't.
    # In nsbp both rotate.

    set Log(log_file) $Log(log)[clock format $now -format %y.%m.%d]
    catch {close $Log(log_fd)}

    # Create log directory, if neccesary, then open the log file
    catch {file mkdir [file dirname $Log(log_file)]}
    if {[catch {set Log(log_fd) [open $Log(log_file) a]} err]} {
	 Stderr $err
    }

    set error_file ""
    append error_file $Log(log_file) ".error";
    catch {close $Log(error_fd)}
    if {[catch {set Log(error_fd) [open $error_file a]} err]} {
	Stderr $err
    }

    # This debug log gets reset daily
    catch {close $Log(debug_fd)}
    if {[info exists Log(debug_file)] && [file exists $Log(debug_file)]} {
	catch {file rename -force $Log(debug_file) $Log(debug_file).old}
    }

    if {[info exist Log(debug_log)] && $Log(debug_log)} {
	set Log(debug_file) $Log(log)debug
	if {[catch {set Log(debug_fd) [open $Log(debug_file) w]} err]} {
	    Stderr $err
	}
    }
}

#
# Introduce a new function and a wrapper to Httpd_ReturnFile in order
# to support Byte Ranges (needed by the idv catalog)
# (the original is in httpd.tcl)
#

# Add this which is not in the list in dist/httpd.tcl
set Httpd_Errors(206) "Partial Content";

rename Httpd_ReturnFile Httpd_ReturnFile.orig;
proc Httpd_ReturnFile {sock type fpath {offset 0}} {

    global Httpd;
    upvar #0 Httpd$sock data;

    if {[info exists data(mime,range)]} {
	#
	# data(mime,range) contains something like "bytes=n1-n2"
	#
	set size [file size $fpath];
	set start 0;
	set end [expr $size - 1];
	if {[regexp {bytes=(\d+)?-(\d+)?} $data(mime,range) match s e]} {
	    if {$s ne ""} {
		set start $s;
	    }
	    if {$e ne ""} {
		set end $e;
	    }

	    #
	    # XXX - Check range error here
	    #
	    # A server sending a response with status code 416
	    # (Requested range not satisfiable) SHOULD include a
	    # Content-Range field with a byte-range- resp-spec of "*".
	    #
	}
	set range "bytes ${start}-${end}/$size";

	Httpd_AddHeaders $sock "Accept-Ranges" "bytes";
	Httpd_AddHeaders $sock "Content-Range" $range;

	httpd_return_partial_file $sock $type $fpath $start $end;
    } else {
	Httpd_ReturnFile.orig $sock $type $fpath $offset;
    }
}

#
# httpd_return_partial_file
#	Return a byte range of a file.
#
# Arguments:
#	sock	handle on the connection
#	type	is a Content-Type
#	path	is the file pathname
#	start	starting byte
#	end     ending byte
#
# Side Effects:
#	Sends the (partial) file contents back as the reply.

proc httpd_return_partial_file {sock type fpath start end} {

    global Httpd;
    upvar #0 Httpd$sock data;

    if {[Thread_Respond $sock \
        [list httpd_return_partial_file $sock $type $fpath $start $end]]} {
	return
    }

    # Set file size early so it gets into all log records

    set data(file_size) [expr $end - $start + 1];
    set data(code) 206

    Count urlreply
    if {[info exists data(mime,if-modified-since)]} {
        # No need for complicated date comparison.
	# If they're identical then 304.
	if {$data(mime,if-modified-since) == [HttpdDate [file mtime $fpath]]} {
            Httpd_NotModified $sock
            return
        }
    } 

    if {[catch {
	set close [HttpdCloseP $sock]
	HttpdRespondHeader $sock $type $close $data(file_size) $data(code)
	HttpdSetCookie $sock
	puts $sock "Last-Modified: [HttpdDate [file mtime $fpath]]"
	puts $sock ""
	if {$data(proto) != "HEAD"} {
	    set in [open $fpath]      ;# checking should already be done
	    fconfigure $in -translation binary -blocking 1
	    if {$start != 0} {
		seek $in $start
	    }
	    fconfigure $sock -translation binary -blocking $Httpd(sockblock)
	    set data(infile) $in
	    Httpd_Suspend $sock 0
	    fcopy $in $sock -size $data(file_size) \
		-command [list HttpdCopyDone $in $sock $close]
	} else {
	    Httpd_SockClose $sock $close
	}
    } err]} {
	HttpdCloseFinal $sock $err
    }
}

#
# Replace the virtual initialization function
#
rename Httpd_VirtualHosts Httpd_VirtualHosts.orig
proc Httpd_VirtualHosts {hostNames file initfile} {

    variable virtual

    foreach host $hostNames {
        set host [string tolower $host]
        if {[info exists virtual($host)]} {
	    error "Virtual host $host already exists"
        }
    }

    set slave [interp create]
    $slave eval set Config(vhost) 1
    $slave eval set Config(vhost,conffile) $file
    $slave eval source $initfile

    foreach host $hostNames {
        set host [string tolower $host]
	set virtual($host) $slave
    }
}
