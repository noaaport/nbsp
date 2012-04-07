#
# $Id$
#
# Contours may be displayed as lines or as a color fill.
# If CTYPE is C, contour lines are drawn using input from CINT
# and LINE.  If CTYPE is F, filled contours are drawn using
# specifications from FINT and FLINE. Both contour lines and
# filled contours are drawn if CTYPE is F/C.
#
# For gdplo2
#    
# TYPE specifies the processing type for the GDPLOT2 GDPFUN parameter.
# The TYPE list does not need separators, however slashes could be
# used for clarity:
#
#                type 1 / type 2 / ... / type n
#

#
# ctype
#
proc ::gempak::ctype_fill {} {

    variable gempak;

    set gempak(param,ctype) "f";
}

proc ::gempak::ctype_contour {} {

    variable gempak;

    set gempak(param,ctype) "c";
}

proc ::gempak::ctype_fc {} {

    variable gempak;

    set gempak(param,ctype) "f/c";
}

#
# line
#
proc ::gempak::line_color {color} {

    variable gempak;

    set gempak(param,line.color) $color;
}

proc ::gempak::line_type {type} {

    variable gempak;

    set gempak(param,line.type) $type;
}

proc ::gempak::line_width {width} {

    variable gempak;

    set gempak(param,line.witdh) $width;
}

proc ::gempak::line_label {label} {

    variable gempak;

    set gempak(param,line.label) $label;
}

proc ::gempak::line_smth {smth} {

    variable gempak;

    set gempak(param,line.smth) $smth;
}

proc ::gempak::line_fltr {fltr} {

    variable gempak;

    set gempak(param,line.fltr) $fltr;
}

proc ::gempak::line_scfflg {scflg} {

    variable gempak;

    set gempak(param,line.scflg) $scflg;
}

proc ::gempak::set_line {} {

    variable gempak;

    # All
    set parts [list color type width label smth fltr scflg];
    ::gempak::_set_param "line" "/" $parts;
}

proc ::gempak::get_line {} {

    return [::gempak::get "line"];
}

proc ::gempak::get_line_list {} {

    return [split [::gempak::get_line] "/"];
}

proc ::gempak::append_cint {} {
#
# gdplot2 can have several of these
#
    set current [::gempak::get_line];
    ::gempak::set_line;
    if {$current ne ""} {
	append current "!" [::gempak::get_line];
	::gempak::define "line" $current;
    }
}

#
# contour interval
#
proc ::gempak::cint_interval {interval} {

    variable gempak;

    set gempak(param,cint.interval) $interval;
}

proc ::gempak::cint_min {min} {

    variable gempak;

    set gempak(param,cint.min) $min;
}

proc ::gempak::cint_max {max} {

    variable gempak;

    set gempak(param,cint.max) $max;
}

proc ::gempak::set_cint {} {

    variable gempak;

    # All
    set parts [list interval min max];
    ::gempak::_set_param "cint" "/" $parts;
}

proc ::gempak::get_cint {} {

    return [::gempak::get "cint"];
}

proc ::gempak::get_cint_list {} {

    return [split [::gempak::get_cint] "/"];
}

proc ::gempak::append_cint {} {
#
# gdplot2 can have several of these
#
    set current [::gempak::get_cint];
    ::gempak::set_cint;
    if {$current ne ""} {
	append current "!" [::gempak::get_cint];
	::gempak::define "cint" $current;
    }
}

#
# fint
#
proc ::gempak::fint_interval {interval} {

    variable gempak;

    set gempak(param,fint.interval) $interval;
}

proc ::gempak::fint_min {min} {

    variable gempak;

    set gempak(param,fint.min) $min;
}

proc ::gempak::fint_max {max} {

    variable gempak;

    set gempak(param,fint.max) $max;
}

proc ::gempak::set_fint {} {

    variable gempak;

    # All
    set parts [list interval min max];
    ::gempak::_set_param "fint" "/" $parts;
}

proc ::gempak::get_fint {} {

    return [::gempak::get "fint"];
}

proc ::gempak::get_fint_list {} {

    return [split [::gempak::get_fint] "/"];
}

proc ::gempak::append_fint {} {
#
# gdplot2 can have several of these
#
    set current [::gempak::get_fint];
    ::gempak::set_fint;
    if {$current ne ""} {
	append current "!" [::gempak::get_fint];
	::gempak::define "fint" $current;
    }
}

#
# fline
#
proc ::gempak::fline_color {color} {

    variable gempak;

    set gempak(param,fline.color) $color;
}

proc ::gempak::fline_type {type} {

    variable gempak;

    set gempak(param,fline.type) $type;
}

proc ::gempak::set_fline {} {

    variable gempak;

    # All
    set parts [list color type];
    ::gempak::_set_param "fline" "/" $parts;
}

proc ::gempak::get_fline {} {

    return [::gempak::get "fline"];
}

proc ::gempak::get_fline_list {} {

    return [split [::gempak::get_fline] "/"];
}

proc ::gempak::append_fline {} {
#
# gdplot2 can have several of these
#
    set current [::gempak::get_fline];
    ::gempak::set_fline;
    if {$current ne ""} {
	append current "!" [::gempak::get_fline];
	::gempak::define "fline" $current;
    }
}

#
# for gdplot2
#
proc ::gempak::gdpfun {name} {

    variable gempak;

    set gempak(param,gdpfun) $name;
}

proc ::gempak::append_gdpfun {name} {

    variable gempak;

    set current [::gempak::get "gdpfun"];
    ::gempak::gdpfun $name;
    if {$current ne ""} {
	append current "!" [::gempak::get "gdpfun"];
	::gempak::gdpfun $current;
    }
}

proc ::gempak::get_gdpfun {} {

    variable gempak;

    return $gempak(param,gdpfun);
}

proc ::gempak::get_gdpfun_list {} {

    return [split [::gempak::get_gdpfun] "!"];
}

#
# type
#
proc ::gempak::type {args} {
#
# For gdplot2, the type can have more flags than just f/c, and also there
# is one type parameter for each gdpfun variable, which requires introducing
# the "append" function below.
#
    variable gempak;

    set gempak(param,type) [join $args "/"];
}

proc ::gempak::append_type {args} {

    variable gempak;

    set current [::gempak::get "type"];
    ::gempak::type $args;
    if {$current ne ""} {
	append current "!" [::gempak::get "type"];
	::gempak::define "type" $current;
    }
}
