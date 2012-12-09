#
# $Id$
#
# WIND specifies the wind symbol, size, width, type, and head size 
# separated by slashes: wind symbol / size / width / type / arrow head size
# The wind symbol contains a letter for symbol type, a letter for symbol 
# units and a color number with no separators.  The character meanings are:
#        TYPE:   B = BARB        A = ARROW               D = DIRECTIONAL ARROW
#       UNITS:   K = KNOTS       M = m/s
#       COLOR:   Color number    0 = no wind plotted
#
proc gempak::wind_symbol_type {type} {

    variable gempak;

    set gempak(param,wind.symbol_type) $type;
}

proc gempak::wind_symbol_units {units} {

    variable gempak;

    set gempak(param,wind.symbol_units) $units;
}

proc gempak::wind_symbol_color {color} {

    variable gempak;

    set gempak(param,wind.symbol_color) $color;
}

proc gempak::wind_size {size} {

    variable gempak;

    set gempak(param,wind.size) $size
}

proc gempak::wind_width {width} {

    variable gempak;

    set gempak(param,wind.width) $width;
}

proc gempak::wind_type {type} {

    variable gempak;

    set gempak(param,wind.type) $type;
}

proc gempak::wind_arrow_head_size {size} {

    variable gempak;

    set gempak(param,wind.arrow_head_size) $size;
}

proc gempak::set_wind {} {

    variable gempak;

    # The wind symbol
    set gempak(param,wind.symbol) [::gempak::_join_subparam \
        "wind" "symbol" "" [list type units color]];

    # All
    set parts [list symbol size width type arrow_head_size];
    ::gempak::_set_param "wind" "/" $parts;
}

proc gempak::get_wind {} {

    return [::gempak::get "wind"];
}

proc gempak::get_wind_list {} {

    return [split [::gempak::get_wind] "/"];
}
