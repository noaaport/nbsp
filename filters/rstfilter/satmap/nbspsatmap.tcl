#!%TCLSH%
#
# $Id$
# 
# Usage: nbspsatmap [-b] [-d <outputdir>] [-g <gpmap_gif>] [-k]
#       [-K] [-L <logfile>] [-p] [-q] [-s <outputsize>]
#       [-t <tmpdir>] [-D <defs>] <inputfile> [<rcfile>]
#
# -b => background mode
# -d => output directory
# -D => key=value,... comma separated list of gpmap(key)=var pairs
# -g => the name (or full path of the gpmap_gif program)
# -k => keep the gif file when [-p] is given
# -K => keep the log file when finished 
# -L => specify the logfile instead of the default
# -p => output png instead of the default gif (gpmap_gif only outputs gif)
# -q => silent (no normal output) [except for errors]
# -s => image size. It is specified as, e.g, "1024;768".
#	If it is ";" then the original image size is used.
#	The default is "800;600" if nothing is specified.
# -t => tmp dir (all paths are relative to current directory)
#
# The input file can the one with the compressed frames
# or the uncompressed gini file.
# If the <rcfile> is not specified, the program uses the same logic as the
# rstfilter to search for the default and use that (in this case the program
# uses the filterlib file, and also the rstfilter configuration file).
#
package require cmdline;

set usage {nbspsatmap [-b] [-d outputdir] [-D <defs>] [-g gmap_gif]
    [-k] [-K] [-L <logfile>] [-p] [-q]
    [-s outputsize] [-t <tmpdir>] <inputfile> [<rcfile>]};
set optlist {b {d.arg ""} {D.arg ""} {g.arg "gpmap_gif"} k K {L.arg ""} p q
    {s.arg "800;600"} {t.arg ""} };

array set option [::cmdline::getoptions argv $optlist $usage];

proc log_warn s {

    global argv0;
    global option;

    set name [file tail $argv0];
    if {$option(b) == 0} {
        puts "$name: $s";
    } else {
        exec logger -t $name $s;
    }
}

proc log_err s {

    log_warn $s;
    exit 1;
}

proc source_template {rcfile} {
#
# The template is sourced in a function so that the template cannot affect
# the main script environment.
#
    global gpmap;
    
    source $rcfile;
}

## The common defaults (to get the location of the netpbm progs).
set defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaultsfile] == 0} {
   log_err "$defaultsfile not found.";
}
source $defaultsfile;

set gpenvfile $gempak(envfile);
if {[file exists $gpenvfile] == 0} {
    log_err "$gpenvfile not found.";
}
source $gpenvfile;

#
# main
#
set argc [llength $argv];
if {$argc == 2} {
    set gpmap(inputfile) [lindex $argv 0];
    set option(rcfile) [lindex $argv 1];
} elseif {$argc == 1} {
    set gpmap(inputfile) [lindex $argv 0];
    # Search for the rcfile using the same logic in the rstfilter
    source $common(filterslib);
    source [file join $common(libdir) "rstfilter.init"];
    if {$rstfilter(satmap_rcfile_fpath) eq ""} {
	set option(rcfile) [filterlib_find_conf $rstfilter(satmap_rcfile) \
	    $rstfilter(satmap_rcdirs) $rstfilter(satmap_rcsubdir)];
    } else {
	set option(rcfile) $rstfilter(satmap_rcfile_fpath);
    }
} else {
    log_err $usage;
}

# Definitions
if {$option(D) ne ""} {
    set Dlist [split $option(D) ","];
    foreach pair $Dlist {
        set p [split $pair "="];
        set var [lindex $p 0];
        set val [lindex $p 1];
        set gpmap(sat,$var) $val;
    }
}

set gpmapbin $option(g);

# gpmap_gif only outputs gif, even if "png" is set in the DEVICE in $gpmaprc
set gpmap(fmt) "gif";

if {$option(s) eq ";"} {
    set gpmap(devsize)	"";
} else {
    set gpmap(devsize) $option(s);
}

# Now construct the command line for nbspsat. We pass [-i] to nbspsat
# to extract the information without procesing. 
set nbspsatopts [list "-i"];
if {$option(b) == 1} {
    lappend nbspsatopts "-b";
}

set params [eval exec $filtersprogs(nbspsat) $nbspsatopts $gpmap(inputfile)];
set sector [lindex $params 2];
set channel [lindex $params 3];
set res [lindex $params 4];
set time [lindex $params 5];
set outfname [lindex $params 6];

# nbspsat creates png files and the outfname parameter here has the .png.
set outrootname [file rootname $outfname];
append  gpmap(outputfile) $outrootname "." $gpmap(fmt);

if {$option(L) eq ""} {
    append logfile $outrootname ".log";
} else {
    set logfile $option(L);
}

if {$option(d) ne ""} {
    set gpmap(outputfile) [file join $option(d) $gpmap(outputfile)];
}

if {$option(t) ne ""} {
    if {[file isdirectory $option(t)] == 0} {
	log_err "No such directory: $option(t)";
    } else {
	set cwd [pwd];
	set gpmap(inputfile) [file join $cwd $gpmap(inputfile)];
	set option(rcfile) [file join $cwd $option(rcfile)];
	set gpmap(outputfile) [file join $cwd $gpmap(outputfile)];

	cd $option(t);
    }
}

file delete $gpmap(outputfile);
file delete "gemglb.nts" "last.nts";
if {$option(K) == 0} {
    file delete $logfile;
}

set status [catch {
    source_template $option(rcfile);
    if {[info exists gpmap(script)] == 0} {
	log_err "gpmap(script) undefined.";
    }
    set fout [open "|$gpmapbin >& $logfile" w];
    fconfigure $fout -translation binary -encoding binary;
    set script [subst $gpmap(script)];
    puts $fout $script;
} errmsg];

if {[info exists fout]} {
    catch {close $fout};
}

file delete "gemglb.nts" "last.nts";
if {$option(K) == 0} {
    file delete $logfile;
}

if {$status != 0} {
    # In case gpmap created the file.
    file delete $gpmap(outputfile);
    log_err $errmsg;
}

# It is possible that gpmap_gif did not produce the image.
if {[file exists $gpmap(outputfile)] == 0} {
    set _msg "Inputfile: $gpmap(inputfile).\n";
    append _msg "gpmap_gif did not produce a map for $gpmap(outputfile).\n";
    append _msg "Probably: No entry in the image type table, imgtyp.tbl.";
    log_err ${_msg};
}

if {$option(p) == 1} {
    append pngoutfile [file rootname $gpmap(outputfile)] ".png";
    set status [catch {exec $filtersprogs(giftopnm) $gpmap(outputfile) \
        | $filtersprogs(pnmtopng) > $pngoutfile} errmsg];

    if {$option(k) == 0} {
	file delete $gpmap(outputfile);
    }

    if {[file exists $pngoutfile] && ([file size $pngoutfile] != 0)} {
        set gpmap(outputfile) $pngoutfile;
        set status 0;
    } else {
	file delete $gpmap(outputfile);
        log_err "Could not convert $gpmap(outputfile) to png." ;
    }
}

if {$option(q) == 0} {
    puts [lreplace $params 6 6 $gpmap(outputfile)];
}

if {[info exists gpmap(post)]} {
    eval $gpmap(post);
}
