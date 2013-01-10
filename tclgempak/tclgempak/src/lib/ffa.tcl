#
# $Id$
#
# ffa utility functions
#

proc gempak::ffa_end_time {end_time} {

    variable gempak;

    set gempak(param,ffa.end_time) $end_time;
}

proc gempak::ffa_color_ff {color} {

    variable gempak;

    set gempak(param,ffa.color_ff) $color;
}

proc gempak::ffa_color_fa {color} {

    variable gempak;

    set gempak(param,ffa.color_fa) $color;
}

proc gempak::ffa_time_flag {flag} {

    variable gempak;

    set gempak(param,ffa.time_flag) $flag;
}

proc gempak::ffa_label_flag {flag} {

    variable gempak;

    set gempak(param,ffa.label_flag) $flag;
}

proc gempak::ffa_immediate_cause_flag {flag} {

    variable gempak;

    set gempak(param,ffa.immediate_cause_flag) $flag;
}

proc gempak::ffa_outline_flag {flag} {

    variable gempak;

    set gempak(param,ffa.outline_flag) $flag;
}

proc gempak::ffa_outline_width_ff {linewidth} {

    variable gempak;

    set gempak(param,ffa.outline_width_ff) $linewidth;
}

proc gempak::ffa_outline_width_fa {linewidth} {

    variable gempak;

    set gempak(param,ffa.outline_width_fa) $linewidth;
}

proc gempak::set_ffa {} {

    variable gempak;

    # Join the colors
    set gempak(param,ffa.color) \
	[::gempak::_join_subparam "ffa" "color" ";" [list ff fa]];

    # Join the outline widths and the join it with the outline flag
    set gempak(param,ffa.outline_width) \
	[::gempak::_join_subparam "ffa" "outline_width" ";" [list ff fa]];

    set gempak(param,ffa.outline) \
	[::gempak::_join_subparam "ffa" "outline" "/" [list flag width]];

    # All
    set parts [list end_time color time_flag label_flag immediate_cause_flag \
		   outline];
    ::gempak::_set_param "ffa" "|" $parts;
}

proc gempak::get_ffa {} {

    return [::gempak::get "ffa"];
}

proc gempak::get_ffa_list {} {

    return [split [::gempak::get_ffa] "|"];
}
