#!/bin/sh

rm -f rgbcolor.tcl
for c in blue green yellow orange red magenta gray
do
    awk -f mktcltable.awk -v cname=$c ${c}.txt > ${c}.tcl
    cat ${c}.tcl >> rgbcolor.tcl
    echo "" >> rgbcolor.tcl
    rm ${c}.tcl
done
