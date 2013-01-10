#
# $Id$
#
# title utility functions
#

proc gempak::title_color {color} {

    variable gempak;

    set gempak(param,title.color) $color;
}

proc gempak::title_location {location} {

    variable gempak;

    set gempak(param,title.location) $location;
}

proc gempak::title_string {string {shorttitle ""}} {

    ::gempak::_join_param_parts "title.string" "/" $string $shorttitle;
}

proc gempak::set_title {} {

    set parts [list color location string];

    ::gempak::_set_param "title" "/" $parts;
}

proc gempak::get_title {} {

    return [::gempak::get "title"];
}

proc gempak::get_title_list {} {

    return [split [::gempak::get_title] "/"];
}
