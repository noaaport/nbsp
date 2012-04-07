#
# $Id$
#
# Copyright (c) 2009 Jose F. Nieves <nieves@ltp.upr.clu.edu>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

package provide ssh 1.0;

package require cmdline;

namespace eval ssh {

    variable ssh;

    array set ssh {};
}

proc ::ssh::connect {args} {

    variable ssh;

    set usage {::ssh::connect [-t <tclsh_name>] [-- <ssh_options>]
	[<user>@]<host>};
    set optlist {{t.arg "tclsh"}};
    
    array set option [::cmdline::getoptions args $optlist $usage];
    set cmd [concat "|ssh" $args $option(t) 2>@ stdout];
    set F [open $cmd r+];

    set host [lindex $args end];
    if {[regexp {(.*)@(.*)} $host match s1 s2]} {
	set user $s1;
	set host $s2;
    }

    # These are the only internal variables (apart from the "user" variables).
    set ssh($host,F) $F;
    set ssh($host,script) [list];
}

proc ::ssh::disconnect {host} {

    variable ssh;

    _verify_connection $host;
    close $ssh($host,F);

    unset ssh($host,F);
    unset ssh($host,script);
}

proc ::ssh::push {host script} {

    variable ssh;

    _verify_connection $host;

    lappend ssh($host,script) $script;
}

proc ::ssh::send {host} {

    variable ssh;

    _verify_connection $host;

    set status [catch {
	puts $ssh($host,F) [join $ssh($host,script) "\n"];
	flush $ssh($host,F);
    } errmsg];

    set ssh($host,script) [list];

    if {$status != 0} {
	return -code error $errmsg;
    }
}

proc ::ssh::send_exit {host} {

    ::ssh::push $host "exit";
    ::ssh::send $host;
}

proc ::ssh::pop_line {host line_varname} {
#
# Returns: the same as "gets <filehandle> line"
#
    upvar $line_varname line; 
    variable ssh;
    
    _verify_connection $host;
    set r [gets $ssh($host,F) line];
    
    return $r;
}

proc ::ssh::pop_all {host output_varname} {
#
# Returns: 0 if eof before reading anything, or number of lines read.
#
    upvar $output_varname output; 
    variable ssh;
    
    _verify_connection $host;

    set r 0;
    set output_list [list];
    while {[::ssh::pop_line $host line] >= 0} {
	incr r;
	lappend output_list $line;
    }
    set output [join $output_list "\n"];

    return $r;
}

proc ::ssh::pop_read {host numbytes output_varname} {
#
# Returns: numbytes read. If numbytes is not positive, then read is
# called without the numbytes argument.
#
    upvar $output_varname output; 
    variable ssh;
    
    _verify_connection $host;
    
    if {$numbytes <= 0} {
	set output [read $ssh($host,F)];
    } else {
	set output [read $ssh($host,F) $numbytes];
    }

    return [string length $output];
}

proc ::ssh::hfileevent {host condition cmdlist} {

    variable ssh;

    _verify_connection $host;
    fileevent $ssh($host,F) $condition $cmdlist;
}

proc ::ssh::hfconfigure {host args} {
    
    variable ssh;

    _verify_connection $host;
    eval fconfigure $ssh($host,F) $args;
}

proc ::ssh::rexec {host script output_varname} {

    upvar $output_varname output;

    ::ssh::rexec_nopop $host $script;
    ::ssh::pop_all $host output;
}

proc ::ssh::rexec_nopop {host script} {

    ::ssh::push $host $script;
    ::ssh::send_exit $host;
}

#
# Utility
#
proc ::ssh::set_var {host var val} {
    
    variable ssh;

    _verify_connection $host;
    set ssh($host,user,$var) $val;
}

proc ::ssh::get_var {host var} {
    
    variable ssh;

    _verify_connection $host;

    if {[info exists ssh($host,user.$var)]} {
	return $ssh($host,user,$var);
    }

    return -code error "$var not defined";
}

#
# low level
#
proc ::ssh::get_filehandle {host} {

    variable ssh;

    _verify_connection $host;
    
    return $ssh($host,F);
}

#
# private
#
proc ::ssh::_verify_connection {host} {

    variable ssh;

    if {[info exists ssh($host,F)]} {
	return 1;
    }

    return -code error "There is no connection to $host.";
}
