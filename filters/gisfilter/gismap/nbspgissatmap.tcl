#!%TCLSH%
#
# $Id$
# 
# Usage: nbspgissatmap [-a] [-b] [-k] [-q] [-v] [-d <outputdir>] [-D <defines>]
#                  [-e <extent>] [-f <mapfonts_dir>] [-g <geodata_dir>]
#                  [-i] [-I <inputdir>] [-m | -M <map_template>] [-n <index>]
#                  [-o <outputfile>] [-p <patt>] [-r <lon1,lat1,lon2,lat2;...>
#                  [-s <size>] [-t <imgtype>] <file1> ... <filenn>
#
# This is cmdline tool with no configuration file.
# The <filen> arguments can be given in the command line or in stdin.
# When there is more than one file, all must be of the same type (e.g. 01).
# The inputfile names must have the format
#
# <tigxxx><anything>
#
# Then from the the <wmoid> the map template to use is determined.
#
# Examples:
#
#    nbspgissatmap tig01_<date>.gini
#
#    The following three are equivalent
#
#    nbspgissatmap -I /var/noaaport/data/digatmos/sat/gini/tig \
#                  -p "*.gini" tigw01 tige01
#
#    nbspgissatmap -a tigw01 tige01
#
#
#    These two are equivalent
#
#    nbspgissatmap -i -I digatmos/sat/gini/tig -p "*.gini" \
#                  -q -r "0,0,5,0;5,0,0,0" tigw01 tige01
#
#    nbspgissatmap -a -q -r "0,0,5,0;5,0,0,0" tigw01 tige01
#
# -I => parent directory for the arguments to the program.
# -i => prepend common(datadir) to the argument given in -I
# -p => the arguments are interpreted as subdirectories of the -I parent dir.
#       Then the list of files is constructed using the glob <patt>, sorted
#       in decreasing order, and the -n <index> option is used to select
#       the file. The default is the most recent file (index = 0).
# -a => is equivalent to set -i, -I digatmos/sat/gini/tig, and
#       -p "*.gini" (it does not require an argument since there is no awips1).
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
# -q, -r => are passed intact to nbspsatgis for controlling the bounding box
#           of each asc file

set usage {nbspgissatmap [-a] [-b] [-k] [-v] [-q]
    [-d <outputdir>] [-D <defines>] [-e <extent>] [-f <mapfontsdir>]
    [-g <geodatadir>][-i] [-I <inputdir>]
    [-m | -M <map_template>] [-n <index>] [-o <outputfile>] [-p <patt>]
    [-r <lon1,lat1,lon2,lat2;...>] [-s <size>] [-t <imgtype>]
    <file1> ... <filen>};

set optlist {a b i k v q {d.arg ""} {D.arg ""} {e.arg ""} {f.arg ""} {g.arg ""}
    {I.arg ""} {m.arg ""} {M.arg ""} {n.arg 0} {o.arg ""} {p.arg ""}
    {r.arg ""} {s.arg ""} {t.arg ""}};

package require cmdline;

# Source filters.init so that the templates can "require" locally
# installed packages
#
source "/usr/local/libexec/nbsp/filters.init";

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
set nbspgismap(map_tmpl_namefmt) "map_sat_%s";
set nbspgismap(map_tmpl_default) "map_sat";
set nbspgismap(ascfext) ".asc";

# variables
set nbspgismap(map_tmplfile) "";
set nbspgismap(map_rcfile) "";
set nbspgismap(input_files_list) [list];
set nbspgismap(outputfile) "";

# inherited
set nbspgismap(datadir) $common(datadir);

# default options if -a is given
set option(default_I) "digatmos/sat/gini/tig";
set option(default_p) {*.gini};

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
    # set re {^[[:alnum:]]{6}.+\.gini$};
    set re {^[[:alnum:]]{6}};

    if {[regexp $re $fbasename] == 0} {
	return -code error "Invalid input file name: $inputfile";
    }
}

proc select_mapname_keyword {wmoid} {

    if {[regexp {^tig} $wmoid]} {
	set mapname "";
    } else {
	return -code error "Unsupported type: $wmoid";
    }

    return $mapname;
}

