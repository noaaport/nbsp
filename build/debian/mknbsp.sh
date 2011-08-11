#!/bin/sh

branchname=nbsp
tgzfile=${branchname}.tgz

rm -rf $branchname
tar -xzf $tgzfile

. ./$branchname/VERSION

rm -rf ${name}-${version}
cp -r $branchname ${name}-${version}

cd ${name}-${version}
cp -R build/debian .
dpkg-buildpackage -rfakeroot
cp ../${name}_${version}*.deb build/debian
rm -rf debian

cd build/debian
./ckplist.sh ${name}_${version}*.deb
