#
# $Id$
#

# This function accepts all arguments (or a partial list) at once.
# The functions to set the various pieces individually follow below.
proc gempak::text {args} {

    variable gempak;

    set gempak(param,text) [join $args "/"];
}

proc gempak::text_size {size} {

    variable gempak;

    set gempak(param,text.size) $size;
}

proc gempak::text_font {font} {

    variable gempak;

    set gempak(param,text.font) $font;
}

proc gempak::text_width {width} {

    variable gempak;

    set gempak(param,text.witdh) $width;
}

proc gempak::text_border {border} {

    variable gempak;

    set gempak(param,text.border) $border;
}

proc gempak::text_rotation {value} {

    variable gempak;

    set gempak(param,text.rotation) $value;
}

proc gempak::text_justification {value} {

    variable gempak;

    set gempak(param,text.justification) $value;
}

proc gempak::text_hwflag {value} {

    variable gempak;

    set gempak(param,text.hwflag) $value;
}

proc gempak::set_text {} {

    variable gempak;

    # All
    set parts [list color font width border roration justification hwflag];
    ::gempak::_set_param "text" "/" $parts;
}

proc gempak::get_text {} {

    return [::gempak::get "text"];
}

proc gempak::get_text_list {} {

    return [split [::gempak::get_text] "/"];
}
