#!/bin/sh

# read name and version
. ../../VERSION

cd ../..
git archive --prefix="${name}-${version}/" \
    -o "build/tools/${name}-${version}.tgz" \
    HEAD
