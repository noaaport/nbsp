#!%TCLSH%
#
# $Id$
#
# Usage: nbspgribplot [-c ctlfile] [-d outputdir] [-g grbfile] [-i idxfile]
#		[-o imgfile] [-D <definitions>] [-M <definitions>]
#               [-n] [-t] <gsfile>
#
# Either (or both) of [-g] of [-c] must be given. If [-g] is not given
# then the ctlfile must exist. If either the ctl or idx file does not
# exist, it is created.
#
# The [-t] option indicates that the script is a tclgrads script. A script
# is assumed to be a tclgrads script also if it has the extension ".tgs",
# unless the [-n] option is given in which case this identification is not
# made.
#
# The only effect of [-d] and [-o] is to give optional values to the variables
# gsplot(outputdir) and gsplot(imgfile), to pass them to the gsfile. Otherwise
# gsplot(outoutdir) is "" and gsplot(imgfile) has the default value (derived
# from the gsfile), unless it is "-" in which case it also left empty.

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

set usage {Usage: nbspgribplot [-c ctlfile] [-g grbfile] [-i idxfile]
    [-d outputdir] [-o imgfile] [-D var=val ...] [-M var=val ...]
    [-n] [-t] <gsfile>};
set optlist {c.arg d.arg g.arg i.arg o.arg D.marg M.marg n t};

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

#
# main
#
array set option [::nbsputil::cmdline_getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc != 1} {
    ::errx::err $usage;
}
set grads(gsfile) [lindex $argv 0];

# At least one of [-c] or [-g] must be given
if {($option(g) eq "") && ($option(c) eq "")} {
    ::errx::err $usage;
}

if {$option(g) ne ""} {
    set grads(grbfile) $option(g);
}

if {$option(c) ne ""} {
    set grads(ctlfile) $option(c);
} else {
    set grads(ctlfile) [file rootname [file tail $grads(grbfile)]];
    append grads(ctlfile) $gribfilter(ctlfext);
}

# If only the ctlfile is given, it must exist.
if {$option(g) eq ""} {
    if {[file exists $grads(ctlfile)] == 0} {
	::errx::err "$grads(ctlfile) not found.";
    }
}

set grads(idxfile) [file rootname [file tail $grads(ctlfile)]];
append grads(idxfile) $gribfilter(idxfext);
set grads(imgfile) [file rootname [file tail $grads(gsfile)]];
append grads(imgfile) $gribfilter(imgfext);

if {$option(i) ne ""} {
    set grads(idxfile) $option(i);
}

if {[file exists $grads(ctlfile)] == 0} {
    set status [catch {
	exec grib2ctl -verf $grads(grbfile) $grads(idxfile) > $grads(ctlfile);
    } errmsg];

    if {$status != 0} {
	::errx::err $errmsg;
    }
}

if {[file exists $grads(idxfile)] == 0} {
    # Work around bug in gribmap (does not set exit status correctly)
    set msg "";
    set status [catch {		 
	set msg [exec gribmap -q -i $grads(ctlfile)];
    } errmsg];
    if {$msg != ""} {
	::errx::err $msg;
    }
}

set gsplot(ctlfile) $grads(ctlfile);
set gsplot(ctlfname) [file rootname [file tail $gsplot(ctlfile)]];
set gsplot(outputdir) $option(d);
if {$option(o) eq ""} {
    set gsplot(imgfile) $grads(imgfile);
} elseif {$option(o) eq "-"} {
    set gsplot(imgfile) "";
} else {
    set gsplot(imgfile) $option(o);
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