#
# Functions to support the determination of the default extent from
# the asc files themselves.
#
proc get_extent_from_ascfile_list {file_list} {
#
# This is the main function of the combo. The other two, next,
# are used in this one.
#
    set extent [list];

    foreach f $file_list {
	set new_extent [get_extent_from_ascfile $f];
	set extent [update_extent $extent $new_extent];
    }

    return $extent;
}

proc get_extent_from_ascfile {file} {

    set data [split [string trim [exec head -n 6 $file]] "\n"];
    foreach line $data {
	set key [lindex [split $line] 0];
	set val [lindex [split $line] 1];
	set a($key) $val;
    }

    set lon1 $a(xllcorner);
    set lat1 $a(yllcorner);
    set lon2 [expr $lon1 + $a(ncols) * $a(cellsize)];
    set lat2 [expr $lat1 + $a(nrows) * $a(cellsize)];

    return [list $lon1 $lat1 $lon2 $lat2];
}

proc update_extent {old_extent new_extent} {

    if {[llength $old_extent] == 0} {
	return $new_extent;
    }
    
    set lon1 [lindex $old_extent 0];
    set new [lindex $new_extent 0];
    if {$new < $lon1} {
	set lon1 $new;
    }

    set lat1 [lindex $old_extent 1];
    set new [lindex $new_extent 1];
    if {$new < $lat1} {
	set lat1 $new;
    }
    
    set lon2 [lindex $old_extent 2];
    set new [lindex $new_extent 2];
    if {$new > $lon2} {
	set lon2 $new;
    }

    set lat2 [lindex $old_extent 3];
    set new [lindex $new_extent 3];
    if {$new > $lat2} {
	set lat2 $new;
    }
    
    return [list $lon1 $lat1 $lon2 $lat2];
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
	set option(p) $option(default_p);
    }
}

if {($option(I) ne "") && ($option(i) == 1)} {
    set option(I) [file join $nbspgismap(datadir) $option(I)];
}

get_input_files_list $argv;

if {[llength $nbspgismap(input_files_list)] == 0} {
    log_err "No files found.";
}

set regionlist [list];
foreach inputfile $nbspgismap(input_files_list) {
    verify_inputfile_namefmt $inputfile;

    set wmoid [string range [file tail $inputfile] 0 5];
    set region [string range $wmoid 0 3];

    if {[info exists type] == 0} {
	set type [string range $wmoid 4 5];
    } elseif {[string range $wmoid 4 5] ne $type} {
	err "Input files have different data type.";
    }

    lappend regionlist $region;
}

# This returns a keyword or ""
set mapname_keyword [select_mapname_keyword $wmoid];

if {$option(r) ne ""} {
    set optr_list [split $option(r) ";"];
} else {
    set optr_list [list];
}

set ascfile_list [list];
foreach inputfile $nbspgismap(input_files_list) {
    set ascname [file rootname [file tail $inputfile]];
    set cmd [list exec nbspunz $inputfile | nbspsatgis -A -n $ascname];

    if {[llength $optr_list] != 0} {
	set opts [list "-r" [lindex $optr_list 0]];
	set optr_list [lreplace $optr_list 0 0];
	if {$option(q) == 1} {
	    lappend opts "-q";
	}
	set cmd [concat $cmd $opts];
    }

    eval $cmd;

    lappend ascfile_list ${ascname}$nbspgismap(ascfext);
}

# Get the default extent from the asc files
set map(extent) [get_extent_from_ascfile_list $ascfile_list];

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
    } elseif {$mapname_keyword ne ""} {
	set map_template [format $nbspgismap(map_tmpl_namefmt) \
			      $mapname_keyword];
    } else {
	set map_template $nbspgismap(map_tmpl_default);
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

if {[file exists $nbspgismap(map_tmplfile)] == 0} {
    log_err "$nbspgismap(map_tmplfile) not found.";
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

foreach k [list d D o s t] {
    if {$option($k) ne ""} {
	lappend cmd "-${k}" $option($k);
    }
}

log_msg "Creating map ...";

set F [open [join $cmd] "w"];
puts $F [join $ascfile_list "\n"];
close $F;

if {$option(k) == 0} {
    foreach f $ascfile_list {
	file delete $f;
    }
}
