#!/bin/sh
#
# $Id$
#

# The misc groups are
#
#	noaaport.misc.adm

grouplist="adm"

for g in $grouplist
do
    ./add_misc1.sh $g $1
done
