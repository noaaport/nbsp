#
# $Id$
#
# wou utility functions
#

proc ::gempak::wou_end_time {end_time} {

    variable gempak;

    set gempak(param,wou.end_time) $end_time;
}

proc ::gempak::wou_outline_color_thunderstorm {color} {

    variable gempak;

    set gempak(param,wou.outline_color_thunderstorm) $color;
}

proc ::gempak::wou_outline_color_tornado {color} {

    variable gempak;

    set gempak(param,wou.outline_color_tornado) $color;
}

proc ::gempak::wou_time_flag {flag} {

    variable gempak;

    set gempak(param,wou.time_flag $flag;
}

proc ::gempak::wou_label_flag {flag} {

    variable gempak;

    set gempak(param,wou.label_flag $flag;
}

proc ::gempak::wou_watch_number {watch_number} {

    variable gempak;

    set gempak(param,wou.watch_number $watch_number;
}

proc ::gempak::wou_color_code_flag {flag} {

    variable gempak;

    set gempak(param,wou.color_code_flag $flag;
}

proc ::gempak::wou_marker_flag {flag} {

    variable gempak;

    set gempak(param,wou.marker_flag) $flag;
}

proc ::gempak::wou_outline_flag {flag} {

    variable gempak;

    set gempak(param,wou.outline_flag) $flag;
}

proc ::gempak::wou_fill_flag {flag} {

    variable gempak;

    set gempak(param,wou.fill_flag) $flag;
}

proc ::gempak::wou_fill_color_thunderstorm {color} {

    variable gempak;

    set gempak(param,wou.fill_color_thunderstorm) $color;
}

proc ::gempak::wou_fill_color_tornado {color} {

    variable gempak;

    set gempak(param,wou.fill_color_tornado) $color;
}

proc ::gempak::wou_union_flag {flag} {

    variable gempak;

    set gempak(param,wou.union_flag) $flag;
}

proc ::gempak::set_wou {} {

    variable gempak;

    # Join the outline colors
    set gempak(param,wou.outline_color) [::gempak::_join_subparam \
        "wou" "outline_color" ";" [list thunderstorm tornado]];

    # Join the fill colors amd join it with the fill flag
    set gempak(param,wou.fill_color) [::gempak::_join_subparam \
        "wou" "fill_color" ";" [list thunderstorm tornado]];

    set gempak(param,wou.fill) [::gempak::_join_subparam \
        "wou" "fill" "/" [list flag color]];

    # All
    set parts [list end_time outline_color time_flag label_flag \
		   watch_number color_code_flag marker_flag outline_flag \
		   fill union_flag];
    ::gempak::_set_param "wou" "|" $parts;
}

proc ::gempak::get_wou {} {

    return [::gempak::get "wou"];
}

proc ::gempak::get_wou_list {} {

    return [split [::gempak::get_wou] "|"];
}
