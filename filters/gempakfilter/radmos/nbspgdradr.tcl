#!%TCLSH%
#
# $Id$
# 
# Usage: nbspgdradr [-b] [-c] [-C <workdir>] [-d <outputdir>]
#        [-f <outputnamefmt> | -o <outputname>] [-k] [-l <logfile>]
#        [-t <tmpdir>] [-D <defs>] [-r <rcfile> | -R <rcfilepath>]
#
# -b => background mode
# -c => create the <outputdir>
# -C => cd to <workdir>
# -d => output directory
# -D => key=value,... comma separated list of gdradr(key)=var pairs
# -f => interpret <outputnamefmt> as a format string for clock seconds
# -k => keep the log file (the default is to delete it)
# -l => use the given logfile (implies -k).
# -o => outputname
# -t => cd to tmp directory (all partial paths are still relative
#       to the working directory)
# -r => rc file, searched in the standard directories
# -R => rc file path, used as given
#
# This program ends up calling gdradr.
# If the <rcfile> is not specified, the program uses the same logic as
# nbspradmap to search for the default file "gdradr.rc".
#
set usage {nbspgdradr [-b] [-c] [-C <workdir>] [-d <outputdir>]
    [-f <outputnamefmt> | -o <outputname>]
    [-k] [-l <logfile>] [-t <tmpdir>] [-D <defs>]
    [-r <rcfile> | -R <rcfilepath>]};
set optlist {b c {C.arg ""} {d.arg ""} {f.arg ""} {o.arg ""} k {l.arg ""}
    {t.arg ""} {D.arg ""} {r.arg ""} {R.arg ""}};

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

## The common defaults
set defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaultsfile] == 0} {
   log_err "$defaultsfile not found.";
}
source $defaultsfile;

if {[file exists $gempak(envfile)] == 0} {
    log_err "$gempak(envfile) not found.";
}
source $gempak(envfile);

# Packages from tcllib
package require cmdline;

# Nbsp packages
## The errx library. syslog enabled below if -b is given.
package require nbsp::errx;
package require nbsp::util;

# Defaults
set nbspgdradr(localconfdirs) $common(localconfdirs);
set nbspgdradr(conf)      [file join $common(confdir) "nbspgdradr.conf"];
#
set nbspgdradr(rcsubdir)  "gdradr";
set nbspgdradr(rcfile)    "gdradr.rc";
set nbspgdradr(namefmt)   "%Y%m%d_%H%M.gem";

# optional config file
if {[file exists $nbspgdradr(conf)]} {
    source $nbspgdradr(conf);
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];

if {$option(b) == 1} {
    ::nbsp::syslog::usesyslog
}

if {$option(C) ne ""} {
    cd $option(C);
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

if {$option(l) eq ""} {
    set outrootname [file rootname [file tail $gdradr(gdfile)]];
    append logfile $outrootname ".log";
} else {
    set logfile $option(l);
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
}

if {[info exists gdradr(post)]} {
    eval $gdradr(post);
}
