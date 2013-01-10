#
# $Id$
#
proc gempak::param_names {} {
#
# Return the names of the main parameters (garea, imcbar, latlon, ...)
#
    variable gempak;

    array set a {};

    set l [list];
    foreach index [array names gempak "param,*"] {
	# Select the portion between the "," and ".", and add it to the list
	# if it has not been seen.
	regexp {param,([^.]+)\..+} $index match name;
	if {[info exists a($name)] == 0} {
	    set a($name) 1;  
	    lappend l $name;
	}
    }

    return $l;
}

proc gempak::param_vars {param_name} {
#
# Returns all the variables associated with a given parameter.
#
    variable gempak;

    set l [list];
    foreach index [array names gempak "param,${param_name}.*"] {
	set name [lindex [split $index ","] 1];
	lappend l $name;
    }

    return $l;
}

proc gempak::puts_param_vars {param} {

    foreach q [lsort [::gempak::param_vars $param]] {
	puts "$q = [::gempak::get $q]";
    }
}

proc gempak::puts_all_param_vars {} {

    foreach p [::gempak::param_names] {
	puts "$p = [::gempak::get $p]";
	::gempak::puts_param_vars $p;
    }
}
