#
# $Id$
#
proc gempak::skip_contour {skip_contour} {

    variable gempak;

    set gempak(param,skip.contour) $skip_contour
}

proc gempak::skip_plot_xy {skip_x skip_y} {

    variable gempak;

    set gempak(param,skip.plot_x) $skip_x;
    set gempak(param,skip.plot_y) $skip_y
}

proc gempak::set_skip {} {

    variable gempak;

    # The skip_plot parts
    set gempak(param,skip.plot) [::gempak::_join_subparam \
        "skip" "plot" ";" [list x y]];

    # All
    set parts [list contour plot];
    ::gempak::_set_param "skip" "/" $parts;
}

proc gempak::get_skip {} {

    return [::gempak::get "skip"];
}

proc gempak::get_skip_list {} {

    return [split [::gempak::get_skip] "/"];
}
