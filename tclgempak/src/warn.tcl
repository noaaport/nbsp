#
# $Id$
#
# warn utility functions
#

proc gempak::warn_end_time {end_time} {

    variable gempak;

    set gempak(param,warn.end_time) $end_time;
}

proc gempak::warn_color_tstorm {color} {

    variable gempak;

    set gempak(param,warn.color_tstorm) $color;
}

proc gempak::warn_color_tornado {color} {

    variable gempak;

    set gempak(param,warn.color_tornado) $color;
}

proc gempak::warn_color_flash_flood {color} {

    variable gempak;

    set gempak(param,warn.color_flash_flood) $color;
}

proc gempak::warn_time_flag {flag} {

    variable gempak;

    set gempak(param,warn.time_flag $flag;
}

proc gempak::warn_label_flag {flag} {

    variable gempak;

    set gempak(param,warn.label_flag $flag;
}

proc gempak::warn_outline_flag {flag} {

    variable gempak;

    set gempak(param,warn.outline_flag) $flag;
}

proc gempak::warn_poly_flag {flag} {

    variable gempak;

    set gempak(param,warn.poly_flag) $flag;
}

proc gempak::set_warn {} {

    variable gempak;

    # Join the colors
    set gempak(param,warn.color) [::gempak::_join_subparam "warn" "color" ";" \
				      [list tstorm tornado flash_flood]];

    # All
    set parts [list \
		   end_time color time_flag label_flag outline_flag poly_flag];
    ::gempak::_set_param "warn" "|" $parts;
}

proc gempak::get_warn {} {

    return [::gempak::get "warn"];
}

proc gempak::get_warn_list {} {

    return [split [::gempak::get_warn] "|"];
}
