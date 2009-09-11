#!/bin/sh

. ../../VERSION

cd ../..

cp -R build/debian .
dpkg-buildpackage -rfakeroot
cp ../${name}_${version}*.deb build/debian
rm -rf debian
