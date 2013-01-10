#!/bin/sh

branchname=tclssh

cd ${branchname}/build/rpm
make clean
make package
