#!%SHELL%
#
# Copyright (c) 2025 Jose F. Nieves <nieves@ltp.uprrp.edu>
#
# See LICENSE
# 
# $Id$
#
# Usage: nbspgoesrmap [-baCDk] [-f mapfontsdir] [-g geodatadir]
#                     [-m user_map_template] [-o output_file] <ncfile>
#
# This is a (shell) script with no configuration file. The input <ncfile>
# is the netcdf file (e.g., tire05_20250116_1256.goesr)
#
# -b => background
# -a => the inputfile is the asc file instead of the netcdf file
# -C => writeout the map template and exit (can be edited and submitted with -m)
# -D => writeout the map and exit (can be edited and submitted to map2img)
# -k => keep all tmp files generated (asc, map and map template)
# -f => mapfonts dir (default is /usr/local/share/nbsp/defaults/mapfonts)
# -g => geodata dir (default is /usr/local/share/nbsp/defaults/geodata)
# -m => specify a map template to use (it is not deleted even if -k is not set)
# -o => output (png) file
#
# In the simplest use,
#
#   nbspgoesrmap <ncfile>  (e.g. tire05_20250116_1256.goesr)
#
# will produce the png file through the following steps:
#
# 1) Write out the default map template (goesr.map.in)
#
# 2) Determine the extent and size to be used in the "map2img" map file
#    by executing
#
#    nbspgoesr -i <ncfile>
#
#    and extracting the relevant parameters from the output. If [-a]
#    is given the parameters are extracted from the asc file.
#
# 3) Converts the map template to the map file (goesr.map)
#    using "sed" to substitute the various parameters by the
#    values extracted in (2), and the name of the ascfile (determined
#    from the name of the input file).
#
# 4) Execute
#
#    nbspgoesrgis -a <ascfile> <ncfile>
#
#    to create the asc file that will be used in the map2img map file.
#    If [-a] is given this step is omitted.
#
# 5) Execute
#
#    map2img -m <mapfile> -o <outputfile>
#
#    using the mapfile created in (3).
#
# The options modify the default behaviour. In particular, the -C option
# can be used to write out the default map template used, which can then
# be manually edited and modified, and then submit as the map template
# to use with -m option. The [-D] writes out the map that is used with map2img
# after making the substitutions in the map template.

#
# functions
#

log_msg(){

    if [ $gbackground -eq 1 ]
    then
        logger -t emftp "$1"
    else
        echo "$1"
    fi
}

log_msg_quit(){

    log_msg "$1"
    exit 0
}

log_err(){

    log_msg "$1"
}

log_err_quit(){

    log_msg "$1"
    exit 1
}

cleanup(){

    # if -k was set then don't delete anything
    [ $option_k -eq 1 ] && { return; }

    # The asc file is removed if it was created
    [ $option_a -eq 0 ] && rm -f $rc_ascfile

    # The map and template are not removed if the flag is set
    [ $f_keep_map -eq 0 ] && { rm -f $gmapfile; }
    [ $f_keep_map_in -eq 0 ] && { rm -f $gmapfile_in; }
}

sanity_check() {

    # If the -m option was given (a map template was specified), it
    # must exist
    if [ $option_m -eq 1 -a ! -f $gmapfile_in ]
    then
	log_err_quit "Not found: $gmapfile_in"
    fi

    # input file
    [ ! -f $ginputfile ] && { log_err_quit "Not found: $ginputfile"; }
}

make_map_in () {

    cat << EOF > $gmapfile_in
#
# The "rc_xxx" parameters will be substituted by their values
# extracted from the ascfile to produce the map file that will be
# used with map2img.
#
MAP 
    UNITS  DD
    EXTENT rc_lon1 rc_lat1 rc_lon2 rc_lat2
    SIZE rc_nx rc_ny
    IMAGETYPE png
    IMAGECOLOR 0 0 0
    # FONTSET "${gmapfontsdir}/fonts.list"
    IMAGETYPE png
 
    LAYER
	NAME sat-1
	DATA rc_ascfile		# asc file
	TYPE RASTER
	STATUS ON
	#TRANSPARENCY 100
	#OPACITY 0
	#
	# PROCESSING "SCALE=0,80"
	# PROCESSING "SCALE_BUCKETS=16"
	#
    END

    LAYER
	TYPE POLYGON
	NAME "world"
	STATUS ON
	DATA "${ggeodatadir}/world/world.shp"
	CLASS
	    NAME "world-outline"
	    OUTLINECOLOR 255 255 255
	END
    END

    LAYER
	TYPE POLYGON
	NAME "states"
	STATUS ON
	DATA "${ggeodatadir}/s_01au07/s_01au07.shp"
	CLASS
	    NAME "states-outline"
	    OUTLINECOLOR 255 255 255
	END
    END
END
EOF
}

