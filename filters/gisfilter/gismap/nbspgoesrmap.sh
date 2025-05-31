#!%SHELL%
#
# Copyright (c) 2025 Jose F. Nieves <nieves@ltp.uprrp.edu>
#
# See LICENSE
# 
# $Id$
#
# [ NOTE: A version of this script is also included in the nbspgislibmap
# package under the name "nbspglgoesrmap" with the only difference been
# in the defnition of the location of the "geodata" and "mapfonts" directories.]
#
# Usage: nbspgoesrmap [-baCDknr] [-f mapfontsdir] [-g geodatadir]
#        [-m user_map_template] [-o output_file] [-s size] <ncfile>
#
# This is a (shell) script with no configuration file. The input <ncfile>
# is the netcdf file (e.g., tire05_20250116_1256.goesr)
#
# In the simple case:
#   nbspgoesrmap tire05_20250116_1256.goesr (outout to stdout)
#   nbspgoesrmap -n tire05_20250116_1256.goesr
#     (outout to tire05_20250116_1256.png
#   nbspgoesrmap -o t.png  tire05_20250116_1256.goesr (output to t.png)
#
# -b => background
# -a => the inputfile is the asc file instead of the netcdf file
# -C => writeout the map template and exit (can be edited and submitted with -m)
# -D => writeout the map and exit (can be edited and submitted to map2img)
# -k => keep all tmp files generated (asc, map and map template)
# -n => write to a file (rootname plus .png extension, or name given with [-o])
# -r => the inputfile is an OR_ABI rather than a tixx noaaport file.
# -f => mapfonts dir (default is /usr/local/share/nbspgislib/mapfonts)
# -g => geodata dir (default is /usr/local/share/nbspgislib/geodata)
# -m => specify a map template to use (it is not deleted even if -k is not set)
# -o => output (png) file; the default is stdout.
# -s => set the max value of the greater between height and width in the map
#
# In the simplest use,
#
#   nbspgoesrmap <ncfile>  (e.g. tire05_20250116_1256.goesr)
#
# will produce the png file through the following steps:
#
# 1) Write out the default map template (goesr.map.in)
#
# 2) Execute
#
#    nbspgoesrgis -a <ascfile> <ncfile>
#
#    to create the asc file that will be used in the map2img map file.
#    If [-a] is given this step is omitted.
#
# 3) Determine the extent and size to be used in the "map2img" map file
#    from the asc file.
#
# 4) Converts the map template to the map file (goesr.map)
#    using "sed" to substitute the various parameters by the
#    values extracted in (3), and the name of the ascfile (determined
#    from the name of the input file).
#
# 5) Execute
#
#    map2img -m <mapfile> -o <outputfile>
#
#    using the mapfile created in (4).
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
        logger -t nbspgoesrmap "$1"
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

    # The map template is removed unless the flag is set (via -D)
    [ $f_keep_map_in -eq 0 ] && { rm -f $gmapfile_in; }
    
    # If -k was set then don't delete anything else
    [ $option_k -eq 1 ] && { return; }

    # The asc file is removed if it was created
    [ $option_a -eq 0 ] && rm -f $rc_ascfile

    # The map is removed unless the flag is set (via -C)
    [ $f_keep_map -eq 0 ] && { rm -f $gmapfile; }
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
    SIZE rc_sx rc_sy
    IMAGETYPE png
    IMAGECOLOR 0 0 0
    # FONTSET "${gmapfontsdir}/fonts.list"
    IMAGETYPE png
 
    LAYER
	NAME sat-1
	DATA "rc_ascfile"		# asc file
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
	    STYLE
	      OUTLINECOLOR 255 255 255
	    END
	END
    END

    LAYER
	TYPE POLYGON
	NAME "states"
	STATUS ON
	DATA "${ggeodatadir}/s_01au07/s_01au07.shp"
	CLASS
	    NAME "states-outline"
	    STYLE
	      OUTLINECOLOR 255 255 255
	    END
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
	-e "/rc_sx/ s||$rc_sx|" \
	-e "/rc_sy/ s||$rc_sy|" \
	-e "/rc_ascfile/ s||$rc_ascfile|" \
	$gmapfile_in > $gmapfile
}

# default location of data files (geodata and mapfonts)
ggeodatadir="%MYSHAREDIR%/defaults/geodata"
gmapfontsdir="%MYSHAREDIR%/defaults/mapfonts"

# default name of map files (the map file name is derived from the inputfile)
gmapfile_in="goesr.map.in"	# can be overriden in cmd line
gmapfile_ext="map"
gmapfile=			

