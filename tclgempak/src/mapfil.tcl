#
# $Id$
#
# These funcions exist to emphasize that the mapfil parameter
# is treated in the library as any other paramater. The library
# function that build script takes care of making the translation
# to the $mapfil gempak variable prior.
#
proc gempak::mapfil {args} {

    variable gempak;

    if {[llength $args] == 0} {
	return -code error "Usage: ::gempak::mapfil <map1> <map2> ...";
    }

    set gempak(param,mapfil) [join $args "+"];
}

proc gempak::get_mapfil {} {

    return [::gempak::get "mapfil"];
}
