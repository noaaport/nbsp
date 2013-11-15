#
# $Id$
#
# Copyright (c) 2013 Jose F. Nieves <nieves@ltp.uprrp.edu>
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

package provide nbsp::ldm 1.0;

package require cmdline;

namespace eval nbsp::ldm {

    variable nbspldm;

    array set nbspldm {};
}

proc nbsp::ldm::open {args} {

    variable nbspldm;

    set usage {::nbspldm::open [-c ccbsize] [-d sep] [-f feedtype] [-g] [-n]
	[-o origin] [-q pqfname]};

    set optlist {{c.arg 24} {d.arg ":"} {f.arg 15} g n {o.arg "nbsp"}
	{q.arg "/var/ldm/ldm.pq"}};

    array set option [::cmdline::getoptions args $optlist $usage];

    set cmd [list "|nbsp2ldm" -b -c $option(c) -d $option(d) -f $option(f)];

    if {$option(g) == 1} {
	lappend cmd "-g";
    }

    if {$option(n) == 1} {
	lappend cmd "-n";
    }

    lappend cmd "-o" $option(o) "-q" $option(q);

    set F [::open $cmd w];

    # parameters
    set nbspldm(delimiter) $option(d);
    set nbspldm(cmd) $cmd;

    # variables
    set nbspldm(F) $F;
    set nbspldm(script) [list];
}

proc nbsp::ldm::close {} {

    variable nbspldm;

    _verify_connection;

    set status [catch {
	::close $nbspldm(F);
    } errmsg];

    unset nbspldm(F);
    unset nbspldm(script);

    if {$status != 0} {
	return -code error $errmsg;
    }
}

proc nbsp::ldm::reopen {} {

    variable nbspldm;

    set saved_script $nbspldm(script);

    set status [catch {
	nbsp::ldm::close;
    } errmsg];

    set F [::open $nbspldm(cmd) w];
    set nbspldm(F) $F;
    set nbspldm(script) [list];
    
    nbsp::ldm::push_list $saved_script;
}

proc nbsp::ldm::push {option {value ""}} {

    variable nbspldm;

    lappend nbspldm(script) $option;
    if {$value ne ""} {
	lappend nbspldm(script) $value;
    }
}

proc nbsp::ldm::push_args {args} {

    variable nbspldm;

    set nbspldm(script) [concat $nbspldm(script) $args];
}

proc nbsp::ldm::push_list {var} {

    variable nbspldm;

    set nbspldm(script) [concat $nbspldm(script) $var];
}

proc nbsp::ldm::push_opt_ccbsize {ccbsize} {

    ::nbsp::ldm::push "-c" $ccbsize;
}

proc nbsp::ldm::push_opt_feedtype {feedtype} {

    ::nbsp::ldm::push "-f" $feedtype;
}

proc nbsp::ldm::push_opt_addgpheader {} {

    ::nbsp::ldm::push "-g";
}

proc nbsp::ldm::push_opt_noccb {} {

    ::nbsp::ldm::push "-n";
}

proc nbsp::ldm::push_opt_origin {origin} {

    ::nbsp::ldm::push "-o" $origin;
}

proc nbsp::ldm::push_opt_prodid {prodid} {

    ::nbsp::ldm::push "-p" $prodid;
}

proc nbsp::ldm::push_opt_md5seqstr {str} {

    ::nbsp::ldm::push "-s" $str;
}

proc nbsp::ldm::send {} {

    variable nbspldm;

    _verify_connection;

    set status [catch {
	puts $nbspldm(F) \
	    [join [concat "nbsp2ldm" $nbspldm(script)] $nbspldm(delimiter)];
	flush $nbspldm(F);
    } errmsg];

    set nbspldm(script) [list];

    if {$status != 0} {
	return -code error $errmsg;
    }
}

#
# private
#
proc nbsp::ldm::_verify_connection {} {

    variable nbspldm;

    if {[info exists nbspldm(F)]} {
	return 1;
    }

    return -code error "There is no pipe to nbsp2ldm.";
}
