#!/usr/local/bin/tclsh8.6
#
# Produces a csv file of the x,y coordinates and the CMI variable
# of a sat nc file. The three variables are converted to physical units
# using the scale and offset params extracted from the ncfile;
# in particular x, y are converted to rad.
#
#   Usage: nc2csv_xycmi ncfile
#
# To convert the xrad,yrad variables to lon,lat, the output must be piped
# to the "xy2lonlat" program,
#
# nc2csv_xycmi ncfile | xy2lonlat
#
# This is what the script "nc2cvs_lonlatcmi" does.
#
proc ncget_fgridparams {ncfile} {
    #
    # Extracts the projection parameters of a sat (cmi fixed grid) nc file,
    # and saves them in the global array g(), with the element names
    # defined by var(i) below.
    #
    # Example usage:
    #	fgridparams tire13.nc
    #
    # Only the first seven are parameters that change from file to file.
    # (The others are really constants.)
    #
    global rc;

    # keywords to search in the headers file.
    set keywords(1) "x:scale_factor";
    set keywords(2) "x:add_offset";
    set keywords(3) "y:scale_factor";
    set keywords(4) "y:add_offset";
    set keywords(5) "Sectorized_CMI:scale_factor";
    set keywords(6) "Sectorized_CMI:add_offset";
    set keywords(7) "fixedgrid_projection:longitude_of_projection_origin";
    set keywords(8) "fixedgrid_projection:latitude_of_projection_origin";
    set keywords(9) "fixedgrid_projection:semi_major";
    set keywords(10) "fixedgrid_projection:semi_minor";
    set keywords(11) "fixedgrid_projection:perspective_point_height";

    # the element names in the g array
    set var(1) "xscale_factor";
    set var(2) "xoffset";
    set var(3) "yscale_factor";
    set var(4) "yoffset";
    set var(5) "CMIscale_factor";
    set var(6) "CMIoffset";
    set var(7) "longitude_of_projection_origin";
    set var(8) "latitude_of_projection_origin";
    set var(9) "semi_major";
    set var(10) "semi_minor";
    set var(11) "perspective_point_height";
    #
    set Nelements 11;

    array set val {};

    set f [open "|ncdump -h $ncfile" "r"];
    while {[gets $f line] >= 0} {
	set line [string trim $line];
	if {$line eq ""} {
	    continue;
	}

	set i 1;
	while {$i <= $Nelements} {
	    if {[regexp "$keywords($i) = (.*) ;" $line match v]} {
		# this eliminates the final "f" in some parameters
		set rc($var($i)) [scan $v "%f"];
	    }
	    incr i;
	}
    }
    close $f;
}

proc ncget_var {varname ncfile} {
    #
    # Extracts the values of a variable "varname" in the "ncfile"
    # and returns them as a tcl list.
    #
    set re {$varname =(.*)};
    set outputdata "";

    set f [open "|ncdump -v $varname $ncfile" "r"];

    set data_line_found 0;
    # ignore all until we get the line of variable values
    while {[gets $f line] >= 0} {
	set line [string trim $line];
	
	if {[string equal $line "data:"] == 1} {
	    set data_line_found 1;
	}

	if {[regexp [subst $re] $line match data]} {
	    if {$data_line_found == 1} {
		#puts -nonewline [string map {" " ""} [string trim $data]];
		append outputdata [string map {" " ""} [string trim $data]];
		break;
	    }
	}
    }

    # print the rest of the lines, removing the newlines and spaces
    set done 0;
    while {[gets $f line] >= 0} {
	set line [string trim $line];
	set lastchar [string index $line end];
	if {$lastchar == ";"} {
	    set line [string trimright [string trimright $line ";"]];
	    set done 1;
	}
	#puts -nonewline [string map {" " ""} $line];
	append outputdata [string map {" " ""} $line];
	
	if {$done == 1} {
	    break;
	}
    }
    
    close $f;

    return [split $outputdata ","];
}

proc xtorad {x} {

    global rc;
    
    set v [expr ($x * $rc(xscale_factor) + $rc(xoffset)) * 0.000001];
    return $v;
}

proc ytorad {y} {

    global rc;

    set v [expr ($y * $rc(yscale_factor) + $rc(yoffset)) * 0.000001];
    return $v;
}

proc CMItoK {CMI} {

    global rc;

    set v [expr ($CMI * $rc(CMIscale_factor) + $rc(CMIoffset))];
    return $v;
}
    
# initialization
array set xrad {};
array set yrad {};
array set CMI {};	# in K

#
# main
#
if {$argc != 1} {
    puts "Usage: nc2csv_xycmi ncfile";
    return 1;
}

set ncfile [lindex $argv 0];

# init
array set rc {};
ncget_fgridparams $ncfile;

set i 0;
foreach line [ncget_var "x" $ncfile] {
    set xrad($i) [xtorad $line];
    incr i;
}
set Nx $i;

# y
set i 0;
foreach line [ncget_var "y" $ncfile] {
    set yrad($i) [ytorad $line];
    incr i;
}
set Ny $i;

# CMI
set i 0;	# index of x
set j 0;	# index of y
set k 0;	# count the number of elements in CMI()

foreach line [ncget_var "Sectorized_CMI" $ncfile] {
    set CMI($j,$i) [CMItoK $line];
    incr k;
    incr i;
    if {$i == $Nx} {
	set i 0;
	incr j;
    }
}
set Nr $k;

# convert x,y to lon,lat and output lon,lat,cmi
#set f [open "|./xy2lonlat" "w"];

set i 0;
set j 0;
while {($i < $Nx) && ($j < $Ny)} {
    puts "$xrad($i),$yrad($j),$CMI($j,$i)";
    #puts $f "$xrad($i),$yrad($j),$CMI($j,$i)";
    incr i;
    if {$i == $Nx} {
	set i 0;
	incr j;
    }
}

#close $f;
