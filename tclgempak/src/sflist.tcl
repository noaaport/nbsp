#
# $Id$
#
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

package provide gempak::sflist 1.0;
package require Expect;
package require cmdline;
package require textutil;
package require gempak;

namespace eval gempak::sflist {

    variable sflist;
    
    array set sflist {};
}

proc ::gempak::sflist::init {args} {
	
    variable sflist;

    _init;
    log_user 0;

    set sflist(prompt) "SFLIST>";

    set usage {::gempak::sflist::init [-v] [-k] [-s <separator>]};
    set optlist {{v} {k} {s.arg ","}};
    array set option [::cmdline::getoptions args $optlist $usage];

    set sflist(option,k) $option(k);
    set sflist(option,v) $option(v);
    set sflist(option,s) $option(s);
    
    if {$option(v) == 1} {
	log_user 1;
    }

    set status [catch {
	spawn sflist;
    } errmsg];
    if {$status != 0} {
	set sflist(output) $errmsg;
	return -code error $sflist(output);
    }

    set sflist(id) $spawn_id;
    expect $sflist(prompt);

    return 0;
}

proc ::gempak::sflist::end {} {

    variable sflist;
	
    _exec "exit";

    set status [catch {
	wait;
    } errmsg];

    if {$sflist(option,k) == 0} {
	catch {file delete last.nts gemglb.nts};
    }

    if {$status != 0} {
	return -code error $errmsg;
    }

    return 0;
}

proc ::gempak::sflist::run {} {

    variable sflist;

    foreach key [array names sflist "param,*"] {
	regexp {param,(.*)} $key match param_name;
	_exec "$param_name = $sflist($key)";
    }
    _exec "run";
}

proc ::gempak::sflist::_init {} {

    variable sflist;

    set sflist(output_raw) "";
    set sflist(output) "";
    set sflist(output_list) [list];
    set sflist(output_status) 0;
}

proc ::gempak::sflist::_exec {s} {

    variable sflist;
    global expect_out;

    set spawn_id $sflist(id);
    set status [catch {
	send "$s\r";
    } errmsg];
    if {$status != 0} {
	set sflist(output) $errmsg;
	return -code error $sflist(output);
    }

    if {$s eq "exit"} {
	return 0;
    } elseif {$s ne "run"} {
	expect $sflist(prompt);
	return 0;
    }

    # Match line by line in order to collect all the lines independently
    # of the expect output buffer size (Libes p. 145). Since here the
    # prompt is a real prompt (i.e., it is not followed by a "\n"),
    # it will not be matched by the "\n" pattern. In grads.tcl, for example,
    # the "prompt" that we look for is </IPC>, but since it is followed by
    # the "\n" (because it is not a "real" prompt), it cannot appear as
    # a separate matching pattern.

    set output [list];
    expect {
	-re {\n} {
	    lappend output $expect_out(buffer);
	    exp_continue;
	}
	$sflist(prompt);
    }

    set sflist(output_raw) [join $output "\n"];
    set sflist(output) "";
    set sflist(output_list) [list];
    set sflist(output_status) 0;

    set record "";
    set rc 0;
    foreach line $output {
	# Check for errors (warnings have positive code).
	if {[regexp {\[SF.* (-\d+)\]\s+(.*)$} $line match rc errmsg]} {
	    set sflist(output_status) $rc;
	    set sflist(output) $match;

	    return -code error $sflist(output);
	}

	# Data lines are indented. And continuation lines are indented
	# by about 24 spaces. After the lines are joined, the entire record
	# is split on blanks and then rejoined with the delimiter.

	if {[regexp {^[[:blank:]]{2,}} $line]} {
	    if {[regexp {^[[:blank:]]{24,}} $line]} {
		append record " " [string trim [string tolower $line]];
	    } else {
		if {$record ne ""} {
		    lappend sflist(output_list) \
			[join [::textutil::splitx $record] $sflist(option,s)];
		}
		set record [string trim [string tolower $line]];
	    }
	} elseif {$record ne ""} {
	    lappend sflist(output_list) \
		[join [::textutil::splitx $record] $sflist(option,s)];
	    set record "";
	}
    }

    set sflist(output) [join $sflist(output_list) "\n"];

    return 0;
}

proc ::gempak::sflist::output {} {

    variable sflist;

    return $sflist(output);
}

proc ::gempak::sflist::output_raw {} {

    variable sflist;

    return $sflist(output_raw);
}

proc ::gempak::sflist::output_list {} {

    variable sflist;

    return $sflist(output_list);
}

proc ::gempak::sflist::output_status {} {

    variable sflist;

    return $sflist(output_status);
}

proc ::gempak::sflist::define {param val} {

    variable sflist;

    set sflist(param,$param) $val;
}

proc ::gempak::sflist::get {param} {

    variable sflist;

    return $sflist(param,$param);
}

proc ::gempak::sflist::sfparm {args} {

    variable sflist;

    set sflist(param,sfparm) [join $args ";"];
}

proc ::gempak::sflist::dattim {val} {

    variable sflist;

    set sflist(param,dattim) $val;
}

proc ::gempak::sflist::sffile {val} {

    variable sflist;

    set sflist(param,sffile) $val;
}

proc ::gempak::sflist::stations {args} {

    variable sflist;

    set sflist(param,area) "";
    append sflist(param,area) "@" [join $args ";"];
}

proc ::gempak::sflist::state {state} {

    variable sflist;

    set sflist(param,area) "";
    append sflist(param,area) "@" $state;
}

proc ::gempak::sflist::latlon12 {lat1 lon1 lat2 lon2} {

    variable sflist;

    sflist(param,area) "";
    set append sflist(param,area) [join [list $lat1 $lon1 $lat2 $lon2] ";"];
}

proc ::gempak::sflist::lat {lat1 lat2} {

    variable sflist;

    set sflist(param,area.lat1) $lat1;
    set sflist(param,area.lat2) $lat2;
}

proc ::gempak::sflist::lon {lon1 lon2} {

    variable sflist;

    set sflist(param,area.lon1) $lon1;
    set sflist(param,area.lon2) $lon2;
}

proc ::gempak::sflist::set_latlon {} {

    variable sflist;

    set l [list];
    foreach k [list lat1 lon1 lat2 lon2] {
	if {[info exists sflist(param,area.$k)] == 0} {
	    set sflist(param,area.$k) "";
	}
	    
	append l $sflist(param,area.$k);
    }

    set sflist(param,area) [string trimright [join $l ";"] ";"];
}

#
# User defined variables
#
proc ::gempak::sflist::set_var {var_name var_val} {
        
    variable sflist;

    set sflist(var,$var_name) $var_val;
}

proc ::gempak::sflist::get_var {var_name} {

    variable sflist;

    return $sflist(var,$var_name);
}
