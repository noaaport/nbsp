#
# $Id$
#
# colors utility functions (used, for example, with sfparm)
#
# colors = color;...
#
proc gempak::colors {args} {

    variable gempak;

    set gempak(param,colors) [join $args ";"];
}

proc gempak::get_colors {} {

    return [::gempak::get "colors"];
}

proc gempak::get_colors_list {} {

    return [split [::gempak::get_colors] ";"];
}

proc gempak::append_colors {color} {

    set current [::gempak::get_colors];
    if {$current eq ""} {
	set current $color;
    } else {
	append current ";" $color;
    }
    ::gempak::define "colors" $current;    
}
