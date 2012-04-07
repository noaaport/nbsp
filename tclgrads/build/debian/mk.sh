#!/bin/sh

. ../../VERSION

cd ..
cp -R debian ..

cd ..
dpkg-buildpackage -rfakeroot
cp ../${name}_${version}*.deb debian
