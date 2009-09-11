#!%TCLSH%
#
# $Id$
#
# Usage: nbspgribplotc [-m <model>] [-g <grid>] [-h <fh>]
#                      [-f <ctlfile>]
#                      [-y] [-Y <Y>] [-I <I>]
#                      [-d outputdir] [-o imgfile]
#                      [-D var1=val1 -D var2=val2 ...]
#                      [-M var1=val1 -M var2=val2 ...]
#                      [-n] [-r] [-t] <rcfile>
#
# (The "c" in the name is a reminder that the data file is a grads ctl file).
#
# -  Options -m, -g, -h are mandatory to specify the ctl file, or -f.
#    The ctlfile is assumed to be relative to the $gribfilter(ctldatadir).
# -  By default the program looks in the current <ymd> directory and
#    searches for the last ctl file, except as modified by the -y, -Y, -I
#    options.
# -f => Use the <ctlfile> as is, instead of giving -m, -g, -h.
# -y => Use the <ymd> for the previous day
# -Y => If -y is given, use the <ymd> for <Y> days before (default is -Y 1).
# -I => Use the ymdh for the model run with index <I> instead of the last
#       (default is "end").
#       (the oldest is 0 and the most recent one is "end")
# -r => The rcfile is searched for in the various script directories:
#       defaults, site, and the last one is used;
#       otherwise it is searched for first in the current directory,
#       and only if it is not found, then the standard directories are used.
# -t => The script is a tclgrads script. A script is also assumed to be a
#       tclgrads script also if it has the extension ".tgs".
# -n => Do not use the extension ".tgs" as indication that the script is
#       a tclgrads script.
# -o => The value of gsplot(imgfile) that is passed to the script. Otherwise
#       it is left empty and the script must construct an appropriate name.
# -d => The value of gsplot(outputdir) that is passed to the script. 
#       If it is given, the directory is taken to be relative to
#       gribfilter(imgdatadir) and the full path to the directory is set
#       in gsplot(outputdir). Otherwise it is left empy.

set usage {Usage: nbspgribplotc [-m model] [-g grid] [-h fh] [-I index]
    [-f ctlfile] [-y] [-Y <Y>]
    [-d outputdir] [-o imgfile] [-D var=val ...]
    [-M var=val ...] [-n] [-r] [-t] <rcfile>};
set optlist {m.arg g.arg h.arg I.arg f.arg d.arg o.arg D.marg M.marg n r t
    y Y.arg};

set option(default_Y) 1;      # previous day if -y is given
set option(default_I) "end";  # the last model run for the given ymd

proc run_tclgrads_script {gsfile} {
#
# In this version of the function the script is should be a tclgrads
# script.
#
    global gsplot;

    source $gsfile;
}

proc run_grads_script {gsfile} {
#
# In this version the script contains a grads script in the tcl variable
# gsplot(script).
#
    global gsplot;

    source $gsfile;

    if {[info exists gsplot(script)] == 0} {
	return;
    }

    ::nbsputil::pwrite_block [subst $gsplot(script)] grads -bl > /dev/null;

    if {[info exists gsplot(post)]} {
	eval $gsplot(post);
    }
}

## The common grib tools initialization
set initfile "/usr/local/libexec/nbsp/nbspgribtools.init";
if {[file exists $initfile] == 0} {
    puts "$initfile not found.";
    return 1;
}
source $initfile;
unset initfile;

package require grads;
package require nbsputil;
package require errx;

array set option [::nbsputil::cmdline_getoptions argv $optlist $usage];
set argc [llength $argv];

foreach k [list I Y] {
    if {$option($k) eq ""} {
	set option($k) $option(default_$k);
    }
}

if {$argc != 1} {
    ::errx::err $usage;
}
set _gsfile [lindex $argv 0];

if {($option(f) eq "") && \
	(($option(m) eq "") || ($option(g) eq "") || ($option(h) eq ""))} {
    ::errx::err "Either -f or, -m and -g and -h are mandatory.";
}

# Locate the script
if {$option(r) == 0} {
    set grads(gsfile) ${_gsfile};
    if {[file exists $grads(gsfile)] == 0} {
	# Retry (below) in the standard directories
	set option(r) 1;
    }
}

