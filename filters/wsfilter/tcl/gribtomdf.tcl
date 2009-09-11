#!%TCLSH%
#
# $Id$
#
# Usage:  gribtomdf [-v] [-d <outputdir>] [-f] [-i <skipx>] [-j <skipy>]
#                   [-m <model>] [-g <grid>] [-h <fh>] [-n <n>]
#                   [-o <outputfile>] [-u <undef>] [-y] [-Y <Y>] [<name>]
#
# If <name> is given and [-f] is given, then the <name> is taken to be
# an input file name it is taken as is. If [-f] is not given, then
# <name> is assumed to be either <ymd>|<ymdh>, and a file with the name
#
#                   <model>_<grid>_<ymdh>_<fh>.<fext>
#
# is looked for in the control directory and used. If only <ymd> is given,
# then <ymd>* is used as a glob pattern and the last (most recent) one
# found is used, or the one given by "-n <n>" (<n> = end, end-1, ...).
# If <name> is not given, then the default <ymd> is the current day or,
# if "-y" is given, the previous day or <Y> days before if "-Y" is also given.
# The defaults for "-m", "-g" and "-h" are gfs, 211, 0, respectively.
#

## The common defaults
set defaults_file "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaults_file] == 0} {
    puts "wsfilter disabled: $defaults_file not found.";
    return 1;
}
source $defaults_file;
unset defaults_file;

## The common tools initialization
set wsfilter_initfile [file join $common(libdir) "wsfilter.init"];
if {[file exists $wsfilter_initfile] == 0} {
    puts "$wsfilter_initfile not found.";
    return 1;
}
source $wsfilter_initfile;
unset wsfilter_initfile;

package require cmdline;
package require fileutil;
package require grads;

