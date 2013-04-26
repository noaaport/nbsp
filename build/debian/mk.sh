#!/bin/sh

branchname=nbsp

cd ../../..
. ./${branchname}/VERSION

rm -rf ${name}-${version}
cp -r $branchname ${name}-${version}

cd ${name}-${version}
rm -rf debian
cp -R build/debian .
dpkg-buildpackage -rfakeroot -uc -us
cp ../${name}_${version}*.deb build/debian

cd build/debian
./ckplist.sh ${name}_${version}*.deb
