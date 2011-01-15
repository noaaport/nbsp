#!/usr/local/bin/tclsh8.5
#
# $Id$
#
source "ssh.tcl";

# exec nbspgismap -d . rad_n0v_vwx;
# exec scp img/rad/n0v_vwx.png tp:n0v_vwx.png

set script {
    set geoclist [exec nbspgismap -L];

    foreach c $geoclist {
    #
    # Include only bundles, exclude the individual sites, and only
    # the lowest level of anything. The "-t" option creates time-stamped
    # image file names.
    #
	if {[regexp {^rad_.+_(ak|hi|pr|conus|central|east|south|west)$} $c]} {
	    exec nbspgismap -t -w $c;
	}
    }
}

set slave "diablo";
::ssh::connect -t tclsh8.6 -- $slave
::ssh::push $slave $script
::ssh::send $slave;
::ssh::disconnect $slave;
