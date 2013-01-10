#
# $Id$
#

proc gempak::ijskip_i {args} {
#
# args should be in the order: start stop skip
#
    variable gempak;

    set gempak(param,ijskip.i) [join $args ";"];
}

proc gempak::ijskip_j {args} {
#
# args should be in the order: start stop skip
#
    variable gempak;

    set gempak(param,ijskip.j) [join $args ";"];
}

proc gempak::ijskip {iskip {jskip ""}} {

    variable gempak;

    set gempak(param,ijskip) $iskip;
    if {$jskip ne ""} {
	append gempak(param,ijskip) "/" $jskip;
    }
}

proc gempak::set_ijskip {} {

    variable gempak;

    # All parts
    set parts [list i j];
    ::gempak::_set_param "ijskip" "/" $parts;
}

proc gempak::get_ijskip {} {

    return [::gempak::get "ijskip"];
}

proc gempak::get_ijskip_list {} {

    return [split [::gempak::get_ijskip] "/"];
}
