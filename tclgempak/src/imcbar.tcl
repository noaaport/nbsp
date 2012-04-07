#
# $Id$
#
# imcbar utility functions
#

proc ::gempak::imcbar_color {color} {

    variable gempak;

    set gempak(param,imcbar.color) $color;
}

proc ::gempak::imcbar_orientation {orientation} {

    variable gempak;

    set gempak(param,imcbar.orientation) $orientation;
}

proc ::gempak::imcbar_anchor {anchor} {

    variable gempak;

    set gempak(param,imcbar.anchor) $anchor;
}

proc ::gempak::imcbar_xy {x y} {

    ::gempak::_join_param_parts "imcbar.xy" ";" $x $y;
}

proc ::gempak::imcbar_lengthwidth {length width} {

    ::gempak::_join_param_parts "imcbar.lengthwidth" ";" $length $width;
}

proc ::gempak::imcbar_frequency {frequency} {

    variable gempak;

    set gempak(param,imcbar.frequency) $frequency;
}

proc ::gempak::set_imcbar {} {

    set parts [list color orientation anchor xy lengthwidth frequency];

    ::gempak::_set_param "imcbar" "/" $parts;
}

proc ::gempak::get_imcbar {} {

    return [::gempak::get "imcbar"];
}

proc ::gempak::get_imcbar_list {} {

    return [split [::gempak::get_imcbar] "/"];
}
