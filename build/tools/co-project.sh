#!/bin/sh

[ $# ne 1 ] && { echo "Needs project name."; exit 0; }
p=$1

svn co http://svn.1-loop.net/nbsprepo/$p/trunk $p

