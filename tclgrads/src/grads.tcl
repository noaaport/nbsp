#
# $Id$
#
# Copyright (c) 2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
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

package provide grads 1.1;

package require cmdline;
package require textutil;
package require struct::matrix;

namespace eval grads {

    variable grads;
    
    array set grads {};
}

proc ::grads::_init {} {

    variable grads;

    set grads(_expect_output_buffer) "";   # all output up to (and including)
                                           # the matching line

    set grads(output) "";
    set grads(output_list) [list];
    set grads(output_rc) 0;
    set grads(output_value) "";
}

proc ::grads::_expect {} {

    variable grads;

    set expect_output_buffer [list];

    set line "";
    while {[regexp $grads(reply_end) $line] == 0} {
        set r [gets $grads(F) line];
	lappend expect_output_buffer $line;
    }

    set grads(_expect_output_buffer) [join $expect_output_buffer "\n"];
}

proc ::grads::_send {s} {

    variable grads;

    set status [catch {
	puts $grads(F) $s;
	flush $grads(F);
    } errmsg];

    if {$status != 0} {
	return -code error $errmsg;
    }

    return 0;
}

proc ::grads::init {args} {
	
    variable grads;

    set usage {::grads::init [-portrait] [-undef <symbol>]};
    set optlist {{portrait} {undef.arg "?"}};

    _init;

    set grads(options) [list -b -u];
    set grads(reply_start) {<IPC>};
    set grads(reply_end) {</IPC>};

    array set option [::cmdline::getoptions args $optlist $usage];
    set grads(undef) $option(undef);
    if {$option(portrait) == 1} {
	lappend grads(options) "-p";
    } else {
	lappend grads(options) "-l";
    }
    
    set grads(output_value) $grads(undef);

    set cmd [concat "|grads" $grads(options)];
    set status [catch {
	set F [::open $cmd r+];
	#
	# In SL (6.1), gfs_161.tcl hangs if used with the binary options.
	# (FreeBSD and Ubuntu do not have the problem). Anyway the output
	# from Grads is pure text so the correct use should be without them.
	#
	# fconfigure $F -buffering line -translation binary -encoding binary;
	#
	fconfigure $F -buffering line;
    } errmsg];

    if {$status != 0} {
	set grads(output) $errmsg;
	return -code error $grads(output);
    }

    set grads(F) $F;
    _expect;		# expect the reply_end </IPC>

    return 0;
}

proc ::grads::end {} {

    variable grads;
	
    set status [catch {
	exec quit;
	_expect;
    } errmsg];

    # Any error here is ignored
    catch {close $grads(F)};

    if {$status != 0} {
	return -code error $errmsg;
    }

    return 0;
}

proc ::grads::exec {args} {

    variable grads;

    set cmd [lindex $args 0];
    append s [join $args] " ";

    set status [catch {
	_send $s;
    } errmsg];

    if {$status != 0} {
	set grads(output) $errmsg;
	return -code error $grads(output);
    }

    if {$cmd eq "quit"} {
	return 0;
    }

    _expect;

    if {[regexp {<IPC>[^\n]*\n(.*)<RC>\s*(\d+)\s*</RC>} \
	     $grads(_expect_output_buffer) match s1 s2]} {
	set grads(output) [string trim $s1];
	set grads(output_rc) $s2;
    } else {
	return -code error "Error parsing output buffer.";
    }

    set grads(output_list) [list];	# defined only if explicitly requested

    if {$grads(output_rc) != 0} {
	return -code error $grads(output);
    }

    if {[regexp {(V|v)alue\s+=\s+(-?\d+\.?\d*(e(-|\+)\d+)?)} \
	     $grads(output) match s1 v]} {
	set grads(output_value) $v;
    } else {
	set grads(output_value) $grads(undef);
    }

    return 0;
}

proc ::grads::open {file} {

    return [exec open $file];
}

proc ::grads::output_value {} {

    variable grads;

    return $grads(output_value);
}

