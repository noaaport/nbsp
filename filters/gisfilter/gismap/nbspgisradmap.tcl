#!%TCLSH%
#
# $Id$
# 
# Usage: nbspgisradmap [-b] [-k] [-v] [-a <awips1> [-A]]
#                  [-d <outputdir>] [-D <defines>]
#                  [-e <extent>] [-f <mapfonts_dir>] [-g <geodata_dir>]
#                  [-i] [-I <inputdir>] [-m | -M <map_template>] [-n <index>]
#                  [-o <outputfile>] [-p <patt>] [-s <size>] [-t <imgtype>]
#                  <file1> ... <filenn>
#
# This is cmdline tool with no configuration file.
# The <filen> arguments can be given in the command line or in stdin.
# When there is more than one file, all must be of the same type (e.g. n0r).
# The inputfile names must have the format
#
# <awips><anything>
#
# Then from the the <awips> the map template to use is determined as well
# as the extent (for the site(s)).
#
# Examples:
#
#    nbspgisradmap n0rjua_20110811_1755.nids
#
#    (These four are equivalent)
#
#    nbspgisradmap -I /var/noaaport/data/digatmos/nexrad/nids \
#                  -p "n0r/*.nids" fdr inx tlx vnx
#
#    nbspgisradmap -i -I digatmos/nexrad/nids \
#                  -p "n0r/*.nids" fdr inx tlx vnx
#
#    nbspgisradmap -a n0r fdr inx tlx vnx
#
#    nbspgisradmap -a n0r -A ok
#
#
# -I => parent directory for the arguments to the program.
# -i => prepend common(datadir) to the argument given in -I
# -p => the arguments are interpreted as subdirectories of the -I parent dir.
#       Then the list of files is constructed using the glob <patt>, sorted
#       in decreasing order, and the -n <index> option is used to select
#       the file. The default is the most recent file (index = 0).
# -a => is equivalent to set -i, -I digatmos/nexrad/nids, and
#       -p <awips1>/*.nids  (where <awips1> is the argument to -a).
# -A => if -a is given, then this option instructs to look every argument
#       and take any two-letter word as the initials of a state, then
#       expand it as a list of stations and add the list to the argument list.
#       (When this option is used, the assumption is that the directories
#       that are given in the arguments, are the names of the sites, as in
#       the default configuration of the dafilter).
# -b => background mode
# -k => keep the generated shp files (when the input are nids)
# -d => output directory
# -D => key=value,... comma separated list of map(key)=var pairs
#       (in practice, extent=...,size=...
#	The extent size values can be separated by spaces or ";", for example
#	-D size="sx sy",extent="a b c d" or -D size=sx;sy,extent=a;b;c;d
# -e => extent
# -f => map fonts directory
# -g => geodata directory
# -m => map template (use as is)
# -M => map temnplate name (without the ext and look in the std directories)
# -o => name of outputfile (otherwise the default is used)
# -s => size
# -t => imgtype (png)

set usage {nbspgisradmap [-b] [-k] [-v]
    [-a <awips1> [-A]] [-d <outputdir>] [-D <defines>]
    [-e <extent>] [-f <mapfontsdir>] [-g <geodatadir>] [-i] [-I <inputdir>]
    [-m | -M <map_template>] [-n <index>] [-o <outputfile>] [-p <patt>]
    [-s <size>] [-t <imgtype>] <file1> ... <filen>};

set optlist {b i k v A {a.arg ""} {d.arg ""} {D.arg ""} {e.arg ""} {f.arg ""}
    {g.arg ""} {I.arg ""} {m.arg ""} {M.arg ""} {n.arg 0} {o.arg ""} {p.arg ""}
    {s.arg ""} {t.arg ""}};

package require cmdline;

# Source filters.init so that the templates can "require" locally
# installed packages (e.g., map_rad requires gis.tcl)
#
source "/usr/local/libexec/nbsp/filters.init";
package require nbsp::radstations;

# Defaults
#
# map(extent) will be determined
# map(size) and map(imagetype) are optional
#
foreach d $common(localsharedirs) {
    if {[file isdirectory [file join $d geodata]]} {
	set map(geodata) [file join $d geodata];
    }

    if {[file isdirectory [file join $d mapfonts]]} {
	set map(mapfonts) [file join $d mapfonts];
    }
}

