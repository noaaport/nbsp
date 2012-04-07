#!/usr/local/bin/tclsh8.4
#
# $Id$
#
# Usage: cspoolrpipe [-e <exec_cmd>] [-f <fpath>] [-p <pipe_cmd>] <key>
#
# Without the "-p" option, the program outputs the content of the file
# to stdout, otherwise it pipes the contents to the command given as the
# argument to "-p" using the tcl open function.
# If the <key> is not found in the db, then it will exec the <exec_cmd>.

proc proc_cspoolrpipe_pipe {cmd body} {

    set status [catch {
	set F [open "|$cmd" w];
	fconfigure $F -translation binary -encoding binary;
	puts -nonewline $F $body;
    } errmsg];

    if {$status != 0} {
	::syslog::err $errmsg;
    }

    if {[info exists F]} {
	set status [catch {
	    catch {close $F};
	} errmsg];

	if {$status != 0} {
	    ::syslog::err $errmsg;
	}
    }
}

proc proc_cspoolrpipe_exec {cmd} {

    set status [catch {
	eval exec $cmd;
    } errmsg];

    if {$status != 0} {
	::syslog::err $errmsg;
    }
}

## The common defaults
set defaults_file "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaults_file] == 0} {
    puts "$defaults_file not found.";
    return 1;
}
source $defaults_file;
unset defaults_file;

# packages
package require cmdline;
package require errx;
package require cspoolbdb;

#
# main
#
set usage {cspoolrpipe [-b] [-v] [-e <cmd>] [-p <cmd>] <key>};
set optlist {{b} {v} {e.arg ""} {p.arg ""}};
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$argc == 0} {
    puts stderr $usage;
    exit 1;
}
set key [lindex $argv 0];

if {$option(b) == 1} {
    ::syslog::usesyslog;
}

::cspoolbdb::init $common(cspoolbdbconf) $common(localconfdirs);

set status [catch {
    set result [::cspoolbdb::get $key];
} errmsg];

if {[::cspoolbdb::query_code_notfound]} {
    if {$option(v) == 1} {
	::syslog::msg $errmsg;
    }
    if {$option(e) ne ""} {
	proc_cspoolrpipe_exec $option(e);
    }
    return;
} elseif {[::cspoolbdb::query_code_error]} {
    ::syslog::err $errmsg;
}

set code [lindex $result 0];
set size [lindex $result 1];
set body [lindex $result 2];

if {$option(v) == 1} {
    ::syslog::msg "$key found in cspool.";
}
if {$option(p) eq ""} {
    fconfigure stdout -translation binary -encoding binary;
    puts -nonewline stdout $body;
} else {
    proc_cspoolrpipe_pipe $option(p) $body;
}
