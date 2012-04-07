#
# $Id$
#
# REFVEC specifies the size and location on the screen of the
# reference arrow using the following format:
#
# Magnitude; x; y; text size/font/width/HW; string

proc ::gempak::refvec_magnitude {magnitude} {

    variable gempak;

    set gempak(param,refvec.magnitude) $magnitude;
}

proc ::gempak::refvec_xy {x y} {

    variable gempak;

    set gempak(param,refvec.x) $x;
    set gempak(param,refvec.y) $y;
}

proc ::gempak::refvec_text_size {size} {

    variable gempak;

    set gempak(param,refvec.text_size) $size;
}

proc ::gempak::refvec_text_font {font} {

    variable gempak;

    set gempak(param,refvec.text_font) $width;
}

proc ::gempak::refvec_text_width {width} {

    variable gempak;

    set gempak(param,refvec.text_width) $width;
}

proc ::gempak::refvec_text_hwflag {flag} {

    variable gempak;

    set gempak(param,refvec.text_hwflag) $flag;
}

proc ::gempak::refvec_string {string} {

    variable gempak;

    set gempak(param,refvec.string) $string;
}

proc ::gempak::set_refvec {} {

    variable gempak;

    # The text parts
    set gempak(param,refvec.text) [::gempak::_join_subparam \
        "refvec" "text" "/" [list size font width hwflag]];

    # All
    set parts [list magnitude x y text string];
    ::gempak::_set_param "refvec" ";" $parts;
}

proc ::gempak::get_refvec {} {

    return [::gempak::get "refvec"];
}

proc ::gempak::get_refvec_list {} {

    return [split [::gempak::get_refvec] ";"];
}
