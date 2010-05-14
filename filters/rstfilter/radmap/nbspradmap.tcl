#!%TCLSH%
#
# $Id$
# 
# Usage: nbspradmap [-b] [-d <outputdir>] [-g gpmap_gif] [-h] \
#        [-K] [-L <logfile>] [-o <outputname>] [-p] [-s <outputsize>]
#        [-t <tmpdir>] [-D <defs>] <inputfile> [<rcfile>]
#
# -D => key=value,... comma separated list of gpmap(key)=var pairs
# -b => background mode
# -g => the name (or full path of the program)
# -h => the input file does not have the gempak header
# -d => output directory
# -K => keep (don't delete) the logfile
# -L => specify the logfile instead of the default
# -o => outputname
# -p => output png instead of the default gif (gpmap_gif only outputs gif)
# -s => image size. It is specified as, e.g, "1024;768".
#	If it is ";" then the original image size is used.
#	The default is "800;600" if nothing is specified.
# -t => cd to tmp directory (all partial paths are still relative
#       to the current directory.
#
# The input file can be the one with the compressed or uncompressed frames,
# but without the CCB, and it can have the gempak header. This is what
# gpmap_gif expects. If it does not have the gempak header, then [-h]
# must be given so that nbspradinfo is called with the correct [-c] option
# when the input file is of the uncompressed type (e.g., n0q and family). 
# If the <rcfile> is not specified, the program uses the same logic as the
# rstfilter to search for the default and use that (in this case the program
# uses the filterlib file, and also the rstfilter configuration file).
#
# If the variable "awips" is defined in the command line by using the -D
# option (e.g., -D awips=n0qjua) then this script executed nbspradinfo
# on the file and defines the following variables for the use of the rc
# script:
#
# gpmap(radinfo,lat)
# gpmap(radinfo,lon)
# gpmap(radinfo,height)
# gpmap(radinfo,seconds)
# gpmap(radinfo,mode)    (0 = maintenance, 1 = clean air, 2 = precip/severe)
# gpmap(radinfo,code)    (Nexrad product code - Table III of td7000.pdf)
#
package require cmdline;

set usage {nbspradmap [-b] [-d outputdir] [-g gpmap_gif] [-h] [-K] [-L logfile]
    [-o outputname] [-p] [-s outputsize] [-t <tmpdir>] [-D <defs>]
    <inputfile> [<rcfile>]};
set optlist {b p {d.arg ""} {g.arg "gpmap_gif"} h K {L.arg ""} {o.arg ""}
    {s.arg "800;600"} {t.arg ""} {D.arg ""}};

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

proc source_template_unused {rcfile} {
#
# The template is sourced in a slave so that the template cannot affect
# the main script environment.
#
    global gpmap;

    interp create slave;

    # Using the list cmd this way prevents the ";" in the mapsize option
    # to be interpreted as a cmd separator.
    foreach key [array names gpmap] {
	slave eval [list set gpmap($key) $gpmap($key)];
    }
    slave eval [list source $rcfile];

    foreach key [array names gpmap] {
	set gpmap($key) [slave eval {return $gpmap($key)}];
    }

    interp delete slave;
}

proc fill_gpmap_radinfo {doradinfounz_regexp} {

    global option;
    global gpmap;
    global filterslib;	# the header sizes

    set gpmap(radinfo,lat) "";
    set gpmap(radinfo,lon) "";
    set gpmap(radinfo,height) "";
    set gpmap(radinfo,seconds) "";
    set gpmap(radinfo,mode) "";
    set gpmap(radinfo,code) "";

    if {[info exists gpmap(rad,awips)] == 0} {
	return;
    }

    # This is copied from the filters.lib file, with the modifications needed
    # for the uncompressed files (e.g., n0q, ...) which do not
    # have the ccb header (since this script is assumed to operate on the
    # files without the ccb).

    if {[regexp $doradinfounz_regexp $gpmap(rad,awips)]} {
	set radinfo [split \
	    [exec nbspunz -c $filterslib(totalheadersize) -n 1 \
		 $gpmap(inputfile) | nbspradinfo]];
    } else {
	if {$option(h) == 0} {
	    # The file has a gempak header (e.g., from digatmos/nexrad)
	    set headersize $filterslib(wmoawipsgmpk_header_size);
	} else {
	    #
	    # The file does not have the gtempak header; such as the
	    # tmp files used by the rstfilter.
	    #
	    set headersize $filterslib(wmoawips_size);
	}

	set radinfo [split \
			 [exec nbspradinfo -c $headersize $gpmap(inputfile)]];
    }

    set gpmap(radinfo,lat) [lindex $radinfo 0];
    set gpmap(radinfo,lon) [lindex $radinfo 1];
    set gpmap(radinfo,height) [lindex $radinfo 2];
    set gpmap(radinfo,seconds) [lindex $radinfo 3];
    set gpmap(radinfo,mode) [lindex $radinfo 4];
    set gpmap(radinfo,code) [lindex $radinfo 5];
}

## The common defaults (e.g., netpbm progs and filterslib(doradinfounz))
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
    if {$rstfilter(radmap_rcfile_fpath) eq ""} {
	set option(rcfile) [filterlib_find_conf $rstfilter(radmap_rcfile) \
	    $rstfilter(radmap_rcdirs) $rstfilter(radmap_rcsubdir)];
    } else {
	set option(rcfile) $rstfilter(radmap_rcfile_fpath);
    }
} else {
    log_err $usage;
}

# Check that both exist
foreach _f [list $gpmap(inputfile) $option(rcfile)] {
    if {[file exists ${_f}] == 0} {
	log_err "${_f} not found.";
    }
}

# Definitions
if {$option(D) ne ""} {
    set Dlist [split $option(D) ","];
    foreach pair $Dlist {
        set p [split $pair "="];
        set var [lindex $p 0];
        set val [lindex $p 1];
        set gpmap(rad,$var) $val;
    }
}

# Fill the gpmap(radinfo,...) variables
# (requires -D awips=... and the gpmap(inputfile) defined).
#
fill_gpmap_radinfo $filterslib(doradinfounz);

set gpmapbin $option(g);

# gpmap_gif only outputs gif, even if "png" is set in the DEVICE in the rcfile.
set gpmap(fmt) "gif";

if {$option(s) eq ";"} {
    set gpmap(devsize)	"";
} else {
    set gpmap(devsize)	$option(s);
}

if {$option(o) ne ""} {
    set gpmap(outputfile) $option(o);
    set outrootname [file rootname [file tail $gpmap(outputfile)]];
} else {
    set outrootname [file rootname [file tail $gpmap(inputfile)]];
    append  gpmap(outputfile) $outrootname "." $gpmap(fmt);
}

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
	return;
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
    log_err "gpmap_gif did not produce $gpmap(outputfile).";
}

if {$option(p) == 1} {
    append pngoutfile [file rootname $gpmap(outputfile)] ".png";
    set status [catch {exec $filtersprogs(giftopnm) $gpmap(outputfile) \
        | $filtersprogs(pnmtopng) > $pngoutfile} errmsg];

    file delete $gpmap(outputfile);

    if {[file exists $pngoutfile] && ([file size $pngoutfile] != 0)} {
        set gpmap(outputfile) $pngoutfile;
        set status 0;
    } else {
        log_err "Could not convert $gpmap(outputfile) to png." ;
    }
}

if {[info exists gpmap(post)]} {
    eval $gpmap(post);
}