proc ::grads::output {} {

    return $grads(output);
}

proc ::grads::output_list {} {

    variable grads;

    if {[llength $grads(output_list)] == 0} {
	set grads(output_list) [split $grads(output) "\n"];
    }
    
    return $grads(output_list);
}

proc ::grads::output_line {lineindex} {

    return [lindex [grads_output_list] $lineindex];
}

proc ::grads::output_word {lineindex wordindex} {

    set line [grads_output_line $lineindex];

    return [lindex [split $line] $wordindex];
}

proc ::grads::get_dimensions {array_name} {

    upvar $array_name a;
    variable grads;

    set a(xsize) $grads(undef);
    set a(ysize) $grads(undef);
    set a(zsize) $grads(undef);
    set a(tsize) $grads(undef);

    set a(lon1) $grads(undef);
    set a(lon2) $grads(undef);
    set a(lat1) $grads(undef);
    set a(lat2) $grads(undef);
    set a(lev1) $grads(undef);
    set a(lev2) $grads(undef);
    set a(time1) $grads(undef);
    set a(time2) $grads(undef);
    set a(x1) $grads(undef);
    set a(x2) $grads(undef);
    set a(y1) $grads(undef);
    set a(y2) $grads(undef);
    set a(z1) $grads(undef);
    set a(z2) $grads(undef);
    set a(t1) $grads(undef);
    set a(t2) $grads(undef);
 
    exec q file;

    if {[regexp {Xsize\s+=\s+(\d+)\s+Ysize\s+=\s+(\d+)\s+Zsize\s+=\s+(\d+)\s+Tsize\s+=\s+(\d+)\s+} $grads(output) match vx vy vz vt]} {
	set a(xsize) $vx;
	set a(ysize) $vy;
	set a(zsize) $vz;
	set a(tsize) $vt;
    } else {
	return -code error "Error parsing output buffer: $grads(output)";
    }

    exec q dims;

    foreach line [output_list] {
	set parts [::textutil::splitx $line];
	if {[regexp {varying\s+Lon} $line]} {
	    set a(lon1) [lindex $parts 5];
	    set a(lon2) [lindex $parts 7];
	    set a(x1) [lindex $parts 10];
	    set a(x2) [lindex $parts 12];
	} elseif {[regexp {fixed\s+Lon} $line]} {
	    set a(lon1) [lindex $parts 5];
	    set a(lon2) $a(lon1);
	    set a(x1) [lindex $parts 8];
	    set a(x2) $a(x1);
	} elseif {[regexp {varying\s+Lat} $line]} {
	    set a(lat1) [lindex $parts 5];
	    set a(lat2) [lindex $parts 7];
	    set a(y1) [lindex $parts 10];
	    set a(y2) [lindex $parts 12];
	} elseif {[regexp {fixed\s+Lat} $line]} {
	    set a(lat1) [lindex $parts 5];
	    set a(lat2) $a(lat1);
	    set a(y1) [lindex $parts 8];
	    set a(y2) $a(y1);
	} elseif {[regexp {varying\s+Lev} $line]} {
	    set a(lev1) [lindex $parts 5];
	    set a(lev2) [lindex $parts 7];
	    set a(z1) [lindex $parts 10];
	    set a(z2) [lindex $parts 12];
	} elseif {[regexp {fixed\s+Lev} $line]} {
	    set a(lev1) [lindex $parts 5];
	    set a(lev2) $a(lev1);
	    set a(z1) [lindex $parts 8];
	    set a(z2) $a(z1);
	} elseif {[regexp {varying\s+Time} $line]} {
	    set a(time1) [lindex $parts 5];
	    set a(time2) [lindex $parts 7];
	    set a(t1) [lindex $parts 10];
	    set a(t2) [lindex $parts 12];
	} elseif {[regexp {fixed\s+Time} $line]} {
	    set a(time1) [lindex $parts 5];
	    set a(time2) $a(time1);
	    set a(t1) [lindex $parts 8];
	    set a(t2) $a(t1);
	}
    }

    return 0;
}

