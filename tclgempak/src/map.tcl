#
# $Id$
#
# map utility functions
#

proc ::gempak::map_color {color} {

    variable gempak;

    set gempak(param,map.color) $color;
}

proc ::gempak::map_dash {dash} {

    variable gempak;

    set gempak(param,map.dash) $dash;
}

proc ::gempak::map_width {width} {

    variable gempak;

    set gempak(param,map.width) $width;
}

proc ::gempak::map_filterflag {filterflag} {

    variable gempak;

    set gempak(param,map.filterflag) $filterflag;
}

proc ::gempak::set_map {} {

    set parts [list color dash width filterflag];

    ::gempak::_set_param "map" "/" $parts;
}

proc ::gempak::get_map {} {

    return [::gempak::get "map"];
}

proc ::gempak::get_map_list {} {

    return [split [::gempak::get_map] "+"];
}


proc ::gempak::append_map {} {
#
# Called instead of ::gempak::set_map to append a map rather than resetting it.
# Save current value, reset and join the new value with the old.
#    
    set current [::gempak::get_map];
    ::gempak::set_map;
    if {$current ne ""} {
	append current "+" [::gempak::get_map];
	::gempak::define "map" $current;
    }
}
