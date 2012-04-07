#
# $Id$
#
# stnplt utility functions
#

proc ::gempak::stnplt_text_color {color} {

    variable gempak;

    set gempak(param,stnplt.text_color) $color;
}

proc ::gempak::stnplt_text_size {size} {

    variable gempak;

    set gempak(param,stnplt.text_size) $size;
}

proc ::gempak::stnplt_text_font {font} {

    variable gempak;

    set gempak(param,stnplt.text_font) $font;
}

proc ::gempak::stnplt_text_width {width} {

    variable gempak;

    set gempak(param,stnplt.text_width) $width;
}

proc ::gempak::stnplt_text_border {border} {

    variable gempak;

    set gempak(param,stnplt.text_border) $border;
}

proc ::gempak::stnplt_text_rotation {rotation} {

    variable gempak;

    set gempak(param,stnplt.text_rotation) $rotation;
}

proc ::gempak::stnplt_text_justification {justification} {

    variable gempak;

    set gempak(param,stnplt.text_justification) $justification;
}

proc ::gempak::stnplt_text_hwflag {hwflag} {

    variable gempak;

    set gempak(param,stnplt.text_hwflag) $hwflag;
}

proc ::gempak::stnplt_marker_color {color} {

    variable gempak;

    set gempak(param,stnplt.marker_color) $color;
}

proc ::gempak::stnplt_marker_type {type} {

    variable gempak;

    set gempak(param,stnplt.marker_type) $type;
}

proc ::gempak::stnplt_marker_size {size} {

    variable gempak;

    set gempak(param,stnplt.marker_size) $size;
}

proc ::gempak::stnplt_marker_width {width} {

    variable gempak;

    set gempak(param,stnplt.marker_width) $width;
}

proc ::gempak::stnplt_marker_hwflag {hwflag} {

    variable gempak;

    set gempak(param,stnplt.marker_hwflag) $hwflag;
}

proc ::gempak::stnplt_stnfile_name {stnfile_name} {

    variable gempak;

    set gempak(param,stnplt.stnfile_name) $stnfile_name;
}

proc ::gempak::stnplt_stnfile_column {stnfile_column} {

    variable gempak;

    set gempak(param,stnplt.stnfile_column) $stnfile_column;
}

proc ::gempak::set_stnplt {} {

    # Join text
    set parts [list \
		   color size font width border rotation justification hwflag];
    ::gempak::_join_subparam "stnplt" "text" "/" $parts;

    # Join marker
    set parts [list color type size width hwflag];
    ::gempak::_join_subparam "stnplt" "marker" "/" $parts;

    # Join stnfile
    set parts [list name column];
    ::gempak::_join_subparam "stnplt" "stnfile" "#" $parts;

    # All
    set parts [list text marker stnfile];
    ::gempak::_set_param "stnplt" "|" $parts;
}

proc ::gempak::get_stnplt {} {

    return [::gempak::get "stnplt"];
}

proc ::gempak::get_stnplt_list {} {

    return [split [::gempak::get_stnplt] "|"];
}
