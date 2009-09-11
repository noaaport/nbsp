#!/bin/sh

. ../VERSION

cd ..
dpkg-buildpackage -rfakeroot
cp ../${name}_${version}*.deb debian
