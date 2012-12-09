#
# $Id$
#
# Auxiliary functions
#
proc gempak::_set_param {param separator parts_list} {

    variable gempak;

    set l [list];
    foreach k $parts_list {
	set p "param,";
	append p $param "." $k;
	if {[info exists gempak($p)] == 0} {
	    set gempak($p) "";
	}
	lappend l $gempak($p);
    }

    # If the last field is "" trim the ending "separator".
    set gempak(param,$param) \
	[string trimright [join $l $separator] $separator];
}

proc gempak::_join_subparam {param subparam separator subparts_list} {
#
# Used to join several parts of a subparameter in the set_<param>
# functions. For example (ffa.tcl)
#
#   set gempak(param,ffa.color) [_join_subparam "ffa" "color" ";" [list ff fa]]
#
# joins the values of "ffa.color_ff" and "ffa.color.ff" into "ffa.color",
#
# separated by a semicolon.

    variable gempak;

    set l [list];
    foreach k $subparts_list {
	set p "param,";
	append p $param "." $subparam "_" $k;
	if {[info exists gempak($p)] == 0} {
	    set gempak($p) "";
	}
	lappend l $gempak($p);
    }

    # If the last field is "" trim the ending "separator".
    set gempak(param,${param}.${subparam}) \
	[string trimright [join $l $separator] $separator];
}

proc gempak::_join_param_parts {param separator args} {
#
# Used to join sevaral parts of a parameter whose values are passed
# as arguments to a function. For example (latlon.tcl)
#
# ::gempak::_join_param_parts "latlon.freq" ";" $xfreq $yfreq;
#
    variable gempak;

    if {[llength $args] == 0} {
	set gempak(param,$param) "";
	return;
    }

    # If the last field is "" trim the ending "separator".
    set gempak(param,$param) \
	[string trimright [join $args $separator] $separator];

}

proc gempak::_script_translate {varname} {
#
# The "mapfil" parameter is a gempak variable and must appear in
# in the script as $mapfil. We have two options: Use $mapfil
# as the parameter name everywhere in the library and be careful
# about quoting it, or use it as just "mapfil" everywhere in the library
# just translate it just before inserting in the script. We have decided
# to the do the latter and encapsulate the translation in this function.
#
    set v [string tolower $varname];
    if {$v eq "mapfil"} {
	append r {$} $varname;
	return $r;
    }
    return $varname;
}
