#!/bin/sh
#
# Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
#
# See LICENSE
#
# $Id$
#

subdirs="tcl"

for d in $subdirs
do
  cd $d
  ./configure.sh
  cd ..
done