# default value of map2img MAXSIZE
MAP2IMG_MAXSIZE=4096

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
rc_sx=
rc_sy=
#
ginputfile=
goutputfile=
#
f_keep_map=0
f_keep_map_in=0

#
# main
#
usage="usage: nbspgoesrmap [-baCDknr] [-f mapfontsdir] [-g geodatadir]\
 [-m user_map_template] [-o output_file] [-s size] <ncfile>"

option_a=0
option_C=0
option_D=0
option_k=0
option_n=0
option_r=0
option_m=0
option_o=0
option_s=$MAP2IMG_MAXSIZE

while getopts ":hbaCDknrf:g:m:o:s:" option
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
	n) option_n=1;;
	r) option_r=1;;		# the inputfile is an OR_ABI nc file
        m) gmapfile_in=$OPTARG; option_m=1;;
	o) goutputfile=$OPTARG; option_o=1; option_n=1;;
	s) option_s=$OPTARG
	   [ $option_s -gt $MAP2IMG_MAXSIZE ] &&
	       { log_err_quit "max val of -s is $MAP2IMG_MAXSIZE"; };;
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

# -a and -r conflict
[ $option_a -eq 1 -a $option_r -eq 1 ] && { echo "-r,-a conflict"; exit 1; }  

# the mapfile name is derived from the input file name
gmapfile="${name}.${gmapfile_ext}"

# If [-a] was set then the input file is the asc file
if [ $option_a -eq 0 ]
then
    rc_ascfile="${name}.asc"
else
    rc_ascfile=$ginputfile
fi

# outputfile
# The default is stdout. If [-n] is given then the output is written
# to the file name derived from the input file (rootname.png),
# or to the file given with [-o]. Giving [-o] implies [-n].
[ $option_n -eq 1 -a $option_o -eq 0 ] && goutputfile="${name}.png"

# checks
sanity_check

trap cleanup HUP INT QUIT ABRT KILL ALRM TERM EXIT

# process

# If -a is not set (the inputfile is the nc file), it is best to create
# the asc file first, and extract the rc_xxx parameters from it. The
# alternative of using something like
#
# set `nbspgoesr -i $ginputfile`
#
# rc_nx=$1
# rc_ny=$2
# rc_lon1=$5
# rc_lat1=$6
# rc_lon2=$7
# rc_lat2=$8
#
# is simpler but would involve reading the nc file twice (here and then
# to create the asc file) and for large files it would be costly.

# Create the asc file if the inputfile is the nc file
if [ $option_a -eq 0 ]
then
    _status=0
    [ $option_r -eq 0 ] && { nbspgoesrgis -a $rc_ascfile $ginputfile;
			     _status=$?; }
    [ $option_r -eq 1 ] && { nbspgoesrgis -r -a $rc_ascfile $ginputfile;
			     _status=$?; }

    [ ${_status} -eq 1 ] && { exit 1; }
fi

# Extract the parameters from the asc file
set `awk 'NR == 1 {print; exit}' $rc_ascfile`; rc_nx=$2
set `awk 'NR == 2 {print; exit}' $rc_ascfile`; rc_ny=$2
set `awk 'NR == 3 {print; exit}' $rc_ascfile`; rc_lon1=$2
set `awk 'NR == 4 {print; exit}' $rc_ascfile`; rc_lat1=$2
set `awk 'NR == 5 {print; exit}' $rc_ascfile`; _cellsize=$2
#
rc_lon2=`echo "$rc_lon1 + $rc_nx * ${_cellsize}" | bc`
rc_lat2=`echo "$rc_lat1 + $rc_ny * ${_cellsize}" | bc`
#
# The (default) SIZE parameter in the map.
#
rc_sx=$rc_nx
rc_sy=$rc_ny
#
# If the larger of width and height is greater than the maximum, renormalize.
# The maximum is given in option_s, which is what was set with [-s] or
# the map2img default.
#
_sg=$rc_sy
[ $rc_sx -gt $rc_sy ] && { _sg=$rc_sx; } # _sg is the larger of width and height

if [ ${_sg} -gt $option_s ]
then
    rc_sx=`echo "$rc_sx * $option_s/${_sg}"  | bc`
    rc_sy=`echo "$rc_sy * $option_s/${_sg}"  | bc`
fi

# Write out the (default) map template if one was not specified in the cmd line
[ $option_m -eq 0 ] && { make_map_in; }

# Create the map
make_map
[ $? -ne 0 ] && exit 1

# Exit if -D was set
[ $option_D -eq 1 ] && { exit 0; }

# Create the png
if [ "$goutputfile" != "" ]
then
    map2img -m $gmapfile -o $goutputfile
else
    map2img -m $gmapfile
fi
