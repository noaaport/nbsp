#!/bin/sh

branchname=nbsp
tgzfile=${branchname}.tgz

rm -rf $branchname
tar -xzf $tgzfile

. ./$branchname/VERSION

rm -rf ${name}-${version}

cp -r $branchname ${name}-${version}
tar -czf ${name}-${version}.tgz ${name}-${version}
cp ${name}-${version}.tgz /home/nieves/rpmbuild/SOURCES
cd ${name}-${version}/build/rpm
make package
