#
# $Id$
#
# Copyright (c) 2009 Jose F. Nieves <nieves@ltp.uprrp.edu>
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

package provide gempak::gdinfo 1.0;
package require Expect;
package require cmdline;
package require textutil;

namespace eval gempak::gdinfo {

    variable gdinfo;
    
    array set gdinfo {};
}

proc gempak::gdinfo::init {args} {
	
    variable gdinfo;

    _init;
    log_user 0;

    set gdinfo(prompt) "GDINFO>";

    set usage {::gempak::gdinfo::init [-v] [-k] [-f] [-s <separator>]};
    set optlist {{v} {k} {f} {s.arg ","}};
    array set option [::cmdline::getoptions args $optlist $usage];

    set gdinfo(option,k) $option(k);
    set gdinfo(option,v) $option(v);
    set gdinfo(option,f) $option(f);
    set gdinfo(option,s) $option(s);

    
    if {$option(v) == 1} {
	log_user 1;
    }

    set status [catch {
	spawn gdinfo;
    } errmsg];
    if {$status != 0} {
	set gdinfo(output) $errmsg;
	return -code error $gdinfo(output);
    }

    set gdinfo(id) $spawn_id;
    expect $gdinfo(prompt);

    return 0;
}

proc gempak::gdinfo::end {} {

    variable gdinfo;
	
    _exec "exit";

    set status [catch {
	wait;
    } errmsg];

    if {$gdinfo(option,k) == 0} {
	catch {file delete last.nts gemglb.nts};
    }

    if {$status != 0} {
	return -code error $errmsg;
    }

    return 0;
}

proc gempak::gdinfo::run {} {

    variable gdinfo;

    foreach key [array names gdinfo "param,*"] {
	regexp {param,(.*)} $key match param_name;
	_exec "$param_name = $gdinfo($key)";
    }
    _exec "run";
}

proc gempak::gdinfo::_init {} {

    variable gdinfo;

    set gdinfo(output_raw) "";
    set gdinfo(output) "";
    set gdinfo(output_list) [list];
    set gdinfo(output_status) 0;
}

proc gempak::gdinfo::_exec {s} {

    variable gdinfo;
    global expect_out;

    set spawn_id $gdinfo(id);
    set status [catch {
	send "$s\r";
    } errmsg];
    if {$status != 0} {
	set gdinfo(output) $errmsg;
	return -code error $gdinfo(output);
    }

    if {$s eq "exit"} {
	return 0;
    } elseif {$s ne "run"} {
	expect $gdinfo(prompt);
	return 0;
    }

    set done 0;
    set output "";
    while {$done == 0} {
	expect {
	    "EXIT:" {
		append output $expect_out(buffer);
		send "\r";
	    }
	    $gdinfo(prompt) {
		append output $expect_out(buffer); 
		set done 1;
	    }
	}
    }

    set gdinfo(output_raw) $output;
    set gdinfo(output) "";
    set gdinfo(output_list) [list];
    # This function computes the inventory and files the output and output_list
    # variables.
    _output_inventory;

    return 0;
}

proc gempak::gdinfo::output {} {

    variable gdinfo;

    return $gdinfo(output);
}

proc gempak::gdinfo::output_raw {} {

    variable gdinfo;

    return $gdinfo(output_raw);
}

proc gempak::gdinfo::output_list {} {

    variable gdinfo;

    if {[llength $gdinfo(output_list)] == 0} {
	set output [::gempak::gdinfo::output];
	set gdinfo(output_list) [split $output "\n"];
    }
    
    return $gdinfo(output_list);
}

proc gempak::gdinfo::define {param val} {

    variable gdinfo;

    set gdinfo(param,$param) $val;
}

proc gempak::gdinfo::get {param} {

    variable gdinfo;

    return $gdinfo(param,$param);
}

proc gempak::gdinfo::gdfile {gdfile} {

    variable gdinfo;

    set gdinfo(param,gdfile) $gdfile;
}

proc gempak::gdinfo::get_gdfile {} {

    return [::gempak::gdinfo::get "gdfile"];
}

proc gempak::gdinfo::_inventory {vars_array} {

    upvar $vars_array vars;

    set regex {\d{6}/\d{4}F\d{3}};

    set output [split [output_raw] "\n"];
    foreach line $output {
	if {[regexp $regex $line] == 0} {
	    continue;
	}
	set parts [::textutil::splitx [string trim $line]];
	set f [string tolower [lindex $parts 1]];	# forecast date
	set l [string tolower [lindex $parts 2]];	# level
	set var [string tolower [lindex $parts end]];	# var name
	set u [string tolower [lindex $parts end-1]];	# level coord
	set l2 [lindex $parts end-2];
	if {$l2 != $l} {
	    append l "-" $l2;
	}
	lappend vars(${var}_${l}_${u}) $f;
    }

    # If the vars arrays is empty, there was an error from gdinfo. In this
    # case we attempt to find the error and return it.
    if {[llength [array names vars]] == 0} {
	set errmsg [list];
	foreach line $output {
	    if {[regexp {\[.+\s+\d+\]} $line]} {
		lappend errmsg $line;
	    }
	}
	return -code error [join $errmsg "\n"];
    }

    return 0;
}

proc gempak::gdinfo::_output_inventory {} {
#
# This function puts each variable in its own line.
#
    variable gdinfo;

    array set vars {};
    _inventory vars;

    set gdinfo(output_list) [list];
    foreach v [lsort [array names vars]] {
	set record [list];
	lappend record $v;
	foreach h $vars($v) {
	    if {$gdinfo(option,f)} {
		lappend record $h;
	    } else {
		lappend record [string range $h end-2 end];
	    }
	}
	lappend gdinfo(output_list) [join $record $gdinfo(option,s)];
    }
    
    set gdinfo(output) [join $gdinfo(output_list) "\n"];
}

#
# User defined variables
#
proc gempak::gdinfo::set_var {var_name var_val} {
        
    variable gdinfo;

    set gdinfo(var,$var_name) $var_val;
}

proc gempak::gdinfo::get_var {var_name} {

    variable gdinfo;

    return $gdinfo(var,$var_name);
}
