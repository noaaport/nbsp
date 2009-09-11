#!/bin/sh

name=nbsp

tgzfile=${name}.tgz

rm -rf $name
tar -xzf $tgzfile

. $name/VERSION

cd $name
dpkg-buildpackage -rfakeroot
cp ../${name}_${version}*.deb debian