# The defauls map directories
set nbspgismap(map_dir) [list];
foreach d $common(localconfdirs) {
    if {[file isdirectory [file join $d gis map]]} {
	lappend nbspgismap(map_dir) [file join $d gis map];
    }
}

# parameters
set nbspgismap(map_rc_fext) ".map";
set nbspgismap(map_tmpl_fext) ".tmpl";
set nbspgismap(map_tmpl_namefmt) "map_rad_%s";

# variables
set nbspgismap(map_tmplfile) "";
set nbspgismap(map_rcfile) "";
set nbspgismap(input_files_list) [list];
set nbspgismap(outputfile) "";

# inherited
set nbspgismap(datadir) $common(datadir);

# default options if -a is given
set option(default_I) "digatmos/nexrad/nids";
set option(default_p) {%s/*.nids};

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

proc log_msg s {

    global argv0;
    global option;

    if {$option(v) == 0} {
	return;
    }

    set name [file tail $argv0];
    if {$option(b) == 0} {
        puts $s;
    } else {
        exec logger -t $name $s;
    }
}

proc check_conflicts {usage} {

    global option;

    set conflict_mM 0;
    
    if {$option(m) ne ""} {
	incr conflict_mM;
    }

    if {$option(M) ne ""} {
	incr conflict_mM;
    }

    if {$conflict_mM > 1} {
	log_err $usage;
    }
}

proc get_input_files_list {argv} {

    global nbspgismap option;

    set nbspgismap(input_files_list) [list];

    set flist [list];
    if {$option(I) ne ""} {
	foreach f $argv {
	    lappend flist [file join $option(I) $f];
	}
    } else {
	set flist $argv;
    }
    
    if {$option(p) eq ""} {
	set nbspgismap(input_files_list) $flist;
	return;
    }

    set dirlist $flist;
    foreach dir $dirlist {
	if {[file isdirectory $dir] == 0} {
	    log_warn "Skiping $dir; not found.";
	    continue;
	}
	# Exclude the latest links in the glob
	set flist [list];
	foreach f [lsort -decreasing \
		       [glob -nocomplain -directory $dir $option(p)]] {
	    if {[file type $f] ne "link"} {
		lappend flist $f;
	    }
	}

	set f [lindex $flist $option(n)];
	if {$f ne ""} {
	    lappend nbspgismap(input_files_list) $f;
	}
    }
}

proc verify_inputfile_namefmt {inputfile} {

    set fbasename [file tail $inputfile];
    # set re {^[[:alnum:]]{6}.*\.nids$};  
    set re {^[[:alnum:]]{6}};           # don't require nids extension

    if {[regexp $re $fbasename] == 0} {
	return -code error "Invalid input file name: $inputfile";
    }
}

proc select_mapname_keyword {awips1} {

    if {[regexp {^n.(r|q|z)$} $awips1]} {
	set mapname "bref";
    } elseif {[regexp {^n.(u|v)$} $awips1]} {
	set mapname "rvel";
    } elseif {[regexp {^n(1|3|t)p$} $awips1]} {
	set mapname "nxp";
    } else {
	return -code error "Unsupported nids type: $awips1";
    }

    return $mapname;
}

proc expand_argv {inputlist} {

    set outputlist [list];
    foreach a $inputlist {
        if {[string length $a] == 2} {
            set slist [::nbsp::radstations::bystate $a];
            set outputlist [concat $outputlist $slist];
        } else {
            lappend outputlist $a;
        }
    }

    return $outputlist;
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

check_conflicts $usage;

if {$argc == 0} {
    set argv [split [read stdin]];
}

# set defaults if -a is given
if {$option(a) ne ""} {
    set option(i) 1;
    if {$option(I) eq ""} {
	set option(I) $option(default_I);
    }

    if {$option(p) eq ""} {
	set option(p) [format $option(default_p) $option(a)];
    }

    if {$option(A) == 1} {
	set argv [expand_argv $argv];
    }
}

if {($option(I) ne "") && ($option(i) == 1)} {
    set option(I) [file join $nbspgismap(datadir) $option(I)];
}

get_input_files_list $argv;

if {[llength $nbspgismap(input_files_list)] == 0} {
    log_err "No files found.";
}

set sitelist [list];
foreach inputfile $nbspgismap(input_files_list) {
    verify_inputfile_namefmt $inputfile;

    set awips [string range [file tail $inputfile] 0 5];
    set site [string range $awips 3 5];

    if {[info exists awips1] == 0} {
	set awips1 [string range $awips 0 2];
    } elseif {[string range $awips 0 2] ne $awips1} {
	log_err "Input files have different data type.";
    }

    lappend sitelist $site;
}

# This returns a keyword: "bref", "rvel", ...
set mapname_keyword [select_mapname_keyword $awips1];

if {[regexp {^n.(r|v|z)} $awips1] || [regexp {^n(1|3|t)p$} $awips1]} {
    set do_nbspunz 1;
    set map(extent) [::nbsp::radstations::extent_bysitelist $sitelist];
} else {
    set do_nbspunz 0;
    if {[llength $sitelist] == 1} {
	set map(extent) [::nbsp::radstations::extent_bysite $site 4];
    } else {
	set map(extent) [::nbsp::radstations::extent_bysitelist $sitelist];
    }
}

if {$option(e) ne ""} {
    set map(extent) $option(e);
}

if {$option(g) ne ""} {
    set map(geodata) $option(g);
}

if {$option(f) ne ""} {
    set map(mapfonts) $option(f);
}

if {$option(m) ne ""} {
    set nbspgismap(map_tmplfile) $option(m);
} else {
    if {$option(M) ne ""} {
	set map_template $option(M);
    } else {
	set map_template [format $nbspgismap(map_tmpl_namefmt) \
			      $mapname_keyword];
    }

    if {[file extension $map_template)] ne $nbspgismap(map_tmpl_fext)} {
	append map_template $nbspgismap(map_tmpl_fext);
    }

    foreach d $nbspgismap(map_dir) {
	if {[file exists [file join $d $map_template]]} {
	    set nbspgismap(map_tmplfile) [file join $d $map_template];
	}
    }
}

# The defines
if {$option(D) ne ""} {
    set Dlist [split $option(D) ","];
    foreach pair $Dlist {
	set p [split $pair "="];
	set var [lindex $p 0];
	set val [lindex $p 1];
	set map($var) "$val";    # extent and size can contain spaces 
    }
}

foreach k [list extent geodata mapfonts] {
    if {[info exists map($k)] == 0} {
	log_err "No $k specified.";
    }
}

if {$nbspgismap(map_tmplfile) eq ""} {
    log_err "Map template file not found.";
} elseif {[file exists $nbspgismap(map_tmplfile)] == 0} {
    log_err "$nbspgismap(map_tmplfile) not found.";
}

#
# process files
#
set shpname_list [list];
foreach inputfile $nbspgismap(input_files_list) {
    set shpname [file rootname [file tail $inputfile]];
    log_msg "Converting $inputfile to shapefile ...";
    if {$do_nbspunz == 1} {
	exec nbspunz $inputfile | nbspradgis -C -n $shpname;
    } else {
	exec nbspradgis -n $shpname $inputfile;
    }
    lappend shpname_list $shpname;
}

#
# use this or \"$extent\" in the open proc.
#
set map(extent) [join $map(extent) ";"];
 
set cmd [list "|nbspgismap" -e $map(extent) \
	     -f $map(mapfonts) \
	     -g $map(geodata) \
	     -m $nbspgismap(map_tmplfile)];

if {$option(b) == 1} {
    lappend cmd "-b";
}

if {$option(D) ne ""} {
    append option(D) "," "awips1=$awips1";
} else {
    set option(D) "awips1=$awips1";
}

foreach k [list d D o s t] {
    if {$option($k) ne ""} {
	lappend cmd "-${k}" $option($k);
    }
}

log_msg "Creating map ...";

set F [open [join $cmd] "w"];
puts $F [join $shpname_list "\n"];
close $F;

if {$option(k) == 0} {
    foreach ext [list "shp" "shx" "dbf" "info"] {
	foreach shpname $shpname_list {
	    set f ${shpname}.${ext};
	    file delete $f;
	}
    }
}
