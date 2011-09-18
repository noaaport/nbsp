#!/bin/sh

branchname=nbsp

rpmroot=$HOME/rpmbuild

tgzfile=${branchname}.tgz
rm -rf $branchname
tar -xzf $tgzfile

. ./$branchname/VERSION

rm -rf ${name}-${version}
cp -r $branchname ${name}-${version}
tar -czf ${name}-${version}.tgz ${name}-${version}
cp ${name}-${version}.tgz $rpmroot/SOURCES
cd ${name}-${version}/build/rpm
make package