proc ::grads::set_dimensions {array_name {vars xyzt}} {
#
# Used for restoring the ranges after the executing the functions that
# modify them (get_levels, get_times). "vars" is an optional argument
# to indicate which variables to restore, in the form of a combination
# of the letters xyzt. If it is absent, then all four variables are restored.
#
    upvar $array_name a;
    variable grads;

    foreach key [list x y z t] {
	if {[regexp $key $vars]} {
	    exec set $key $a(${key}1) $a(${key}2);
	}
    } 
}

proc ::grads::get_vars {var_list} {
#
# Returns a list in var_list (passed by name) with the names of the
# variables.
#
    upvar $var_list vars;
    variable grads;

    exec q file;

    set doline 0;
    set vars [list];
    foreach line [output_list] {
	if {[regexp {Number\s*of\s*Variables\s+=\s+\d+} $line]} {
	    set doline 1;
	    continue;
	} elseif {$doline == 0} {
	    continue;
	}
	lappend vars [lindex [split [string trimleft $line]] 0];
    }
    if {[llength $vars] == 0} {
	return -code error "No variable list found.";
    }

    return 0;
}

proc ::grads::get_levels {level_list args} {
#
# Returns all the z levels as a list. If args is "-r", then only
# the levels in the currently set range are returned.
# See also ::gradsu::levels {level_list}
#
    upvar $level_list levels;
    variable grads;
    set usage {::grads::get_levels level_list [-r]};
    set optlist {{r}};

    array set option [::cmdline::getoptions args $optlist $usage];

    get_dimensions a;

    set levels [list];

    if {$option(r) == 0} {
	set z 1;
	set last $a(zsize);
    } else {
	set z $a(z1);
	set last $a(z2);
    }

    while {$z <= $last} {
	exec set z $z;
	if {[regexp {LEV\s+set.+\s+(\d+)} $grads(output) match v]} {
	    lappend levels $v;
	}
	incr z;
    }

    # Restore levels range
    set_dimensions a z;

    return 0;
}

proc ::grads::get_times {time_list args} {
#
# Returns all the times as a list. args means the same as in get_levels.
# See also ::gradsu::times {time_list}
#
    upvar $time_list times;
    variable grads;

    set usage {::grads::get_times time_list [-r]};
    set optlist {{r}};
    array set option [::cmdline::getoptions args $optlist $usage];

    get_dimensions a;

    set times [list];

    if {$option(r) == 0} {
	set t 1;
	set last $a(tsize);
    } else {
	set t $a(t1);
	set last $a(t2);
    }
    while {$t <= $last} {
	exec set t $t;
	if {[regexp {Time\s+val.+set.*\s+(\d{4}:\d+:\d+:\d+)} $grads(output) \
		 match v]} {
	    lappend times $v;
	}
	incr t;
    }

    # Restore t
    set_dimensions a t;

    return 0;
} 

proc ::grads::get_lons {lon_list args} {
#
# Returns all the longitudes as a list. args means the same as in get_levels.
# See also ::gradsu::lons {lon_list}
#
    upvar $lon_list lon;
    variable grads;

    set usage {::grads::get_lons lon_list [-r]};
    set optlist {{r}};
    array set option [::cmdline::getoptions args $optlist $usage];

    get_dimensions a;

    set lon [list];

    if {$option(r) == 0} {
	set x 1;
	set last $a(xsize);
    } else {
	set x $a(x1);
	set last $a(x2);
    }
    while {$x <= $last} {
	exec set x $x;
	if {[regexp {LON\s+set.+\s+(-?\d+\.?\d*(e(-|\+)\d+)?)} \
		 $grads(output) match v]} {
	    lappend lon $v;
	} else {
	    return -code error "Error parsing output buffer.";
	}
	incr x;
    }

    # Restore range
    set_dimensions a x;

    return 0;
}

