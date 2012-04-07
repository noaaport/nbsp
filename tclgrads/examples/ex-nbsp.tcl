#!/usr/local/bin/tclsh8.4
#
# $Id$
#
package require grads;

# The date (and model) has to be adjusted to whatever is available in the Nbsp
# directory ``"/var/noaaport/data/grib/ctl''

set dir "/var/noaaport/data/grib/ctl/gfs/2008092006";
set flist [glob -directory $dir *yq*.ctl];

::grads::init;

foreach f $flist {
    ::grads::open $f;

    ::gradsu::times times;
    set date [lindex $times 1];

    ::gradsu::display "skip(ugrdprs,3);skip(vgrdprs,3);tmpprs";
    ::gradsu::draw "title Wind/Temp $date";
    ::gradsu::printim uvt-$date.png;

    ::gradsu::reinit;
}

::grads::end;
