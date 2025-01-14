#!/bin/sh
#
# Illustrate how to set the name of the output png file using
# the nbspgoesrinfo
#
file=$1

set - `nbspgoesrinfo $file`
name=$1

nbspgoesr -o ${name}.png $file
