#
# $Id$
#
# wcp utility functions
#

proc gempak::wcp_end_time {end_time} {

    variable gempak;

    set gempak(param,wcp.end_time) $end_time;
}

proc gempak::wcp_color_thunderstorm {thunderstorm_color} {

    variable gempak;

    set gempak(param,wcp.color_thunderstorm) $thunderstorm_color;
}

proc gempak::wcp_color_tornado {tornado_color} {

    variable gempak;

    set gempak(param,wcp.color_tornado) $tornado_color;
}

proc gempak::wcp_time_flag {flag} {

    variable gempak;

    set gempak(param,wcp.time_flag) $flag;
}

proc gempak::wcp_watch_number_flag {flag} {

    variable gempak;

    set gempak(param,wcp.watch_number_flag) $flag;
}

proc gempak::wcp_color_code_flag {flag} {

    variable gempak;

    set gempak(param,wcp.color_code_flag) $flag;
}

proc gempak::set_wcp {} {

    variable gempak;

    # Join the colors
    set gempak(param,wcp.color) [::gempak::_join_subparam \
        "wcp" "color" ";" [list thunderstorm tornado]];

    # All
    set parts [list \
        end_time color time_flag watch_number_flag color_code_flag];
    ::gempak::_set_param "wcp" "|" $parts;
}

proc gempak::get_wcp {} {

    return [::gempak::get "wcp"];
}

proc gempak::get_wcp_list {} {

    return [split [::gempak::get_wcp] "|"];
}
