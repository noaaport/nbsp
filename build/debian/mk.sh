#!/bin/sh

. ../../VERSION

cd ../..

cp -R build/debian .
dpkg-buildpackage
cp ../${name}_${version}*.deb build/debian
fakeroot debian/rules distclean

cd build/debian
./ckplist.sh ${name}_${version}*.deb