proc ::grads::get_lats {lat_list args} {
#
# Returns all the latitudes as a list. args means the same as in get_levels.
# See also ::gradsu::lats {lat_list}
#
    upvar $lat_list lat;
    variable grads;

    set usage {::grads::get_lats lat_list [-r]};
    set optlist {{r}};
    array set option [::cmdline::getoptions args $optlist $usage];

    get_dimensions a;

    set lat [list];

    if {$option(r) == 0} {
	set y 1;
	set last $a(ysize);
    } else {
	set y $a(y1);
	set last $a(y2);
    }
    while {$y <= $last} {
	exec set y $y;
	if {[regexp {LAT\s+set.+\s+(-?\d+\.?\d*(e(-|\+)\d+)?)} \
		 $grads(output) match v]} {
	    lappend lat $v;
	} else {
	    return -code error "Error parsing output buffer.";
	}
	incr y;
    }

    # Restore range
    set_dimensions a y;

    return 0;
}

proc ::grads::transform {cmd v1 v2 r1_name r2_name} {
#
# cmd = xy2w, xy2gr, w2xy, w2gr, gr2w, gr2xy
#
    upvar $r1_name r1;
    upvar $r2_name r2;
    variable grads;

    exec q $cmd $v1 $v2;

    set result [::textutil::splitx $grads(output)];
    set s1 [lindex $result 1];
    set s2 [lindex $result 4];
    set r1 [lindex $result 2];
    set r2 [lindex $result 5];
    if {($s1 ne "=") || ($s2 ne "=") || \
	    ([regexp {\-?\d+(.\d+)?(e(-|\+)\d+)?} $r1] == 0) || \
	    ([regexp {\-?\d+(.\d+)?(e(-|\+)\d+)?} $r2] == 0)} {
	return -code error "Error: Cannot parse output from $cmd.";
    }

    return 0;
}

proc ::grads::eval_expr {expression args} {

    set usage {::grads::eval_expr <expr> <var_name>
	[-s <input_sep>] [-S <output_sep>]};
    set optlist {{s.arg "|"} {S.arg ","}};
    array set option [::cmdline::getoptions args $optlist $usage];

    set exprlist [split $expression $option(s)];

    set r [_eval_expr_list $exprlist $option(S)];

    return $r;
}

proc ::grads::eval_expr1 {expression param args} {
#
# Here "param" is one of x,y,z,t. If args is "-r", then 
# the list is restricted to the currently set range for "param".
# "expression" is a Grads expression or a list  separated
# by the "|" character.
#
    set usage {::grads::eval_expr1 <expr> <param> [-r]
	[-s <input_sep>] [-S <output_sep>]};
    set optlist {{r} {s.arg "|"} {S.arg ","}};
    array set option [::cmdline::getoptions args $optlist $usage];

    set exprlist [split $expression $option(s)];

    get_dimensions a;

    set result [list];

    if {$option(r) == 0} {
	set p 1;
	set last $a(${param}size);
    } else {
	set p $a(${param}1);
	set last $a(${param}2);
    }
    while {$p <= $last} {
	exec set $param $p;
	lappend result [_eval_expr_list $exprlist $option(S)];
	incr p;
    }

    # Restore param
    set_dimensions a $param;

    return $result;
}

