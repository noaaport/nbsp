#!/bin/sh
#
# Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
#
# See LICENSE
#
# $Id$
#

subdirs="c tcl"

for d in $subdirs
do
  cd $d
  ./configure.sh
  cd ..
done
