#
# $Id$
#
proc metar::get_param_list {} {

    variable metar;

    set r [list];
    foreach key [array names ::metar::metar "param,*"] {
	regexp {param,(.+)} $key math s1;
	lappend r $s1;
    }
    return $r;
}

proc metar::get_param {param} {

    variable metar;

    if {[info exists metar(param,$param)]} {
	return $metar(param,$param);
    }

    return -code error "$param is not a variable.";
}

proc metar::get_flag_list {} {

    variable metar;

    set r [list];
    foreach key [array names ::metar::metar "flag,*"] {
	regexp {flag,(.+)} $key math s1;
	lappend r $s1;
    }
    return $r;
}

proc metar::get_flag {flag} {

    variable metar;

    if {[info exists metar(flag,$flag)]} {
	return $metar(flag,$flag);
    }

    return -code error "$flag is not a variable.";
}

proc metar::get_textitem_list {} {

    variable metar;

    set r [list];
    foreach key [array names ::metar::metar "text,*"] {
	regexp {text,(.+)} $key math s1;
	lappend r $s1;
    }
    return $r;
}

proc metar::get_textitem {item} {

    variable metar;

    if {[info exists metar(text,$item)]} {
	return $metar(text,$item);
    }

    return -code error "$item is not a variable.";
}

proc metar::get_var_list {} {

    variable metar;

    set r [list];
    foreach key [array names ::metar::metar "var,*"] {
	regexp {var,(.+)} $key math s1;
	lappend r $s1;
    }
    return $r;
}

proc metar::get_var {var} {

    variable metar;

    if {[info exists metar(var,$var)]} {
	return $metar(var,$var);
    }

    return -code error "$var is not a variable.";
}

proc metar::set_var {var val} {

    variable metar;

    set metar(var,$var) $val;
}
