#!/bin/sh

branchname=nbsp

cd ../../..
. ./${branchname}/VERSION

rm -rf ${name}-${version}
cp -r $branchname ${name}-${version}

cd ${name}-${version}
cp -R build/debian .
dpkg-buildpackage -rfakeroot -uc -us
cp ../${name}_${version}*.deb build/debian
rm -rf debian

cd build/debian
./ckplist.sh ${name}_${version}*.deb
