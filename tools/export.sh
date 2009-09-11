#!/bin/sh
#
# $Id$
#

tmpdir=tmp
exclude="tools dev-notes examples/filters bsd rpm macosx debian"
libs="libconnth libtclconf libqdb libspoolbdb"

# read name and version
. ../VERSION

rm -r -f $tmpdir
mkdir $tmpdir

cd $tmpdir
cvs export -D now -d ${name}-${version} ${name}

cd ${name}-${version}
rm -r $exclude
cvs export -D now -d tclhttpd nbsptclhttpd
#cvs export -D now tclmetar
#cvs export -D now tclupperair
for l in tclmetar tclupperair
do
  cvs -d :ext:nieves@${l}.cvs.sourceforge.net:/cvsroot/${l} export -D now ${l}
done

cd src
for l in $libs
do
  cvs export -D now ${l}
done

cd ../.. 
tar -czf ../${name}-${version}.tgz ${name}-${version}

cd ..
rm -r $tmpdir
