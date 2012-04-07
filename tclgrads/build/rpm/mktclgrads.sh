#!/bin/sh

name=tclgrads

tgzfile=${name}.tgz

rm -rf $name
tar -xzf $tgzfile
. $name/VERSION

rm -rf $name-$version
cp -R $name $name-$version
tar -czf $name-$version.tgz $name-$version
mv $name-$version.tgz /usr/src/redhat/SOURCES
rm -rf $name-$version

cd $name/build/rpm
make package