proc ::grads::eval_expr_xy {expression args} {
#
# The function assumes that z and t are fixed. It then
# evaluates <expression> for all values of x and y and returns
# a matrix "object" that can be manipulated with the struct::matrix package.
# If args is "-r", then the matrix is restricted to the currently set range
# for x and y.
#
# Without options the matrix is written like M_{xy}.
# The first (top) row of the matrix has all the values of y for the
# first value of x. The values of y increase from left to right.
# The second row then has all the values of y for the
# second value of x, and so on; the values of x increase from top to
# bottom. The "-t" option reverses x and y in these roles. The "-T" option
# implies "-t" and in addition the values of y decrease from top to bottom. 
#
# <expression> can be an expression list of the form "e1|e2|...|en", where
# each e1, ..., en is a single expression (that can be evaluated by GrADS).
# In that case each element of the matrix would contain the result of each
# expression separated by a "," or the character specified by -S.
# A separating char other than "|" in <expression> can be passed with
# the "-s" option. 
#

    set usage {::grads::eval_expr_xy <expr>[-r]
	[-s <input_sep>] [-S <output_sep>] [-t] [-T]};
    set optlist {{r} {s.arg "|"} {S.arg ","} {t} {T}};
    array set option [::cmdline::getoptions args $optlist $usage];

    get_dimensions a;

    set exprlist [split $expression $option(s)];

    if {$option(T) == 1} {
	set option(t) 1;
    }

    # Index 1 refers to the outer loop and 2 to the inner loop.
    if {$option(t) == 0} {
	set param1 "x";
	set param2 "y";
    } else {
	set param1 "y";
	set param2 "x";
    }

    if {$option(r) == 0} {
	set p1 1;
	set p2 1;
	set last1 $a(${param1}size);
	set last2 $a(${param2}size);
	set size1 $a(${param1}size);
	set size2 $a(${param2}size);
    } else {
	set p1 $a(${param1}1);
	set p2 $a(${param2}1);
	set last1 $a(${param1}2);
	set last2 $a(${param2}2);
	set size1 [expr $last1 - $p1 + 1];
	set size2 [expr $last2 - $p2 + 1];
    }

    set m [::struct::matrix];
    $m add rows $size1;
    $m add columns $size2;

    if {$option(T) == 0} {
	set row_index 0;
    } else {
	set row_index [expr $size1 - 1];
    }

    while {$p1 <= $last1} {
	exec set $param1 $p1;
	set row [list];
	set q2 $p2;
	while {$q2 <= $last2} {
	    exec set $param2 $q2;
	    lappend row [_eval_expr_list $exprlist $option(S)];
	    incr q2;
	}
	$m set row $row_index $row;
	if {$option(T) == 0} {
	    incr row_index;
	} else {
	    incr row_index -1;
	}
	incr p1;
    }

    # Restore params
    set_dimensions a ${param1}${param2};

    return $m;
}

#
# Internal functions
#

proc ::grads::_eval_expr {expression} {

    ::grads::exec d $expression;

    return [::grads::output_value];
}

proc ::grads::_eval_expr_list {expr_list {sep ","}} {
#
# This is an internal function used by eval_expr_xy.
#
    set s "";
    set r "";
    foreach e $expr_list {
	append r $s [_eval_expr $e];
	set s $sep;
    }

    return $r;
}

#
# Higher level utility functions
#
namespace eval gradsu {}

proc ::gradsu::display {args} {

    return [::grads::exec "display" [join $args]];
}

proc ::gradsu::draw {args} {

    return [::grads::exec "draw" [join $args]];
}

proc ::gradsu::printim {args} {

    return [::grads::exec "printim" [join $args]];
}

proc ::gradsu::clear {args} {

    return [::grads::exec "clear" [join $args]];
}

proc ::gradsu::reinit {} {

    return [::grads::exec "reinit"];
}

proc ::gradsu::reset {args} {

    return [::grads::exec "reset" [join $args]];
}

proc ::gradsu::mset {args} {

    foreach {name val} $args {
	set status [::grads::exec "set" $name $val];
	if {$status != 0} {
	    break;
	}
    }
    return $status;
}

proc ::gradsu::dims {array_name} {

    upvar $array_name a;

    return [::grads::get_dimensions "a"];
}

proc ::gradsu::levels {level_list args} {
#
# Returns the z levels as a list, with the first (0th) element of the list
# being the number of levels (the same a the value of zsize). This is to
# facilitate the relation with the "set z <value>" grads command, in which
# <value> goes from 1 to zsize.
#
    upvar $level_list levels;

    ::grads::get_levels "_levels" $args;
    set levels [linsert ${_levels} 0 [llength ${_levels}]];
}

