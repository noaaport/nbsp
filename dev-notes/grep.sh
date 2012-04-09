#!/bin/sh

currdir=`pwd`
for d in `find . -type d -not -regex '.*/.svn.*'`
do
    cd $d
    for f in *.tcl *.tcl.in *.init *.lib
    do
      [ $f = "*.tcl" -o $f = "*.tcl.in" -o $f = "*.init" -o $f = "*.lib" ] && continue
      grep -H "nbspunz" $f
    done
    cd $currdir
done
