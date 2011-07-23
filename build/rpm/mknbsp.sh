#!/bin/sh

name=nbsp

rpmroot=$HOME/rpmbuild

tgzfile=${name}.tgz

rm -rf $name
tar -xzf $tgzfile

. $name/VERSION

rm -rf $name-${version}

cp -r $name $name-${version}
tar -czf $name-$version.tgz $name-$version
cp $name-$version.tgz $rpmroot/SOURCES
cd $name/build/rpm
make package
