#!%TCLSH%
#
# $Id$
# 
# Usage: nbspgdradr [-b] [-c] [-C] [-d <outputdir>] [-D <defs>]
#        [-f <outputnamefmt> | -o <outputname>] [-k] [-l <logfile>]
#        [-t <tmpdir>] [-r <rcfile> | -R <rcfilepath>]
#
# -b => background mode
# -c => create the <outputdir>
# -C => cd to <workdir> defined by nbspgdradr(Cdir)
# -d => output directory
# -D => key=value,... comma separated list of gdradr(key)=var pairs
# -f => interpret <outputnamefmt> as a format string for clock seconds
# -k => keep the log file (the default is to delete it)
# -l => use the given logfile (implies -k).
# -o => outputname
# -r => rc file, searched in the standard directories
# -R => rc file path, used as given
# -t => cd to tmp directory (all partial paths are still relative
#       to the working directory)
#
# This program ends up calling gdradr.
# If the <rcfile> is not specified, the program uses the same logic as
# nbspradmap to search for the default file "gdradr.rc".
#
# This script loads the nbspradmos.init file (which in turn optionally
# loads the nbspradmos.conf file).
#
set usage {nbspgdradr [-b] [-c] [-C] [-d <outputdir>] [-D <defs>]
    [-f <outputnamefmt> | -o <outputname>]
    [-k] [-l <logfile>] [-t <tmpdir>] [-r <rcfile> | -R <rcfilepath>]};
set optlist {b c C {d.arg ""} {D.arg ""} {f.arg ""} {o.arg ""} k {l.arg ""}
    {t.arg ""} {r.arg ""} {R.arg ""}};

proc log_warn s {

    ::nbsp::syslog::warn $s;
}

proc log_err s {

    ::nbsp::syslog::err $s;
}

proc source_template {rcfile} {
#
# The template is sourced in a function so that the template cannot affect
# the main script environment.
#
    global gdradr;
    
    source $rcfile;
}

## The common initialization
set initfile "/usr/local/libexec/nbsp/nbspradmos.init";
if {[file exists $initfile] == 0} {
   log_err "$initfile not found.";
}
source $initfile;

# private settings
set nbspgdradr(_tmpfext) ".tmp";
set nbspgdradr(_logfext) ".log";

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];

if {$option(b) == 1} {
    ::nbsp::syslog::usesyslog
}

if {$option(C) == 1} {
    cd $nbspgdradr(Cdir);
}

if {$option(R) eq ""} {
    # Search for the rcfile 
    if {$option(r) eq ""} {
	set option(r) $nbspgdradr(rcfile);
    }
    set option(rcfile) [::nbsp::util::find_local_rcfile $option(r) \
			    $nbspgdradr(localconfdirs) \
			    $nbspgdradr(rcsubdir)];
} else {
    set option(rcfile) $option(R);
}

# Check that it exists
if {[file exists $option(rcfile)] == 0} {
    log_err "$option(rcfile) not found.";
}

# Definitions - Override any gdradr() settings in conf file.
if {$option(D) ne ""} {
    set Dlist [split $option(D) ","];
    foreach pair $Dlist {
        set p [split $pair "="];
        set var [lindex $p 0];
        set val [lindex $p 1];
        set gdradr($var) $val;
    }
}

if {$option(o) ne ""} {
    set gdradr(gdfile) $option(o);
} else {
    set now [clock seconds];
    if {$option(f) eq ""} {
	set option(f) $nbspgdradr(namefmt);
    }
    set gdradr(gdfile) [clock format $now -format $option(f) -gmt 1];
}

# logfile
if {$option(l) eq ""} {
    set option(l) $nbspgdradr(logfile);
}
if {$option(l) eq ""} {
    set outrootname [file rootname [file tail $gdradr(gdfile)]];
    append logfile $outrootname $nbspgdradr(_logfext);
} else {
    set logfile $option(l);
    set option(k) 1;
}

if {$option(d) ne ""} {
    set gdradr(gdfile) [file join $option(d) $gdradr(gdfile)];
    if {$option(c) == 1} {
	file mkdir $option(d);
    }
}

if {$option(t) ne ""} {
    if {[file isdirectory $option(t)] == 0} {
	log_err "No such directory: $option(t)";
    } else {
	set cwd [pwd];
	set option(rcfile) [file join $cwd $option(rcfile)];
	set gdradr(gdfile) [file join $cwd $gdradr(gdfile)];
	# If a logfile was given, it is relative to the original dir.
	if {$option(l) ne ""} {
	    set logfile [file join $cwd $logfile];
	}
	cd $option(t);
    }
}

file delete "gemglb.nts" "last.nts";
if {$option(k) == 0} {
    file delete $logfile;
}

# Create the data file in a tmp file, then rename it at the end
set _real_gdfile $gdradr(gdfile);
append gdradr(gdfile) $nbspgdradr(_tmpfext);

set status [catch {
    source_template $option(rcfile);
    if {[info exists gdradr(script)] == 0} {
	return;
    }
    set fout [open "|gdradr >& $logfile" w];
    fconfigure $fout -translation binary -encoding binary;
    set script [subst $gdradr(script)];
    puts $fout $script;
} errmsg];

if {[info exists fout]} {
    catch {close $fout};
}

file delete "gemglb.nts" "last.nts";
if {$option(k) == 0} {
    file delete $logfile;
}

if {$status != 0} {
    # In case gdradr created the file.
    file delete $gdradr(gdfile);
    log_err $errmsg;
} else {
    file rename -force $gdradr(gdfile) ${_real_gdfile};
    set gdradr(gdfile) ${_real_gdfile};     #in cast the post-script uses it
}

if {$nbspgdradr(latest_enable) == 1} {
    set cwd [pwd];
    cd [file dirname $gdradr(gdfile)];
    file delete $nbspgdradr(latest_name);
    file link -symbolic $nbspgdradr(latest_name) [file tail $gdradr(gdfile)];
    cd $cwd;
}

if {[info exists gdradr(post)]} {
    eval $gdradr(post);
}
