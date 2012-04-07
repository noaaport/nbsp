#
# $Id$
#
# sfparm utility functions
#
# The sfmpap settings are:
#
# sfparm = param:size:width[:type:headsiz];...
# colors = color;...
#
# These functions sfparm. The colors functions are in colors.tcl.
#
proc ::gempak::sfparm_name {name} {

    variable gempak;

    set gempak(param,sfparm.name) $name;
}

proc ::gempak::sfparm_size {size} {

    variable gempak;

    set gempak(param,sfparm.size) $size;
}

proc ::gempak::sfparm_width {width} {

    variable gempak;

    set gempak(param,sfparm.width) $width;
}

proc ::gempak::sfparm_type {type} {

    variable gempak;

    set gempak(param,sfparm.type) $type;
}

proc ::gempak::sfparm_headsiz {headsiz} {

    variable gempak;

    set gempak(param,sfparm.headsiz) $headsiz;
}

proc ::gempak::set_sfparm {} {

    set parts [list name size width type headsiz];

    ::gempak::_set_param "sfparm" ":" $parts;
}

proc ::gempak::get_sfparm {} {

    return [::gempak::get "sfparm"];
}

proc ::gempak::get_sfparm_list {} {

    return [split [::gempak::get_sfparm] ";"];
}

proc ::gempak::append_sfparm {} {
#
# Called instead of ::gempak::set_sfparm to append a map rather
# than resetting it. Save current value, reset and join the new
# value with the old.
#    
    set current [::gempak::get_sfparm];
    ::gempak::set_sfparm;
    if {$current ne ""} {
	append current ";" [::gempak::get_sfparm];
	::gempak::define "sfparm" $current;
    }
}

