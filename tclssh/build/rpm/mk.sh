#!/bin/sh

. ../../VERSION

savedir=`pwd`
cd ../../..

rm -rf $name-${version}
cp -R $name $name-${version}

tar -czf $name-$version.tgz $name-$version
mv $name-$version.tgz /usr/src/redhat/SOURCES
rm -rf $name-${version}

cd $savedir
make package