make_map () {
    #
    # This is not used if the map template below contains the
    # variables $rc_xx themselves rather than the names.
    #
    sed \
	-e "/rc_lon1/ s||$rc_lon1|" \
	-e "/rc_lat1/ s||$rc_lat1|" \
	-e "/rc_lon2/ s||$rc_lon2|" \
	-e "/rc_lat2/ s||$rc_lat2|" \
	-e "/rc_nx/ s||$rc_nx|" \
	-e "/rc_ny/ s||$rc_ny|" \
	-e "/rc_ascfile/ s||$rc_ascfile|" \
	$gmapfile_in > $gmapfile
}

# default location of data files (geodata and mapfonts)
_bindir=`dirname $0`
_prefix=`dirname ${_bindir}`
ggeodatadir="${_prefix}/share/nbsp/defaults/geodata"
gmapfontsdir="${_prefix}/share/nbsp/defaults/mapfonts"

# default name of map files
gmapfile="goesr.map"
gmapfile_in="goesr.map.in"	# can be overriden in cmd line

# options
gbackground=0

# variables
rc_lon1=
rc_lat1=
rc_lon2=
rc_lat2=
rc_nx=
rc_ny=
rc_ascfile=
#
ginputfile=
goutputfile=
#
f_keep_map=0
f_keep_map_in=0

#
# main
#
usage="usage: nbspgoesrmap [-baCDk] [-f mapfontsdir] [-g geodatadir]\
 [-m user_map_template] [-o output_file] <ncfile>"

option_a=0
option_C=0
option_D=0
option_k=0
option_m=0
option_o=0

while getopts ":hbaCDkf:g:m:o:" option
do
    case $option in
        h) echo "$usage"; exit 0;;
        b) gbackground=1;;
	a) option_a=1;;		# the inputfile is the asc file rather than nc
	C) option_C=1
	   f_keep_map_in=1;;
	D) option_D=1
	   f_keep_map=1;;
	f) gmapfontsdir=$OPTARG;;
	g) ggeodatadir=$OPTARG;;
        k) option_k=1;;
        m) gmapfile_in=$OPTARG; option_m=1;;
	o) goutputfile=$OPTARG; option_o=1;;
        \?) echo "Unsupported option $OPTARG"; exit 1;;
        :) echo "Missing value option $OPTARG"; exit 1;;
    esac
done
shift $((OPTIND-1))

if [ $option_C -eq 1 ]
then
    make_map_in
    exit 0
fi

[ $# -eq 0 ] && { echo "inputfile?"; exit 1; }

ginputfile=$1
name=${ginputfile%.*}	# drop the suffix
name=`basename $name`

# If [-a] was set then the input file is the asc file
if [ $option_a -eq 0 ]
then
    rc_ascfile="${name}.asc"
else
    rc_ascfile=$ginputfile
fi

# set the default outputfile if it was not set in the command line
[ $option_o -eq 0 ] && goutputfile="${name}.png"

# checks
sanity_check

trap cleanup HUP INT QUIT ABRT KILL ALRM TERM EXIT

# process

# If the input is the nc file extract the parameters from nbspgoesr -i,
# otherwise from the asc file
if [ $option_a -eq 0 ]
then
    set `nbspgoesr -i $ginputfile`
    #
    rc_nx=$1
    rc_ny=$2
    #
    rc_lon1=$5
    rc_lat1=$6
    rc_lon2=$7
    rc_lat2=$8
else
    set `awk 'NR == 1 {print; exit}' $rc_ascfile`; rc_nx=$2
    set `awk 'NR == 2 {print; exit}' $rc_ascfile`; rc_ny=$2
    set `awk 'NR == 3 {print; exit}' $rc_ascfile`; rc_lon1=$2
    set `awk 'NR == 4 {print; exit}' $rc_ascfile`; rc_lat1=$2
    set `awk 'NR == 5 {print; exit}' $rc_ascfile`; _cellsize=$2
    #
    rc_lon2=`echo $rc_lon1 + $rc_nx \* ${_cellsize} | bc`
    rc_lat2=`echo $rc_lat1 + $rc_ny \* ${_cellsize} | bc`
fi

# Write out the (default) map template if one was not specified in the cmd line
[ $option_m -eq 0 ] && { make_map_in; }

# Create the map
make_map

# Exit if -D was set
[ $option_D -eq 1 ] && { exit 0; }

# Create the asc file if the inputfile is the nc file
[ $option_a -eq 0 ] && nbspgoesrgis -a $rc_ascfile $ginputfile

# Create the png
map2img -m $gmapfile -o $goutputfile
