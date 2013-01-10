#
# $Id$
#
# wcn utility functions
#

proc gempak::wcn_end_time {end_time} {

    variable gempak;

    set gempak(param,wcn.end_time) $end_time;
}

proc gempak::wcn_outline_color_thunderstorm {color} {

    variable gempak;

    set gempak(param,wcn.outline_color_thunderstorm) $color;
}

proc gempak::wcn_outline_color_tornado {color} {

    variable gempak;

    set gempak(param,wcn.outline_color_tornado) $color;
}

proc gempak::wcn_time_flag {flag} {

    variable gempak;

    set gempak(param,wcn.time_flag $flag;
}

proc gempak::wcn_label_flag {flag} {

    variable gempak;

    set gempak(param,wcn.label_flag $flag;
}

proc gempak::wcn_watch_number {watch_number} {

    variable gempak;

    set gempak(param,wcn.watch_number $watch_number;
}

proc gempak::wcn_color_code_flag {flag} {

    variable gempak;

    set gempak(param,wcn.color_code_flag $flag;
}

proc gempak::wcn_marker_flag {flag} {

    variable gempak;

    set gempak(param,wcn.marker_flag) $flag;
}

proc gempak::wcn_outline_flag {flag} {

    variable gempak;

    set gempak(param,wcn.outline_flag) $flag;
}

proc gempak::wcn_fill_flag {flag} {

    variable gempak;

    set gempak(param,wcn.fill_flag) $flag;
}

proc gempak::wcn_fill_color_thunderstorm {color} {

    variable gempak;

    set gempak(param,wcn.fill_color_thunderstorm) $color;
}

proc gempak::wcn_fill_color_tornado {color} {

    variable gempak;

    set gempak(param,wcn.fill_color_tornado) $color;
}

proc gempak::wcn_union_flag {flag} {

    variable gempak;

    set gempak(param,wcn.union_flag) $flag;
}

proc gempak::set_wcn {} {

    variable gempak;

    # Join the outline colors
    set gempak(param,wcn.outline_color) [::gempak::_join_subparam \
        "wcn" "outline_color" ";" [list thunderstorm tornado]];

    # Join the fill colors amd join it with the fill flag
    set gempak(param,wcn.fill_color) [::gempak::_join_subparam \
        "wcn" "fill_color" ";" [list thunderstorm tornado]];

    set gempak(param,wcn.fill) [::gempak::_join_subparam \
        "wcn" "fill" "/" [list flag color]];

    # All
    set parts [list end_time outline_color time_flag label_flag \
		   watch_number color_code_flag marker_flag outline_flag \
		   fill union_flag];
    ::gempak::_set_param "wcn" "|" $parts;
}

proc gempak::get_wcn {} {

    return [::gempak::get "wcn"];
}

proc gempak::get_wcn_list {} {

    return [split [::gempak::get_wcn] "|"];
}
