#!/bin/sh

# read name and version
. ../../VERSION

exclude="build dev-notes examples"

tmpdir=tmp
rm -r -f $tmpdir
mkdir $tmpdir
cd $tmpdir

bzr export ${name}-${version}
cd ${name}-${version}
rm -r $exclude

cd ..
tar -czf ../${name}-${version}.tgz ${name}-${version}

cd ..
rm -r $tmpdir