set usage {gribtomdf [-v] [-d <outputdir>] [-f] [-i <skipx>] [-j <skipy>]
    [-m <model>] [-g <grid>] [-h <fh>] [-n <n>] [-o <outputfile>] [-u <undef>]
    [-y] [-Y <Y> [<ymd|ymdh>]};

set optlist {{d.arg ""} {f} {g.arg "211"} {h.arg "0"} {m.arg "gfs"}
    {i.arg "1"} {j.arg "1"} {n.arg "end"} {o.arg ""} {u.arg ""} {v}
    {y} {Y.arg "1"}};

array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

# Defaults unless given in cmd line
set g(undef) $wsfilter(grib_undef);

proc gribtomdf_isvalid {val} {

    global g;

    if {[regexp $g(grads_undef_regex) $val]} {
	return 0;
    }

    return 1;
}

proc _uv_to_wdir {u v} {

    global g;

    if {![gribtomdf_isvalid $u] || ![gribtomdf_isvalid $v]} {
	return $g(undef);
    }

    set theta [expr 57.29 * atan2($u,$v)];

    if {$v >= 0.0} {
	set theta [expr $theta + 180.0];
    } else {
	set theta [expr 360.0 - $theta];
    }

    return [format "%.2f" $theta];
}

proc gribtomdf_sfc {pres_Pa} {
#
# The presure is output in Pa, WS accepts kPa or mb. We will use mb.
#
# Get
#
#        pressfc_Pa
#
# and return
#
#        pres_mb

    global g;

    if {![gribtomdf_isvalid $pres_Pa]} {
	return $g(undef);
    }

    set pres_mb [expr $pres_Pa * 0.01];
    
    return $pres_mb;
}

proc gribtomdf_trop {val_str} {
#
# Get
#
# prestrp_Pa tmptrp ugrdtrp vgrdtrp
#
# and return
#
# prestrp_mb tmptrp wspeedprs wdirprs
#
    global g;

    set val_list [split $val_str];
    set p_Pa [lindex $val_list 0];
    set t [lindex $val_list 1];
    set u [lindex $val_list 2];
    set v [lindex $val_list 3];

    if {![gribtomdf_isvalid $u] || ![gribtomdf_isvalid $v]} {
	set ws $g(undef);
	set wd $g(undef);
    } else {
	set ws [format "%.2f" [expr hypot($u,$v)]];
	set wd [_uv_to_wdir $u $v];
    }

    if {![gribtomdf_isvalid $p_Pa]} {
	set p_mb $g(undef);
    } else {
	set p_mb [expr $p_Pa * 0.01];
    }

    return [join [list $p_mb $t $ws $wd] " "];
}

proc gribtomdf_prs {val_str} {
#
# Get
#
#        "hgtprs tmpprs rhprs ugrdprs vgrdprs"
#
# and return
#
#         "hgtprs tmpprs rhprs wspeedprs wdirprs"

    global g;

    set val_list [split $val_str];

    set h [lindex $val_list 0];
    set t [lindex $val_list 1];
    set rh [lindex $val_list 2];
    set u [lindex $val_list 3];
    set v [lindex $val_list 4];

    if {![gribtomdf_isvalid $u] || ![gribtomdf_isvalid $v]} {
	set ws $g(undef);
	set wd $g(undef);
    } else {
	set ws [format "%.2f" [expr hypot($u,$v)]];
	set wd [_uv_to_wdir $u $v];
    }

    return [join [list $h $t $rh $ws $wd] " "];
}

proc gribtomdf_cvt {ctlfpath} {

    global wsfilter;
    global g;
    global option;

    set fname [file rootname [file tail $ctlfpath]];
    if {[regexp {([^_]+)_(\d+)_((\d{4})(\d{2})(\d{2})(\d{2}))_(\d+)h} \
	     $fname match model grid ymdh yyyy mm dd hh fh] == 0} {
	return -code error "Invalid file name format: $fname.";
    }

    set params(sfc) $wsfilter(grib_params,sfc,$g(model),$g(grid));
    set params(trop) [join $wsfilter(grib_params,trop,$g(model),$g(grid)) "|"];
    set prslevels $wsfilter(grib_prslevels,$g(model),$g(grid));
    set params(prs) [join $wsfilter(grib_params,prs,$g(model),$g(grid)) "|"];

    # Count the parameters
    #
    # lat, lon => 2
    # sfc      => 1
    set num_latlonsfc 3;
    set num_trop \
	[llength $wsfilter(grib_param_titles,trop,$g(model),$g(grid))];
    set num_prslevels [llength $wsfilter(grib_prslevels,$g(model),$g(grid))];
    set num_prsparams \
	[llength $wsfilter(grib_param_titles,prs,$g(model),$g(grid))];
    set numparams [expr $num_latlonsfc + $num_trop + \
		       $num_prslevels * $num_prsparams];

    set header "  101\n  $numparams $yyyy $mm $dd $hh 00 00";

    set titles " stid stnm time lat lon";
    append titles " " $wsfilter(grib_param_titles,sfc,$g(model),$g(grid));
    if {$num_trop != 0} {
	append titles " " \
	    [join $wsfilter(grib_param_titles,trop,$g(model),$g(grid)) " "];
    }
    foreach level $wsfilter(grib_prslevels,$g(model),$g(grid)) {
	foreach name $wsfilter(grib_param_titles,prs,$g(model),$g(grid)) {
	    append titles " " "${name}_${level}mb";
	}
    }

    # <stid> <stnm> <hour_run>
    set mdfparams "%06d %d 0";

    ::grads::init;
    ::grads::open $ctlfpath;

    set output_data_list [list];

    lappend output_data_list $header;
    lappend output_data_list $titles;

    ::grads::get_dimensions a;

    set count 1;	# to generate stid and stnm
    set x $a(x1);
    while {$x <= $a(x2)} {
	::grads::exec set x $x;
	set y $a(y1);
	while {$y <= $a(y2)} {
	    ::grads::exec set y $y;
	    
	    if {$option(v) == 1} {
		puts stderr "x=$x y=$y";
	    }

	    set r [format $mdfparams $count $count];

	    append r " " [::grads::eval_expr "lat|lon" -S " "];
	    append r " " [gribtomdf_sfc [::grads::eval_expr $params(sfc)]];
	    if {$num_trop != 0} {
		append r " " \
		    [gribtomdf_trop [::grads::eval_expr $params(trop) -S " "]];
	    }
	    foreach l $prslevels {
		::grads::exec set lev $l;
		append r " " \
		    [gribtomdf_prs [::grads::eval_expr $params(prs) -S " "]];
	    }

	    # Change 9.999e+20 or ? to -99x
	    regsub -all $wsfilter(grads_undef_regex) \
		$r $wsfilter(grib_undef) r;
	    lappend output_data_list $r;
	    incr count;
	    incr y $option(j);
	}
	incr x $option(i);
    }

    ::grads::end;

    return [join $output_data_list "\n"];
}

#
# main
#
set g(ctldir) $wsfilter(grib_ctl_basedir);
set g(grib_mdf_fext) $wsfilter(grib_mdf_fext);
set g(grads_undef_regex) $wsfilter(grads_undef_regex);

if {$option(u) ne ""} {
    set g(undef) $option(u);
}

set g(model) $option(m);
set g(grid) $option(g);
set fh $option(h);
set option(ymdh_glob) 0;
set ctlfpath "";

if {$argc != 0} {
    if {$option(f) == 1} {
	set ctlfpath [lindex $argv 0];
    } else {
	set name [lindex $argv 0];
	if {[regexp {(\d{4})(\d{2})(\d{2})(\d{2})} \
	     $name match yyyy mm dd hh]} {
	    set ymdh $name;
	} elseif {[regexp {(\d{4})(\d{2})(\d{2})} \
		$name match yyyy mm dd]} {
	    set ymdh "${name}*";
	    set option(ymdh_glob) 1;
	} else {
	    puts "Invalid argument $name.";
	    exit 1;
	}
    }
} else {
    set seconds [clock seconds];
    if {$option(y) == 1} {
	set seconds [expr $seconds - $option(Y)*24*3600];
    }
    set ymd [clock format $seconds -format "%Y%m%d" -gmt true];
    set ymdh "${ymd}*";
    set option(ymdh_glob) 1;
}

if {$option(ymdh_glob) == 1} {
    # Look for the ymdh
    set dirlist [lsort -dictionary [glob -nocomplain \
        -dir [file join $g(ctldir) $option(m)] -tails $ymdh]];
    if {[llength $dirlist] == 0} {
	puts "$ymdh not found.";
	exit 1;
    } else {
	set ymdh [lindex $dirlist $option(n)];
	if {$ymdh eq ""} {
	    puts "Invalid value of option -n.";
	    exit 1;
	}
    }
}

if {$ctlfpath eq ""} {
    set fname [join [list $g(model) $g(grid) $ymdh ${fh}h] "_"];
    set ctlfpath [file join $g(ctldir) $g(model) $ymdh $fname];
} else {
    set fname [file rootname [file tail $ctlfpath]];
}

# Here ctlfpath does not include the extension and therefore we cannot
# try to see if the file exists using literally the ctlfpath as the full
# path name. We can leave up to grads to return an error but we can try
# to catch it here also.
set status [catch {
    glob -directory [file dirname $ctlfpath] "$fname*";
} errmsg];
if {$status != 0} {
    puts $errmsg;
    exit 1;
}

if {$option(o) eq ""} {
    set fbasename [file rootname [file tail $ctlfpath]];
    append fbasename $g(grib_mdf_fext);
    if {$option(d) ne ""} {
	set outputfile [file join $option(d) $fbasename];
    } else {
	set outputfile [file join $wsfilter(ws_datadir) $ymd \
			       $wsfilter(ws_gribdir) $fbasename];
    }
} else {
    set outputfile $option(o);
    if {($outputfile ne "-") && ($option(d) ne "")} {
	set outputfile [file join $option(d) $option(o)];
    }
}

if {$outputfile ne "-"} {
    # This will create the subdirectories
    ::fileutil::writeFile $outputfile [gribtomdf_cvt $ctlfpath];
} else {
    puts [gribtomdf_cvt $ctlfpath];
}
