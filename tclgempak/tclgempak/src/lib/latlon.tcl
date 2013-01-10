#
# $Id$
#
# latlon utility functions
#

proc gempak::latlon_color {color} {

    variable gempak;

    set gempak(param,latlon.color) $color;
}

proc gempak::latlon_linetype {linetype} {

    variable gempak;

    set gempak(param,latlon.linetype) $linetype;
}

proc gempak::latlon_linewidth {linewidth} {

    variable gempak;

    set gempak(param,latlon.linewidth) $linewidth;
}

proc gempak::latlon_freq {xfreq yfreq} {

    ::gempak::_join_param_parts "latlon.freq" ";" $xfreq $yfreq;
}

proc gempak::latlon_incr {xincr yincr} {

    ::gempak::_join_param_parts "latlon.incr" ";" $xincr $yincr;
}

proc gempak::latlon_label {latlabel lonlabel} {

    ::gempak::_join_param_parts "latlon.label" ";" $latlabel $lonlabel;
}

proc gempak::latlon_format {format} {

    variable gempak;

    set gempak(param,latlon.format) $format;
}

proc gempak::set_latlon {} {

    set parts [list color linetype linewidth freq incr label format];

    ::gempak::_set_param "latlon" "/" $parts;
}

proc gempak::get_latlon {} {

    return [::gempak::get "latlon"];
}

proc gempak::get_latlon_list {} {

    return [split [::gempak::get_latlon] "/"];
}
