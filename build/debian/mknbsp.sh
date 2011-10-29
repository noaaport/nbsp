#!/bin/sh

name=nbsp

tgzfile=${name}.tgz

rm -rf $name
tar -xzf $tgzfile

cd $name/build/debian
./mk.sh
