#
# $Id$
#
# watch utility functions
#
proc ::gempak::watch_end_time {end_time} {

    variable gempak;

    set gempak(param,watch.end_time) $end_time;
}

proc ::gempak::watch_color_thunderstorm {thunderstorm_color} {

    variable gempak;

    set gempak(param,watch.color_thunderstorm) $thunderstorm_color;
}

proc ::gempak::watch_color_tornado {tornado_color} {

    variable gempak;

    set gempak(param,watch.color_tornado) $tornado_color;
}

proc ::gempak::watch_color_status {status_color} {

    variable gempak;

    set gempak(param,watch.color_status) $status_color;
}

proc ::gempak::watch_time_flag_watch_box {flag} {

    variable gempak;

    set gempak(param,watch.time_flag_watch_box) $flag;
}

proc ::gempak::watch_time_flag_status_line {flag} {

    variable gempak;

    set gempak(param,watch.time_flag_status_line) $flag;
}

proc ::gempak::watch_number_flag_watch_box {flag} {

    variable gempak;

    set gempak(param,watch.number_flag_watch_box) $flag;
}

proc ::gempak::watch_number_flag_status_line {flag} {

    variable gempak;

    set gempak(param,watch.number_flag_status_line) $flag;
}

proc ::gempak::watch_color_code_flag {flag} {

    variable gempak;

    set gempak(param,watch.color_code_flag) $flag;
}

proc ::gempak::watch_most_recent_status_line_flag {flag} {

    variable gempak;

    set gempak(param,watch.most_recent_status_line_flag) $flag;
}

proc ::gempak::set_watch {} {

    variable gempak;

    # Join the colors
    set gempak(param,watch.color) [::gempak::_join_subparam \
       "watch" "color" ";" [list thunderstorm tornado status]];
				       
    # Join the time flags
    set gempak(param,watch.time_flag) [::gempak::_join_subparam \
	"watch" "time_flag" ";" [list watch_box status_line]];

    # Join the number flags
    set gempak(param,watch.number_flag) [::gempak::_join_subparam \
	 "watch" number_flag" ";" [list watch_box status_line]];

    # All
    set parts [list end_time color time_flag number_flag color_code_flag \
		   most_recent_status_line_flag];
    ::gempak::_set_param "watch" "|" $parts;
}

proc ::gempak::get_watch {} {

    return [::gempak::get "watch"];
}

proc ::gempak::get_watch_list {} {

    return [split [::gempak::get_watch] "|"];
}
