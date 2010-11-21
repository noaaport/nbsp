#!/bin/sh

name=nbsp

tgzfile=${name}.tgz

rm -rf $name
tar -xzf $tgzfile

. $name/VERSION

cd $name
cp -R build/debian .
dpkg-buildpackage -rfakeroot
cp ../${name}_${version}*.deb build/debian
rm -rf debian

cd build/debian
./ckplist.sh ${name}_${version}*.deb