if {$option(r) == 1} {
    set grads(gsfile) "";
    foreach d $gribfilter(gsdirlist) {
	if {[file exists [file join $d ${_gsfile}]]} {
	    set grads(gsfile) [file join $d ${_gsfile}];
	}
    }
}
if {$grads(gsfile) eq ""} {
    ::errx::err "${_gsfile} not found.";
}

set grads(ctlfile) "";
if {$option(f) ne ""} {
    set grads(ctlfile) $option(f);
    if {[file extension $grads(ctlfile)] eq ""} {
	append grads(ctlfile) $gribfilter(ctlfext);
    }
    if {[file exists $grads(ctlfile)] == 0} {
	::errx::err "$grads(ctlfile) not found.";
    }
}

if {$grads(ctlfile) eq ""} {
    # Locate the ctlfile
    set seconds [clock seconds];
    if {$option(y) == 1} {
	set seconds [expr $seconds - $option(Y)*24*3600];
    }
    set ymd [clock format $seconds -format "%Y%m%d" -gmt true];
    set ymdh_glob "${ymd}*";

    # Now look for the last subdirectory <model>/ymd*
    set _d [file join $gribfilter(datadir) $gribfilter(ctldatadir) $option(m)];
    set dirlist [lsort -dictionary [glob -dir ${_d} -nocomplain $ymdh_glob]];
    if {[llength $dirlist] == 0} {
	::errx::err "$ymdh_glob not found.";
    }
    set ymdh_fpath [lindex $dirlist $option(I)];
    if {$ymdh_fpath eq ""} {
	::errx::err "Invalid value of option -I.";
    }
    set ymdh [file tail $ymdh_fpath];

    # Now look inside the ymdh directory for the ctl file. If option(h) is a
    # number (without the unit specification h,d) assume h.
    if {[regexp {^[0-9]+$} $option(h)]} {
	append option(h) "h";
    }
    set _ctlname [gribfilter_join_default_name \
		      $option(m) $option(g) $ymdh $option(h)];

    set grads(ctlfile) [file join $ymdh_fpath ${_ctlname}];
    append grads(ctlfile) $gribfilter(ctlfext);

    if {[file exists $grads(ctlfile)] == 0} {
	::errx::err "$grads(ctlfile) not found.";
    }
}

# Define the variables for the script
set gsplot(ctlfile) $grads(ctlfile);
set gsplot(ctlfname) [file rootname [file tail $gsplot(ctlfile)]];
set gsplot(imgfile) $option(o);
if {$option(d) ne ""} {
    set _imgbasedir [file join $gribfilter(datadir) $gribfilter(imgdatadir)];
    if {[file isdirectory ${_imgbasedir}]} {	 
	set gsplot(outputdir) [file join ${_imgbasedir} $option(d)];
	file mkdir $gsplot(outputdir);
    } else {
	::errx::err "${_imgbasedir} not found.";
    }
} else {
    set gsplot(outputdir) "";
}

# Break the name using the function provided by the gribfilter
# (in gribfilter.init)
set ctlname_parts [gribfilter_break_default_name $gsplot(ctlfname)];
set gsplot(model) [lindex $ctlname_parts 0];
set gsplot(grid) [lindex $ctlname_parts 1];
set gsplot(reftime) [lindex $ctlname_parts 2];
set gsplot(forecasttime) [lindex $ctlname_parts 3];

# The user-defined variables
foreach d $option(D) {
    set keyval [::nbsputil::split_first $d "="];
    set key [lindex $keyval 0];
    set val [lindex $keyval 1];
    set gsplot(D,$key) $val;
}

foreach d $option(M) {
    set keyval [::nbsputil::split_first $d "="];
    set key [lindex $keyval 0];
    set val [lindex $keyval 1];
    set gsplot(M,$key) $val;
}

if {([file extension $grads(gsfile)] eq ".tgs") && ($option(n) == 0)} {
    set option(t) 1;
}

if {$option(t) == 0} {
    run_grads_script $grads(gsfile);
} else {
    run_tclgrads_script $grads(gsfile);
}
