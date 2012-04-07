#
# $Id$
#
# wstm utility functions
#

proc ::gempak::wstm_end_time {end_time} {

    variable gempak;

    set gempak(param,wstm.end_time) $end_time;
}

proc ::gempak::wstm_color_warn {warn_color} {

    variable gempak;

    set gempak(param,wstm.color_warn) $warn_color;
}

proc ::gempak::wstm_color_watch {watch_color} {

    variable gempak;

    set gempak(param,wstm.color_watch) $watch_color;
}

proc ::gempak::wstm_color_advisory {advisory_color} {

    variable gempak;

    set gempak(param,wstm.color_advisory) $advisory_color;
}

proc ::gempak::wstm_time_flag {time_flag} {

    variable gempak;

    set gempak(param,wstm.time_flag) $time_flag;
}

proc ::gempak::wstm_label_flag {label_flag} {

    variable gempak;

    set gempak(param,wstm.label_flag $label_flag;
}

proc ::gempak::wstm_outline_flag {outline_flag} {

    variable gempak;

    set gempak(param,wstm.outline_flag) $outline_flag;
}

proc ::gempak::wstm_outline_width_warn {warn_linewidth} {

    variable gempak;

    set gempak(param,wstm.outline_width_warn) $warn_linewidth;
}

proc ::gempak::wstm_outline_width_watch {watch_linewidth} {

    variable gempak;

    set gempak(param,wstm.outline_width_watch) $watch_linewidth;
}

proc ::gempak::wstm_outline_width_advisory {advisory_linewidth} {

    variable gempak;

    set gempak(param,wstm.outline_width_advisory) $advisory_linewidth;
}

proc ::gempak::set_wstm {} {

    variable gempak;

    # Join the colors
    set gempak(param,wstm.color) [::gempak::_join_subparam \
        "wstm" "color" ";" [list warn watch advisory]];

    # Join the linewidth and then join with the outline
    set gempak(param,wstm.outline_width) [::gempak::_join_subparam \
        "wstm" "outline_width" ";" [list warn watch advisory]];

    set gempak(param,wstm.outline) [::gempak::_join_subparam \
        "wstm" "outline" "/" [list flag width]];

    # All
    set parts [list end_time color time_flag label_flag outline];
    ::gempak::_set_param "wstm" "|" $parts;
}

proc ::gempak::get_wstm {} {

    return [::gempak::get "wstm"];
}

proc ::gempak::get_wstm_list {} {

    return [split [::gempak::get_wstm] "|"];
}
