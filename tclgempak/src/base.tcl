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

package provide gempak 1.0;
package require cmdline;

namespace eval gempak {

    variable gempak;
    
    array set gempak {};

    set gempak(_supported_programs) \
    {^(gp(front|map)|sf(cntr|map|list)|gd(cntr|stream|wind|plot|plot2|map))$};
}

proc gempak::init {args} {
	
    variable gempak;

    set usage {::gempak::init [-k] [-l <logfile>] [-v] <program>};
    set optlist {{k} {l.arg "/dev/null"} {v}};
 
   array set option [::cmdline::getoptions args $optlist $usage];

    if {[llength $args] == 0} {
	return -code error $usage;
    }
    set program [lindex $args 0];

    if {[regexp $gempak(_supported_programs) $program] == 0} {
	return -code error "Only $gempak(_supported_programs) are supported.";
    }

    if {$option(v)} {
	set F [open "|$program" w];
    } else {
	set F [open "|$program > $option(l)" w];
    }
    fconfigure $F -translation binary -encoding binary -buffering none;

    set gempak(F) $F;

    set gempak(progname) $program;
    set gempak(option_k) $option(k);

    return 0;
}

proc gempak::end {} {

    variable gempak;

    set status [catch {
	puts $gempak(F) "exit";
	close $gempak(F);
    } errmsg];

    catch {exec gpend};
    if {$gempak(option_k) == 0} {
	catch {file delete last.nts gemglb.nts};
    }
    
    if {$status != 0} {
	return -code error $errmsg;
    }

    return 0;
}

proc gempak::define {var value} {

    variable gempak;

    set gempak(param,$var) $value;
}

proc gempak::run {} {

    variable gempak;

    # Build script
    foreach a [array names gempak param,*] {
	if {[regexp {\.} $a]} {
	    continue;
	}
	set k [lindex [split $a ","] 1];
	append script "[::gempak::_script_translate $k] = $gempak($a)" "\n";
    }
    append script "run\n";

    set status [catch {
	puts $gempak(F) $script;
    } errmsg];

    if {$status != 0} {
	catch ::gempak::end;
	return -code error $errmsg;
    }

    return 0;
}

proc gempak::get {param_name} {

    variable gempak;

    return $gempak(param,$param_name);
}

#
# User defined variables
#
proc gempak::set_var {var_name var_val} {
	
    variable gempak;

    set gempak(var,$var_name) $var_val;
}

proc gempak::get_var {var_name} {

    variable gempak;

    return $gempak(var,$var_name);
}
