#
# $Id$
#
# nbsp::gis::init {color_def_file}
# nbsp::gis::x11color {color_name}
# nbsp::gis::radcolor {}
#
# The gisfilter.init1 file must have been sourced before requiring this
# package, so that it can find the gisfilter template directories.
#
package provide nbsp::gis 1.0;

namespace eval nbsp::gis {} {

    variable gis;
    variable x11color;
    variable radcolor;

    array set x11color {};
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

proc nbsp::gis::radcolor {awips1} {

    variable radcolor;

    return $radcolor($awips1);
}
