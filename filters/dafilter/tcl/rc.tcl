#
# $Id$
#

#
# These functions are used to construct the default file name. They here
# and not in a library because thet are meant to be user-modifiable.
# (They are copied from the gribfilter.)
#
proc dafilter_make_default_grbname {rc_array_name f_flag {modelname ""}} {
    #
    # "modelname" is put as an argument because some rules use a special
    # value. If f_flag is 1, the forecast time is included in the name;
    # otherwise it is not.
    #
    upvar 1 $rc_array_name rc;

    if {$modelname eq ""} {
	set modelname $rc(gribmodel);
    }

    set fname "";
    append fname $modelname "_" $rc(gribgrid) "_" $rc(gribymdh);

    if {$f_flag == 1} {
	append fname "_" $rc(gribforecasttime);
    }
    append fname [dafilter_make_default_grbext $rc(gribedition)];

    return $fname;
}

proc dafilter_make_default_grbext {gribedition} {

    global dafilter;

    set fext $dafilter(gribfext);
    if {$gribedition != 1} {
	append fext $gribedition;
    }

    return $fext;
}
