#
# $Id$
#
# garea utility functions
#

proc gempak::garea_lat1 {lat} {

    variable gempak;

    set gempak(param,garea.lat1) $lat;
}

proc gempak::garea_lon1 {lon} {

    variable gempak;

    set gempak(param,garea.lon1) $lon;
}

proc gempak::garea_lat2 {lat} {

    variable gempak;

    set gempak(param,garea.lat2) $lat;
}

proc gempak::garea_lon2 {lon} {

    variable gempak;

    set gempak(param,garea.lon2) $lon;
}

proc gempak::set_garea {} {

    set parts [list lat1 lon1 lat2 lon2];

    ::gempak::_set_param "garea" ";" $parts;
}

proc gempak::get_garea {} {

    return [::gempak::get "garea"];
}

proc gempak::get_garea_list {} {

    return [split [::gempak::get_garea] ";"];
}
