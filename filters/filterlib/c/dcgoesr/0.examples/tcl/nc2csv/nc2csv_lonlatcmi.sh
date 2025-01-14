#!/bin/sh
#
# Usage: nc2scv_lonlatcmi ncfile
#
# This script first calls "nc2cvs_xycmi" to extract the xrad,yrad,cmi
# values from the ncfile, then pipes the output to "xy2lonlat" to
# convert the xrad,yrad to lonrad,latrad.
#
# Thing to do:
#
# -o <output>   (default is stdout>
# -l lorigin  (east/west - default is east)

sanity_check () {
    #
    # Check that the ncfile and both programs exist
    #

    ncfile=$1
    [ ! -f "$ncfile" ] && { echo "ncfile not found: $ncfile"; exit 1; }

    for p in $gxy2lonlat_program $gnc2cvs_xycmi_program
    do
	[ ! -x $p ] && { echo "Not found: $p"; exit 1; }
    done
}

# options
gconfdir=
gconffile=
#
gxy2lonlat_program="./xy2lonlat"
gnc2cvs_xycmi_program="./nc2csv_xycmi.tcl"

#
# main
#
usage="usage: nc2cvs_lonlat [-f userconffile] <ncfile>"
option_f=0
while getopts ":hf:" option
do
    case $option in
	h) echo "$usage"; exit 0;;
	f) userconffile=$OPTARG; option_f=1;;
        \?) echo "Unsupported option $OPTARG"; exit 1;;
	:) echo "Missing value option $OPTARG"; exit 1;;
    esac
done
shift $((OPTIND-1))

if [ $option_f -eq 1 ]
then
    . $userconffile
else
    [ -f ${gconfdir}/${gconffile} ] && . ${gconfdir}/${gconffile}
fi

if [ $# -eq 0 ]
then
    echo "Requires ncfile"
    exit 1
fi

ncfile=$1
sanity_check $ncfile

$gnc2cvs_xycmi_program $ncfile | $gxy2lonlat_program
