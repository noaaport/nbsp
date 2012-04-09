#!/bin/sh
#
# $Id$
#

base=noaaport.data

if [ $# -eq 1 ]
then
    subgroup=$1
    file=
elif [ $# -eq 2 ]
then
    subgroup=$1
    file=$2
else
    echo "One subgroup as argument."
    exit 1
fi

group=$base.$subgroup


if [ -z "$file" ]
then
    cd
    echo -n "$group: "
    bin/ctlinnd newgroup $group y
else
    echo $group >> $file
fi
