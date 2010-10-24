#
# $Id$
#
# nbsp::gis::init {color_def_file}
# nbsp::gis::x11color {color_name}
# nbsp::gis::x11color_set {name rgb}
# nbsp::gis::rgbcolor {family number}
# nbsp::gis::radcolor {awips1 level}
# nbsp::gis::radcolor_set {awips1 level rgb}
#
# The gisfilter.init1 file must have been sourced before requiring this
# package, so that it can find the gisfilter template directories.
#
package provide nbsp::gis 1.0;

namespace eval nbsp::gis {} {

    variable gis;
    variable x11color;
    variable rgbcolor;
    variable radcolor;

    array set x11color {};
    array set rgbcolor {};
    array set radcolor {};
}

proc nbsp::gis::init {color_def_file} {
    
    #
    # Load all the color definitions
    #
    source $color_def_file;
}

proc nbsp::gis::x11color {name} {

    variable x11color;

    return $x11color($name);
}

proc nbsp::gis::x11color_set {name rgb} {

    variable x11color;

    set x11color($name) $rgb;
}

proc nbsp::gis::rgbcolor {family number} {

    variable rgbcolor;

    return $rgbcolor($family,$number);
}

proc nbsp::gis::radcolor {awips1 level} {

    variable radcolor;

    if {[regexp {^n?(r|q|z)$} $awips1]} {
	set k "bref";
    } elseif {[regexp {^n?(v|u)$} $awips1]} {
	set k "rvel";
    } else {
	return -code error "No color table for $awips1.";
    }

    return $radcolor($k,$level);
}

proc nbsp::gis::radcolor_set {awips1 level rgb} {

    variable radcolor;

    if {[regexp {^n?(r|q|z)$} $awips1]} {
	set k "bref";
    } elseif {[regexp {^n?(v|u)$} $awips1]} {
	set k "rvel"; 
    } else {
	return -code error "No color table for $awips1.";
    }

    set radcolor($k,$level) $rgb;
}
