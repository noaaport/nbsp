#
# $Id$
#
# svrl utility functions
#

proc ::gempak::svrl_end_time {end_time} {

    variable gempak;

    set gempak(param,svrl.end_time) $end_time;
}

proc ::gempak::svrl_color_thunderstorm {thunderstorm_color} {

    variable gempak;

    set gempak(param,svrl.color_thunderstorm) $thunderstorm_color;
}

proc ::gempak::svrl_color_tornado {tornado_color} {

    variable gempak;

    set gempak(param,svrl.color_tornado) $tornado_color;
}

proc ::gempak::svrl_time_flag {flag} {

    variable gempak;

    set gempak(param,svrl.time_flag) $flag;
}

proc ::gempak::svrl_label_flag {flag} {

    variable gempak;

    set gempak(param,svrl.label_flag) $flag;
}

proc ::gempak::svrl_outline_flag {flag} {

    variable gempak;

    set gempak(param,svrl.outline_flag) $flag;
}

proc ::gempak::svrl_color_code_flag {flag} {

    variable gempak;

    set gempak(param,svrl.color_code_flag_flag) $flag;
}

proc ::gempak::set_svrl {} {

    variable gempak;

    # Join the colors
    set gempak(param,svrl.color) [::gempak::_join_subparam "svrl" "color" ";" \
				      [list thunderstorm tornado]];

    # All
    set parts [list end_time color time_flag label_flag outline_flag \
		   color_code_flag];
    ::gempak::_set_param "svrl" "|" $parts;
}

proc ::gempak::get_svrl {} {

    return [::gempak::get "svrl"];
}

proc ::gempak::get_svrl_list {} {

    return [split [::gempak::get_svrl] "|"];
}
