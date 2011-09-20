#!/bin/sh

branchname=nbsp

tgzfile=${branchname}.tgz
rm -rf $branchname
tar -xzf $tgzfile
cd ${branchname}/build/rpm

make package