proc ::gradsu::times {time_list args} {
#
# Returns the times as a list, with the first (0th) element of the list
# being the number of times (the same a the value of tsize). This is to
# facilitate the relation with the "set t <value>" grads command, in which
# <value> goes from 1 to tsize.
#
    upvar $time_list times;

    ::grads::get_times "_times" $args;
    set times [linsert ${_times} 0 [llength ${_times}]];
}

proc ::gradsu::lons {lon_list args} {
#
# Returns the longitudes as a list, with the first (0th) element of the list
# being the number of times (the same a the value of xsize). This is to
# facilitate the relation with the "set t <value>" grads command, in which
# <value> goes from 1 to xsize.
#
    upvar $lon_list lon;

    ::grads::get_lons "_lon" $args;
    set lon [linsert ${_lon} 0 [llength ${_lon}]];
}

proc ::gradsu::lats {lat_list args} {
#
# Similar to gradsu::lons, but for latitudes
#
    upvar $lat_list lat;

    ::grads::get_lats "_lat" $args;
    set lat [linsert ${_lat} 0 [llength ${_lat}]];
}

proc ::gradsu::coords {name coord_list args} {

    upvar $coord_list coords;

    set usage {::gradsu::coords <time|lev|lon|lat> <list_name> args};

    switch -glob $name {
	"time*" {times "coords" $args}
	"lev*" {levels "coords" $args}
	"lon*" {lons "coords" $args}
	"lat*" {lats "coords" $args}
	default {return -code error $usage}
    }

    return 0;
}

proc ::gradsu::cmd {args} {

    return [grads::exec $args];
}

proc ::gradsu::rline {linenumber} {

    set lineindex [expr $linenumber - 1];

    return [lindex [::grads::output_line $lineindex]];
}

proc ::gradsu::rword {linenumber wordnumber} {

    set lineindex [expr $linenumber - 1];
    set wordindex [expr $wordnumber - 1];

    return [::grads::output_word $lineindex $wordindex];
}

proc gradsu::getval {expression var_name args} {

    upvar $var_name var;

    set var [eval ::grads::eval_expr $expression [join $args " "]];
}

proc gradsu::getval1 {expression param var_name args} {
#
# Here "param" is one of x,y,z,t. The list returned is one-base indexed.
# If args is "-r" only the currently set of range of values of <param>
# is used, otherwise all values are used. "expression" has the same meaning
# as in grads::eval_expr1.
#
    upvar $var_name var;

    set _var [eval grads::eval_expr1 $expression $param [join $args " "]];
    set var [linsert ${_var} 0 [llength ${_var}]];
}

proc gradsu::getval2 {expression param1 param2 var_name args} {

    upvar $var_name var;

    set usage {::gradsu::getval2 <expr> <var_name> [-r] [-s <input_sep>]
	[-S <output_sep>]};
    set optlist {{r} {s.arg "|"} {S.arg ","}};
    array set option [::cmdline::getoptions args $optlist $usage];

   ::grads::get_dimensions a;

    set exprlist [split $expression $option(s)];

    if {$option(r) == 0} {
	set p1 1;
	set p2 1;
	set last1 $a(${param1}size);
	set last2 $a(${param2}size);
	set size1 $a(${param1}size);
	set size2 $a(${param2}size);
    } else {
	set p1 $a(${param1}1);
	set p2 $a(${param2}1);
	set last1 $a(${param1}2);
	set last2 $a(${param2}2);
	set size1 [expr $last1 - $p1 + 1];
	set size2 [expr $last2 - $p2 + 1];
    }

    set var [list];
    while {$p1 <= $last1} {
	::grads::exec set $param1 $p1;
	set row [list];
	set q2 $p2;
	while {$q2 <= $last2} {
	    ::grads::exec set $param2 $q2;
	    lappend row [::grads::_eval_expr_list $exprlist $option(S)];
	    incr q2;
	}
	lappend var $row;
	incr p1;
    }

    # Restore params
    ::grads::set_dimensions a ${param1}${param2};
}
